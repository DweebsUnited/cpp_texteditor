#pragma once

class Editor {

	size_t currx = 0, curry = 0;

	// Get current cursor position
	std::pair<size_t, size_t> getCurrPos( ) { return { this->currx, this->curry }; };



};