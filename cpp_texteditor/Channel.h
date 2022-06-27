#pragma once

#include <queue>
#include <memory>
#include <mutex>

template <typename M>
class Channel {

	std::mutex queue_mtx;
	std::queue<std::unique_ptr<M>> queue;

public:
	void push( M && msg ) {

		std::unique_lock<std::mutex> lock( queue_mtx );
		queue.push( std::make_unique( std::move( msg ) ) );

	};

	std::unique_ptr<M> pop( ) {

		std::unique_lock lock( queue_mtx );
		if( queue.empty( ) )
			return nullptr;

		std::unique_ptr<M> last = queue.front( );
		queue.pop( );

		return last;

	};

	size_t size( ) {

		std::unique_lock lock( queue_mtx );
		return queue.size( );

	};

};