#pragma once

#include <utility>

// The abstract Screen class, defining everything a screen must be able to do
class Screen {
protected:

	// Size of screen -- Should be set during init and resize
	size_t rows = 0;
	size_t cols = 0;

	size_t currx = 0;
	size_t curry = 0;

	// Do any initialization needed
	virtual bool doInit( ) = 0;

public:
	// Public non-virtual init function
	bool init( ) { return this->doInit( ); };

	// Get current size
	std::pair<size_t, size_t> getSize( ) { return { this->rows, this->cols }; };

	// Get current cursor state -- Not required to be correctly updated
	std::pair<size_t, size_t> getCurrPos( ) { return { this->currx, this->curry }; };

};