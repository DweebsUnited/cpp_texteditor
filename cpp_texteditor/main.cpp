#include "Screen.h"
#include "Keyboard.h"
#include "WinConsole.h"
#include "Channel.h"

#include <mutex>
#include <iostream>

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
void input_worker( std::shared_ptr<Keyboard> kb, std::shared_ptr<Channel<KeyEvent>> ch, std::shared_ptr<State> state ) {

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
			if( c.printable ) {
				if( ( c.ascii == 'Q' || c.ascii == 'q' ) && c.shft == false && c.ctrl == true && c.alt == false ) {
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
void screen_worker( std::shared_ptr<Screen> screen, std::shared_ptr<Channel<KeyEvent>> ch, std::shared_ptr<State> state ) {

	while( state->shouldRun( ) ) {

		while( ch->size( ) > 0 ) {

			std::unique_ptr<KeyEvent> c = ch->pop( );
			if( c->printable )
				std::cerr << c->ascii;

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


int main( ) {

	// Create console object 
	std::shared_ptr<WinConsole> console;
	try {

		console = std::make_shared<WinConsole>( );
		console->init( );

	} catch( std::system_error & e ) {
		printError( e, "Failed to initialize the Console!" );
	}

	// Make a state and a few channels
	std::shared_ptr<State> state = std::make_shared<State>( );

	std::shared_ptr<Channel<KeyEvent>> inToScreen = std::make_shared<Channel<KeyEvent>>( );

	// Start up some worker threads
	std::thread input_thread( input_worker, console, inToScreen, state );
	std::thread screen_thread( screen_worker, console, inToScreen, state );

	input_thread.join( );
	screen_thread.join( );

	return 0;

}