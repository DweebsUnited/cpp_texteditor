#pragma once

#include <utility>
#include <string>
#include <variant>

// The screen command class, documenting everything that we might tell the screen to do
// These will be emitted by an Editor or Keyboard to update the screen
enum class ScreenCommandType {
	SC_NOP,
	SC_RESIZE,
	SC_CLEAR,
	SC_PUTSTRING,
};

struct ScreenCommandResize {
	size_t cols;
	size_t rows;
};
struct ScreenCommandPutStr {
	std::string msg;
	size_t x;
	size_t y;
	bool insert;
};

struct ScreenCommand {

	ScreenCommandType type;

	std::variant<ScreenCommandResize, ScreenCommandPutStr> cmd;

	ScreenCommand( ) : type( ScreenCommandType::SC_NOP ) { };
	ScreenCommand( ScreenCommandType & sc ) : type( sc ) { };
	ScreenCommand( size_t cols, size_t rows ) :
		type( ScreenCommandType::SC_RESIZE ),
		cmd( ScreenCommandResize( { cols, rows } ) ) { }
	ScreenCommand( std::string && msg, size_t x, size_t y, bool insert = true ) :
		type( ScreenCommandType::SC_PUTSTRING ),
		cmd( ScreenCommandPutStr( { std::move( msg ), x, y, insert } ) ) { };

};

// The abstract Screen class, defining everything a screen must be able to do
class Screen {
protected:

	// Size of screen -- Should be set during init and resize
	size_t rows = 0;
	size_t cols = 0;

	// Do any initialization needed
	virtual bool doInit( ) = 0;

	// Clear the screen, reset cursor to 0,0
	virtual bool doClear( ) = 0;

	// Set the size, realloc any buffers if needed
	virtual bool doSetSize( size_t cols, size_t rows ) = 0;
	// Put a string
	virtual size_t doPutString( std::string & str, size_t x, size_t y, bool insert = true ) = 0;

public:
	// Public non-virtual init function
	bool init( ) { return this->doInit( ) && this->clear( ); };

	// Consume a screen command -- Call public funcs as needed
	// Define this in the base, not the derived
	bool consumeCommand( ScreenCommand & sc ) {
		switch( sc.type ) {
		case ScreenCommandType::SC_NOP:
			return true;
		case ScreenCommandType::SC_RESIZE:
		{
			ScreenCommandResize & size = std::get<ScreenCommandResize>( sc.cmd );
			return this->setSize( size.cols, size.rows );
		}
		case ScreenCommandType::SC_CLEAR:
			return this->clear( );
		case ScreenCommandType::SC_PUTSTRING:
		{
			ScreenCommandPutStr & message = std::get<ScreenCommandPutStr>( sc.cmd );
			return this->putString( message.msg, message.x, message.y, message.insert ) == message.msg.length( );
		}
		}

		return false;
	};

	bool clear( ) { return this->doClear( ); }

	// Get current size
	std::pair<size_t, size_t> getSize( ) { return { this->rows, this->cols }; };
	// Set size -- update the tracked size here
	bool setSize( size_t cols, size_t rows ) { this->rows = rows; this->cols = cols; return this->doSetSize( cols, rows ); };

	// Put a string on screen -- Trim if end of line reached
	size_t putString( std::string & str, size_t x, size_t y, bool insert = true ) { return this->doPutString( str, x, y, insert ); }

};