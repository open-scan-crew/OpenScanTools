#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

struct Color {

	Color() {};
	Color(float r, float g, float b) { rgb[0] = r; rgb[1] = g; rgb[2] = b; }
	float rgb[3];
};

struct Color_R8G8B8A8 {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct Position {

	Position() {};
	Position(float x, float y, float z) { coord[0] = x; coord[1] = y; coord[2] = z; }
	float coord[3];
};

struct PointColor {
	float x;
	float y;
	float z;
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

// TODO(robin) Gérer le cas ou est out_of_memory en dehors du transfert
enum TransfertState {
	TFT_NOT_STARTED,
	TFT_STARTED,
	TFT_FINISHED,
	TFT_FAILED,
	TFT_OUT_OF_MEMORY
};




/* Thread safe queue */
template <typename T>
class tsQueue {

public:

	void pop(T& item)
	{
		std::unique_lock<std::mutex> mlock(_mutex);
		while (_queue.empty())
		{
			_cond.wait(mlock);
		}
		item = _queue.front();
		_queue.pop();
	}

	void push(const T& item)
	{
		std::unique_lock<std::mutex> mlock(_mutex);
		_queue.push(item);
		mlock.unlock();
		_cond.notify_one();
	}



private:
	std::queue<T> _queue;
	std::mutex _mutex;
	std::condition_variable _cond;
};