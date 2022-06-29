#pragma once

// Control key events are specially handled
// We don't really care about the values, but ERROR should be 0
enum class ControlKeyEvent {
	// Error -- Key not recognized, or other, we don't care
	CK_ERROR = 0,

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
	CK_M_MUTE,

	// Same for Browser keys
	// TODO

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
// This can be one of several types of key event
enum class KeyEventType {
	KET_PRINT,
	KET_CONTROL,
	KET_RESIZE, // We need to be able to pass resizes through when they come from WinConsole for example
};

struct KeyEvent {

	KeyEventType type;
	
	union {
		struct {
			char ascii;

			bool shft;
			bool ctrl;
			bool alt;
			bool os;
		};

		ControlKeyEvent ck;
		
		struct {
			size_t cols;
			size_t rows;
		};
	};

	// Default constructor is an error
	KeyEvent( ) : type( KeyEventType::KET_CONTROL ), ck( ControlKeyEvent::CK_ERROR ) { };

	// Constructors
	// Printable
	KeyEvent( char c, bool shft = false, bool ctrl = false, bool alt = false, bool os = false ) :
		type( KeyEventType::KET_PRINT ), ascii( c ), shft( shft ), ctrl( ctrl ), alt( alt ), os( os ) { };
	// Control
	KeyEvent( ControlKeyEvent ck ) : type( KeyEventType::KET_CONTROL ), ck( ck ) { };
	// Resize
	KeyEvent( size_t cols, size_t rows ) : type( KeyEventType::KET_RESIZE ), cols( cols ), rows( rows ) { };

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