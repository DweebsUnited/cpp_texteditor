#pragma once

// Control key events are specially handled
// We don't care about the values, but we do want them to be the same size for the union with printable ascii chars
enum class ControlKeyEvent : char {
	// Error -- Key not recognized, or other, we don't care
	CK_ERROR = 0x00,

	// Other keys
	CK_ESC,
	CK_INSERT,
	CK_NUMLK,

	// Modifiers on their own
	CK_SHFT,
	CK_CTRL,
	CK_ALT,
	CK_OS,

	// Movement
	CK_LEFT,
	CK_RIGHT,
	CK_UP,
	CK_DOWN,
	CK_HOME,
	CK_END,
	CK_PGUP,
	CK_PGDN,

	// F- keys
	CK_F_1,
	CK_F_2,
	CK_F_3,
	CK_F_4,
	CK_F_5,
	CK_F_6,
	CK_F_7,
	CK_F_8,
	CK_F_9,
	CK_F_10,
	CK_F_11,
	CK_F_12,

	// These are special, we do not include whitespace as control keys, but do need to have bkspace and del
	CK_BKSPC,
	CK_DEL,

	// Media keys, why not
	CK_M_PLAY,
	CK_M_SKIP,
	CK_M_VOLU,
	CK_M_VOLD,

	// Add a couple OEM for keyboard-specific functions
	// GKOS for example needs symbol, one shot, 
	CK_OEM1,
	CK_OEM2,
	CK_OEM3,
	CK_OEM4,
	CK_OEM5,
	CK_OEM6,
	CK_OEM7,
	CK_OEM8,
};

// This is a single key event within our program
// This can be a printable ascii character, or a control key
struct KeyEvent {

	bool printable;

	// Printables can have modifiers applied
	bool shft;
	bool ctrl;
	bool alt;
	bool os;
	
	union {
		char ascii;
		ControlKeyEvent ck;
	};

	// Default constructor is an error
	KeyEvent( ) : printable( false ), ck( ControlKeyEvent::CK_ERROR ), shft( false ), ctrl( false ), alt( false ), os( false ) { };

	// Constructor for printables and control keys
	KeyEvent( char c, bool shft = false, bool ctrl = false, bool alt = false, bool os = false ) : printable( true ), ascii( c ), shft( shft ), ctrl( ctrl ), alt( alt ), os( os ) { };
	KeyEvent( ControlKeyEvent ck ) : printable( false ), ck( ck ), shft( false ), ctrl( false ), alt( false ), os( false ) { };

};

// The abstract Keyboard class, defining everything a keyboard must be able to do
class Keyboard {
protected:

	virtual KeyEvent doReadKey( ) = 0;
	virtual bool doKeysReady( ) = 0;

public:

	KeyEvent readKey( ) { return this->doReadKey( ); };
	bool keysReady( ) { return this->doKeysReady( ); };

};