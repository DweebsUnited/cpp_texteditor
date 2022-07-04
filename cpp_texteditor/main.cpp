#include "Screen.h"
#include "Keyboard.h"
#include "WinConsole.h"
#include "Channel.h"
#include "Emacs.h"

#include <mutex>
#include <iostream>
#include <format>
#include <string>

void printError( std::system_error & e, const char * msg ) {
	std::cerr << "Received a system error!" << std::endl;
	std::cerr << msg << std::endl;
	std::cerr << e.code( ) << " " << e.what( ) << std::endl;
}

// Global state
class State {
	bool _shouldRun = true;
	std::mutex state_mtx;

public:

	bool shouldRun( ) {
		std::unique_lock<std::mutex> state_lock( state_mtx );
		return _shouldRun;
	}

	void stop( ) {
		std::unique_lock<std::mutex> state_lock( state_mtx );
		_shouldRun = false;
	}

	void start( ) {
		std::unique_lock<std::mutex> state_lock( state_mtx );
		_shouldRun = true;
	}

};


// Input worker thread
// Takes:
//   Message queue to output to
//   Shared state
// Does:
//   Exit if no longer running
//   If input, read and interpret, output messages to queue
//   Else wait on State change semaphore with timeout -- Or naively sleep
//
void input_worker(
	std::shared_ptr<Keyboard> kb,
	std::shared_ptr<Channel<KeyEvent>> ch,
	std::shared_ptr<State> state ) {

	while( state->shouldRun( ) ) {

		while( kb->keysReady( ) ) {
			
			KeyEvent c;

			try {
				c = kb->readKey( );
			} catch( std::system_error & e ) {
				// Failed to read!
				// Set state and return
				state->stop( );
				printError( e, "Input worker caught system error!" );
			}

			// If KeyEvent is a Ctrl-Q, emit quit and tell global state to stop
			if( c.type == KeyEventType::KET_PRINT ) {
				KeyEventPrintable & prnt = std::get<KeyEventPrintable>( c.event );
				if( prnt.ascii == 'q' && prnt.shft == false && prnt.ctrl == true && prnt.alt == false ) {
					// Quit!
					state->stop( );
					return;
				}
			}

			ch->push( std::move( c ) );

		}

		// Sleep 5 ms
		std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );

	}

}

// Screen worker thread
// Takes:
//   Message queue to read from
//   Shared state
// Does:
//   Exit if no longer running
//   If input, read and interpret
//   Else wait 
void screen_worker(
	std::shared_ptr<Screen> screen,
	std::shared_ptr<Channel<ScreenCommand>> ch,
	std::shared_ptr<State> state ) {

	while( state->shouldRun( ) ) {

		while( ch->size( ) > 0 ) {

			std::unique_ptr<ScreenCommand> c = ch->pop( );
			screen->consumeCommand( *c );

		}

		// Sleep 5 ms
		std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );

	}

}

// Editor thread
// Takes:
//   Message queue from Keyboard
//   Message queue to Screen
//   Shared state
// Does:
//   Exit if no longer running
//   If input, read and interpret
//   Else wait
void editor_worker(
	std::shared_ptr<Editor> editor,
	std::shared_ptr<Channel<KeyEvent>> ch_in,
	std::shared_ptr<Channel<ScreenCommand>> ch_out,
	std::shared_ptr<State> state ) {

	while( state->shouldRun( ) ) {

		while( ch_in->size( ) > 0 ) {

			std::unique_ptr<KeyEvent> c = ch_in->pop( );
			if( c->type == KeyEventType::KET_PRINT ) {

				KeyEventPrintable & prnt = std::get<KeyEventPrintable>( c->event );

				ch_out->push( ScreenCommand( std::format(
					"{}{}{}",
					prnt.ctrl ? 'C' : ' ',
					prnt.alt ? 'A' : ' ',
					prnt.ascii ),
					0, 0, false ) );

			}

		}

		// Sleep 5 ms
		std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );

	}

}


int main( ) {

	// Create console object 
	std::shared_ptr<WinConsole> console;
	try {

		console = std::make_shared<WinConsole>( );
		console->init( );

	} catch( std::system_error & e ) {
		printError( e, "Failed to initialize the Console!" );
	}

	// Start with a line buffer-backed emacs
	std::shared_ptr<Emacs<LineEditor>> editor = std::make_shared<Emacs<LineEditor>>( );


	// Make a state and a few channels
	std::shared_ptr<State> state = std::make_shared<State>( );

	std::shared_ptr<Channel<KeyEvent>> ch_keybrd = std::make_shared<Channel<KeyEvent>>( );
	std::shared_ptr<Channel<ScreenCommand>> ch_screen = std::make_shared<Channel<ScreenCommand>>( );

	// Start up some worker threads
	std::thread input_thread( input_worker, console, ch_keybrd, state );
	std::thread editor_thread( editor_worker, editor, ch_keybrd, ch_screen, state );
	std::thread screen_thread( screen_worker, console, ch_screen, state );

	input_thread.join( );
	screen_thread.join( );

	return 0;

}