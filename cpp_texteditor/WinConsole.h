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

	char doReadKey( ) {

		DWORD read;
		if( !ReadConsoleInput(
			this->hStdin,
			this->irInBuf,
			1,
			&read
		) )
			throw std::runtime_error( "Failed to read from console" );

		WORD vKeycode;
		switch( irInBuf[ 0 ].EventType ) {
			case KEY_EVENT:
				if( irInBuf[ 0 ].Event.KeyEvent.bKeyDown ) {
					vKeycode = irInBuf[ 0 ].Event.KeyEvent.wVirtualKeyCode;
					// For now, only return ascii chars
					if( ( vKeycode >= 0x30 && vKeycode <= 0x39 ) || ( vKeycode >= 0x41 && vKeycode <= 0x5A ) )
						return vKeycode;

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

		return 0;

	};

public:
	// Get the console handle, save the current settings
	WinConsole( );

	// Restore the console settings
	~WinConsole( );

};