#include "WinConsole.h"
#include <iostream>
#include <system_error>

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

	// Clear the screen, reset cursor to 0,0
	DWORD written;
	if( !WriteConsole(
		this->hStdout,
		L"\x1B[2J",
		4,
		&written,
		NULL ) )
		return false;

	return written == 4;

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

	WORD vKeycode;
	switch( input.EventType ) {
	case KEY_EVENT:
		if( input.Event.KeyEvent.bKeyDown ) {
			vKeycode = input.Event.KeyEvent.wVirtualKeyCode;
			// For now, only return ascii chars
			if( ( vKeycode >= 0x30 && vKeycode <= 0x39 ) || ( vKeycode >= 0x41 && vKeycode <= 0x5A ) ) {
				// Promote q to a quitter
				if( vKeycode == 'q' || vKeycode == 'Q' )
					return KeyEvent( 'q', false, true, false, false );
				else
					return KeyEvent( ( char ) vKeycode );
			}

			// TODO: This should check the virtual code for a known control character
			//   If not, then check if it is an allowed ascii char
			//   If not, we don't recognize it
		}
		break;

	case WINDOW_BUFFER_SIZE_EVENT:
		// TODO: Handle buffer resize
		break;

	default:
		// TODO: Should we enumerate all the options?
		// Meh...
		break;
	}

	return KeyEvent( ControlKeyEvent::CK_ERROR );

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