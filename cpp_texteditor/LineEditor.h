#pragma once

#include "Editor.h"

// Editor using a series of line buffers each with a max length
// No overflow, no wraparound, etc
class LineEditor : public Editor {
protected:
	
	std::vector<ScreenCommand> doConsumeKey( KeyEvent & key ) { return std::vector<ScreenCommand>( ); };

};