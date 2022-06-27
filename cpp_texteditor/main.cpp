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

} state;


// Input worker thread
// Takes:
//   Message queue to output to
//   Shared state variable
// Does:
//   Exit if no longer running
//   If input, read and interpret, output messages to queue
//   Else wait on State change semaphore with timeout
//
void input_worker( std::shared_ptr<Channel<KeyEvent>> ch, std::mutex &  ) {

}

// Screen worker thread
// Takes:
//   Message queue from Keyboard
//   Shared state variable
// Does:
//   Exit if no longer running
//   If input, read and interpret
//   Else wait 


int main( ) {

	// Create console object 
	try {
		WinConsole console;

		while( true ) {
			char c = console.readKey( );
			if( c != 0 ) {
				std::cerr << c;
				if( c == 'Q' )
					break;
			}
		}
	} catch( std::system_error e ) {
		std::cerr << e.code( ) << " " << e.what( ) << std::endl;
	}

	return 0;

}