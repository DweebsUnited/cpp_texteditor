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
			), "GetStdHandle for input failed" );

	// Save the current input mode
	if( !GetConsoleMode( this->hStdin, &( this->savedInMode ) ) )
		throw std::system_error(
			std::error_code(
				GetLastError( ),
				std::system_category( )
			), "GetConsoleMode on input failed" );


	// Get output handle
	this->hStdout = GetStdHandle( STD_OUTPUT_HANDLE );
	if( this->hStdout == INVALID_HANDLE_VALUE )
		throw std::system_error(
			std::error_code(
				GetLastError( ),
				std::system_category( )
			), "GetStdHandle for output failed" );

	// Save the current output mode
	if( !GetConsoleMode( this->hStdout, &( this->savedOutMode ) ) )
		throw std::system_error(
			std::error_code(
				GetLastError( ),
				std::system_category( )
			), "GetConsoleMode on output failed" );


	// Enable window inputs
	DWORD reqInMode = ENABLE_WINDOW_INPUT;
	if( !SetConsoleMode( this->hStdin, reqInMode ) )
		throw std::system_error(
			std::error_code(
				GetLastError( ),
				std::system_category( )
			), "SetConsoleMode on input failed" );

	// Enable virtual terminal processing on stdout, no newlines on writes
	// Weird, you have to enable processed output to get virtual terminal processing...
	DWORD reqOutMode = ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
	if( !SetConsoleMode( this->hStdout, reqOutMode ) )
		throw std::system_error(
			std::error_code(
				GetLastError( ),
				std::system_category( )
			), "SetConsoleMode on output failed" );


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
				GetLastError( ),
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
	// If this fails, we don't really care since there's not much we can do
	SetConsoleMode( this->hStdin, this->savedInMode );
	SetConsoleMode( this->hStdout, this->savedOutMode );

};