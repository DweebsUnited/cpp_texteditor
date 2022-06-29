#pragma once

#include <utility>
#include <string>

// The screen command class, documenting everything that we might tell the screen to do
// These will be emitted by an Editor or Keyboard to update the screen


// The abstract Screen class, defining everything a screen must be able to do
class Screen {
protected:

	// Size of screen -- Should be set during init and resize
	size_t rows = 0;
	size_t cols = 0;

	// Do any initialization needed
	virtual bool doInit( ) = 0;

	// Set the size, realloc any buffers if needed
	virtual bool doSetSize( size_t cols, size_t rows ) = 0;

	// Put a string
	virtual size_t doPutString( std::string & str, size_t x, size_t y ) = 0;

public:
	// Public non-virtual init function
	bool init( ) { return this->doInit( ); };

	// Get current size
	std::pair<size_t, size_t> getSize( ) { return { this->rows, this->cols }; };
	// Set size -- update the tracked size here
	bool setSize( size_t cols, size_t rows ) { this->rows = rows; this->cols = cols; return this->doSetSize( cols, rows ); };

	// Put a string on screen -- Trim if end of line reached
	size_t putString( std::string & str, size_t x, size_t y ) { return this->doPutString( str, x, y ); }

};