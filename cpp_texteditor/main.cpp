#include "Screen.h"
#include "Keyboard.h"
#include "WinConsole.h"
#include "Channel.h"

#include <mutex>
#include <iostream>

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
void input_worker( Keyboard * kb, std::shared_ptr<Channel<KeyEvent>> ch, std::shared_ptr<State> state ) {

	while( state->shouldRun( ) ) {

		while( kb->keysReady( ) ) {
			
			KeyEvent c = kb->readKey( );
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
void screen_worker( Screen * screen, std::shared_ptr<Channel<KeyEvent>> ch, std::shared_ptr<State> state ) {

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
	try {
		WinConsole console;
		console.init( );

		while( true ) {
			KeyEvent c = console.readKey( );
			if( c.printable ) {
				std::cerr << c.ascii;
				if( c.ascii == 'Q' )
					break;
			}
		}
	} catch( std::system_error e ) {
		std::cerr << e.code( ) << " " << e.what( ) << std::endl;
	}

	return 0;

}