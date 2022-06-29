#include "WinConsole.h"
#include <iostream>
#include <system_error>
#include <string>
#include <format>

// Courtesy of stackoverflow: https://stackoverflow.com/a/69410299
std::wstring string_to_wide_string( const std::string & string ) {
	if( string.empty( ) ) {
		return L"";
	}

	const auto size_needed = MultiByteToWideChar( CP_UTF8, 0, &string.at( 0 ), ( int ) string.size( ), nullptr, 0 );
	if( size_needed <= 0 ) {
		throw std::runtime_error( "MultiByteToWideChar() failed: " + std::to_string( size_needed ) );
	}

	std::wstring result( size_needed, 0 );
	MultiByteToWideChar( CP_UTF8, 0, &string.at( 0 ), ( int ) string.size( ), &result.at( 0 ), size_needed );
	return result;
}

WinConsole::WinConsole( ) {

	// Get the standard input handle. 
	this->hStdin = GetStdHandle( STD_INPUT_HANDLE );
	if( this->hStdin == INVALID_HANDLE_VALUE )
		throw std::system_error(
			std::error_code(
				GetLastError( ),
				std::system_category( )
			), "GetStdHandle for input failed!" );

	// Save the current input mode
	if( !GetConsoleMode( this->hStdin, &( this->savedInMode ) ) )
		throw std::system_error(
			std::error_code(
				GetLastError( ),
				std::system_category( )
			), "GetConsoleMode on input failed!" );


	// Get output handle
	this->hStdout = GetStdHandle( STD_OUTPUT_HANDLE );
	if( this->hStdout == INVALID_HANDLE_VALUE )
		throw std::system_error(
			std::error_code(
				GetLastError( ),
				std::system_category( )
			), "GetStdHandle for output failed!" );

	// Save the current output mode
	if( !GetConsoleMode( this->hStdout, &( this->savedOutMode ) ) )
		throw std::system_error(
			std::error_code(
				GetLastError( ),
				std::system_category( )
			), "GetConsoleMode on output failed!" );


	// Enable window inputs
	DWORD reqInMode = ENABLE_WINDOW_INPUT;
	if( !SetConsoleMode( this->hStdin, reqInMode ) )
		throw std::system_error(
			std::error_code(
				GetLastError( ),
				std::system_category( )
			), "SetConsoleMode on input failed!" );

	// Enable virtual terminal processing on stdout, no newlines on writes
	// Weird, you have to enable processed output to get virtual terminal processing...
	DWORD reqOutMode = ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
	if( !SetConsoleMode( this->hStdout, reqOutMode ) )
		throw std::system_error(
			std::error_code(
				GetLastError( ),
				std::system_category( )
			), "SetConsoleMode on output failed!" );


	// Switch to alternate screen buffer
	// VTESC [ ? 1 0 4 9 h
	DWORD written;
	if( !WriteConsole(
		this->hStdout,
		L"\x1b[?1049h",
		8,
		&written,
		NULL ) or written != 8 )
		throw std::system_error(
			std::error_code(
				written == 8 ? 1 : GetLastError( ),
				std::system_category( )
			), "Failed to switch to alternative buffer!" );

	// Get the screenbuffer info
	CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;
	if( !GetConsoleScreenBufferInfo( this->hStdout, &ScreenBufferInfo ) )
		throw std::system_error(
			std::error_code(
				GetLastError( ),
				std::system_category( )
			), "Failed to get screenbuffer info!" );

	// Save screenbuffer size
	this->cols = ScreenBufferInfo.srWindow.Right - ScreenBufferInfo.srWindow.Left + 1;
	this->rows = ScreenBufferInfo.srWindow.Bottom - ScreenBufferInfo.srWindow.Top + 1;

};

WinConsole::~WinConsole( ) {

	// Write out the switch back to the standard screenbuffer
	// If this fails, the console is in an undefined state, but nothing we can do :shrug:
	// VTESC [ ? 1 0 4 9 l
	DWORD written;
	WriteConsole(
		this->hStdout,
		L"\x1B[?1049l",
		4,
		&written,
		NULL );

	// Restore the console settings
	// If these fail, we don't really care since there's not much we can do
	SetConsoleMode( this->hStdin, this->savedInMode );
	SetConsoleMode( this->hStdout, this->savedOutMode );

};

