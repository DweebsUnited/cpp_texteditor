#pragma once

// Since this is the middleware class, it needs to know about both incoming Keyboard events and outgoing Screen events
#include "Keyboard.h"
#include "Screen.h"

#include <vector>

class Editor {
protected:
	size_t currx = 0, curry = 0;

	// Consume a KeyEvent -- Possibly emit a series of ScreenCommands
	//virtual std::vector<ScreenCommand> doConsumeKey( KeyEvent & key ) = 0;

public:
	// Get current cursor position
	std::pair<size_t, size_t> getCurrPos( ) { return { this->currx, this->curry }; };

	// Consume a KeyEvent, convert to ScreenCommands -- This could potentially redraw the entire screen...
	// TODO: Better concept for screen buffering, or more granular screen commands?
	//   Or limit the throughput of the channel based on amount of data being sent?
	//     That would limit overall memory usage, and the editor would just block until the screen catches up
	//     Could be done directly in the Channel, but means this class would need to learn about Channels..
	//std::vector<ScreenCommand> consumeKey( KeyEvent & key ) { return this->doConsumeKey( key ); };

};