#pragma once

#include "Screen.h"
#include "Keyboard.h"

#include "Windows.h"
#include <stdexcept>


// This is an RAII wrapper around the windows console, that handles input and output
class WinConsole : public Screen, public Keyboard {

	// StdIn, StdOut, and the screen buffer
	HANDLE hStdin;
	HANDLE hStdout;

	// Saved console modes
	DWORD savedOutMode;
	DWORD savedInMode;

	// Buffer for input events
	INPUT_RECORD irInBuf[ 16 ];

	// Clear the screen, reset cursor
	bool doInit( ) {

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

	KeyEvent doReadKey( );
	bool doKeysReady( );

public:
	// Get the console handle, save the current settings
	WinConsole( );

	// Restore the console settings
	~WinConsole( );

};