bool WinConsole::doInit( ) {

	// All init done in the constructor
	return true;

};

bool WinConsole::doClear( ) {

	// Clear the screen, reset cursor to 0,0
	DWORD written;
	if( !WriteConsole(
		this->hStdout,
		L"\x1B[2J\x1B[0;0H",
		10,
		&written,
		NULL ) )
		return false;

	return written == 10;

}


bool WinConsole::doSetSize( size_t cols, size_t rows ) {

	// No need to do anything special for this class -- tracked size already updated by base class
	return true;

};

size_t WinConsole::doPutString( std::string & str, size_t x, size_t y ) {

	// Write cursor position, write to end of line or string
	std::string movecmd = std::format( "\x1B[{};{}H", y, x );
	
	DWORD written = 0;
	if( !WriteConsoleA(
		this->hStdout,
		movecmd.c_str( ),
		movecmd.length( ),
		&written,
		NULL ) )
			
		return 0;

	written = 0;
	int lenToWrite = min( str.length( ), this->cols - x );
	// If this fails here, should we care? Might cause an infinite loop returning 0...
	WriteConsoleA(
		this->hStdout,
		str.c_str( ),
		lenToWrite,
		&written,
		NULL );
	
	return written;

};

KeyEvent WinConsole::doReadKey( ) {

	// TODO: Profile, would batch reading be faster than one at a time?

	DWORD read;
	INPUT_RECORD input;
	if( !ReadConsoleInput(
		this->hStdin,
		&input,
		1,
		&read
	) )
		throw std::system_error(
			std::error_code(
				GetLastError( ),
				std::system_category( )
			), "Failed to read from console" );


	switch( input.EventType ) {
	case KEY_EVENT:
		if( input.Event.KeyEvent.bKeyDown ) {
			WORD vKeycode = input.Event.KeyEvent.wVirtualKeyCode;
			DWORD controlKeys = input.Event.KeyEvent.dwControlKeyState;

			// This is messy, but here goes..
			// We need to match against every key we care about
			// This might be more efficient as a LUT? Hashtable? Blech
			// TODO: Optimize
			switch( vKeycode ) {
			case VK_BACK:
				return KeyEvent( ControlKeyEvent::CK_BKSPC );
			case VK_TAB:
				return KeyEvent( '\t' );
			case VK_RETURN:
				return KeyEvent( '\n' );
			case VK_ESCAPE:
				return KeyEvent( ControlKeyEvent::CK_ESC );
			case VK_SPACE:
				return KeyEvent( ' ' );
			case VK_PRIOR:
				return KeyEvent( ControlKeyEvent::CK_PGUP );
			case VK_NEXT:
				return KeyEvent( ControlKeyEvent::CK_PGDN );
			case VK_END:
				return KeyEvent( ControlKeyEvent::CK_END );
			case VK_HOME:
				return KeyEvent( ControlKeyEvent::CK_HOME );
			case VK_LEFT:
				return KeyEvent( ControlKeyEvent::CK_LEFT );
			case VK_UP:
				return KeyEvent( ControlKeyEvent::CK_UP );
			case VK_RIGHT:
				return KeyEvent( ControlKeyEvent::CK_RIGHT );
			case VK_DOWN:
				return KeyEvent( ControlKeyEvent::CK_DOWN );
			case VK_INSERT:
				return KeyEvent( ControlKeyEvent::CK_INSERT );
			case VK_DELETE:
				return KeyEvent( ControlKeyEvent::CK_DEL );

			// Printables! Output correctly cased, with the appropriate control keys
			case 0x30:
				if( controlKeys & 0x0010 )
					return KeyEvent( '!',
						false,
						controlKeys & ( 0x0008 | 0x0004 ),
						controlKeys & ( 0x0002 | 0x0001 ),
						false );
			case 0x31:
				if( controlKeys & 0x0010 )
					return KeyEvent( '@',
						false,
						controlKeys & ( 0x0008 | 0x0004 ),
						controlKeys & ( 0x0002 | 0x0001 ),
						false );
			case 0x32:
				if( controlKeys & 0x0010 )
					return KeyEvent( '#',
						false,
						controlKeys & ( 0x0008 | 0x0004 ),
						controlKeys & ( 0x0002 | 0x0001 ),
						false );
			case 0x33:
				if( controlKeys & 0x0010 )
					return KeyEvent( '$',
						false,
						controlKeys & ( 0x0008 | 0x0004 ),
						controlKeys & ( 0x0002 | 0x0001 ),
						false );
			case 0x34:
				if( controlKeys & 0x0010 )
					return KeyEvent( '%',
						false,
						controlKeys & ( 0x0008 | 0x0004 ),
						controlKeys & ( 0x0002 | 0x0001 ),
						false );
			case 0x35:
				if( controlKeys & 0x0010 )
					return KeyEvent( '^',
						false,
						controlKeys & ( 0x0008 | 0x0004 ),
						controlKeys & ( 0x0002 | 0x0001 ),
						false );
			case 0x36:
				if( controlKeys & 0x0010 )
					return KeyEvent( '&',
						false,
						controlKeys & ( 0x0008 | 0x0004 ),
						controlKeys & ( 0x0002 | 0x0001 ),
						false );
			case 0x37:
				if( controlKeys & 0x0010 )
					return KeyEvent( '*',
						false,
						controlKeys & ( 0x0008 | 0x0004 ),
						controlKeys & ( 0x0002 | 0x0001 ),
						false );
			case 0x38:
				if( controlKeys & 0x0010 )
					return KeyEvent( '(',
						false,
						controlKeys & ( 0x0008 | 0x0004 ),
						controlKeys & ( 0x0002 | 0x0001 ),
						false );
			case 0x39:
				if( controlKeys & 0x0010 )
					return KeyEvent( ')',
						false,
						controlKeys & ( 0x0008 | 0x0004 ),
						controlKeys & ( 0x0002 | 0x0001 ),
						false );
				else
					return KeyEvent(
						'0' + ( vKeycode - 0x30 ),
						false,
						controlKeys & ( 0x0008 | 0x0004 ),
						controlKeys & ( 0x0002 | 0x0001 ),
						false );

			case 0x41:
			case 0x42:
			case 0x43:
			case 0x44:
			case 0x45:
			case 0x46:
			case 0x47:
			case 0x48:
			case 0x49:
			case 0x4A:
			case 0x4B:
			case 0x4C:
			case 0x4D:
			case 0x4E:
			case 0x4F:
			case 0x50:
			case 0x51:
			case 0x52:
			case 0x53:
			case 0x54:
			case 0x55:
			case 0x56:
			case 0x57:
			case 0x58:
			case 0x59:
			case 0x5A:
				return KeyEvent(
					'a' + ( vKeycode - 0x41 ) - ( controlKeys & 0x0010 ? 0x20 : 0 ),
					controlKeys & 0x0010,
					controlKeys & ( 0x0008 | 0x0004 ),
					controlKeys & ( 0x0002 | 0x0001 ),
					false );

			// I HATE everything about this arithmetic, but it works :D
			case VK_F1:
			case VK_F2:
			case VK_F3:
			case VK_F4:
			case VK_F5:
			case VK_F6:
			case VK_F7:
			case VK_F8:
			case VK_F9:
			case VK_F10:
			case VK_F11:
			case VK_F12:
				return KeyEvent( (ControlKeyEvent)( (int)ControlKeyEvent::CK_F_1 + ( vKeycode - VK_F1 ) ) );

			case VK_NUMLOCK:
				return KeyEvent( ControlKeyEvent::CK_NUMLK );

			default:
				return KeyEvent( );
			};
		}
		break;

	case WINDOW_BUFFER_SIZE_EVENT:
		return KeyEvent( (size_t)input.Event.WindowBufferSizeEvent.dwSize.X, (size_t)input.Event.WindowBufferSizeEvent.dwSize.Y );
		break;

	default:
		// TODO: Should we enumerate all the options?
		// Meh...
		break;
	}

	return KeyEvent( );

};

bool WinConsole::doKeysReady( ) {

	DWORD num;
	if( !GetNumberOfConsoleInputEvents( this->hStdin, &num ) )
		throw std::system_error(
			std::error_code(
				GetLastError( ),
				std::system_category( )
			), "Failed to get count of input events!" );

	return num > 0;

};