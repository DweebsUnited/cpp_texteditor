#pragma once

// Control key events are specially handled
// We don't care about the values, but we do want them to be the same size for the union above
enum ControlKeyEvent : char {
	CK_ERROR = 0x00,
	CK_COPY,
	CK_PASTE,
	// TODO
};

// This is a single key event within our program
// This can be a printable ascii character, or a control key
struct KeyEvent {

	bool printable;
	union {
		char ascii;
		ControlKeyEvent ck;
	};

	KeyEvent( char c ) : printable( true ), ascii( c ) { };
	KeyEvent( ControlKeyEvent ck ) : printable( false ), ck( ck ) { };

};

class Keyboard {
protected:

	virtual KeyEvent doReadKey( ) = 0;
	virtual bool doKeysReady( ) = 0;

public:

	KeyEvent readKey( ) { return this->doReadKey( ); };
	bool keysReady( ) { return this->doKeysReady( ); };

};