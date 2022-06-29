#pragma once

#include <queue>
#include <memory>
#include <mutex>


// Very simple thread-safe queue
// Basically just wrap push/pop with a mutex and unique_locks

template <typename M>
class Channel {

	std::mutex queue_mtx;
	std::queue<std::unique_ptr<M>> queue;

public:
	void push( M && msg ) {

		std::unique_lock<std::mutex> lock( queue_mtx );
		queue.push( std::move( std::make_unique<M>( std::move( msg ) ) ) );

	};

	std::unique_ptr<M> pop( ) {

		std::unique_lock<std::mutex> lock( queue_mtx );
		if( queue.empty( ) )
			return nullptr;

		std::unique_ptr<M> last = std::move( queue.front( ) );
		queue.pop( );

		return last;

	};

	size_t size( ) {

		std::unique_lock<std::mutex> lock( queue_mtx );
		return queue.size( );

	};

};