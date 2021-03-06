#pragma once

#include "LineEditor.h"

// Class wrapping a LineEditor to add on modes, states, and special commands
// Templated over the type of Editor it extends from...
//   This seems like a terrible idea for readability of errors :D
template<typename E>
class Emacs : public E {
protected:
	
	std::vector<ScreenCommand> doConsumeKey( KeyEvent & key ) { return E::doConsumeKey( key ); };

};