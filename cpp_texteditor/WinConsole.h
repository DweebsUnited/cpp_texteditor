#pragma once

#include "Screen.h"
#include "Keyboard.h"
#include "Editor.h"

#include "Windows.h"

// This is an RAII wrapper around the windows Console, that handles input and output
// Since the console is both I and O, we inherit from Screen and Keyboard, and share access to the Console
// We will have separate classes for Editors, do not combine it here
class WinConsole : public Screen, public Keyboard {
protected:

	// StdIn, StdOut streams
	HANDLE hStdin;
	HANDLE hStdout;

	// Saved console modes, we reset on destruction
	DWORD savedOutMode;
	DWORD savedInMode;

	// Screen operations!

	// WinConsole can track cursor for optimizations
	// This duplicates effort in Editor however
	size_t curx = 0, cury = 0;

	// All init done in the constructor
	bool doInit( );

	// Clear the screen, reset cursor
	bool doClear( );

	bool doSetSize( size_t cols, size_t rows );

	size_t doPutString( std::string & str, size_t x, size_t y, bool insert = true );

	// Keyboard operations!
	
	// Read some keys, interpreting some platform specific ones to ControlKeyEvents
	KeyEvent doReadKey( );
	bool doKeysReady( );

public:
	// Get the console handle, save the current settings
	WinConsole( );

	// Restore the console settings
	~WinConsole( );

	// Helper -- get current cursor
	std::pair<size_t, size_t> getCurPos( ) { return { this->curx, this->cury }; };

};