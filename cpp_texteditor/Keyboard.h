#pragma once

struct KeyEvent {
	char ascii;

};

class Keyboard {
protected:

	virtual char doReadKey( ) = 0;

public:

	char readKey( ) { return this->doReadKey( ); };

};