#ifndef CIRCULAR_BUFFER_HH
#define CIRCULAR_BUFFER_HH

#include <iostream>
#include <string>
#include <utility>

#define BUFFER_DEGUG_PRINT
#ifdef BUFFER_DEGUG_PRINT
#define BDP(x) x
#else
#define BDP(x)
#endif

#define BUFFER_ENTRY 50

using namespace std;

class CircularBuffer
{
public:
	const string name_;

	uint64_t *buffer_;

	uint32_t front_,
		rear_,
		count_;

	uint64_t evict_;

	CircularBuffer(string name)
		: name_(name), front_(0), rear_(0), count_(0), evict_(0)
	{
		buffer_ = new uint64_t[BUFFER_ENTRY];
	};

	~CircularBuffer()
	{
		delete[] buffer_;
	}

	uint8_t is_full();
	void clear_buffer();
	void print_all();

	void enqueue(uint64_t ip);
	uint64_t dequeue();

	pair<uint64_t *, uint32_t> dequeue_all();
};

uint8_t CircularBuffer::is_full()
{
	return count_ == BUFFER_ENTRY;
}

void CircularBuffer::clear_buffer()
{
	for (uint32_t i = 0; i < BUFFER_ENTRY; i++)
		buffer_[i] = 0;

	front_ = 0;
	rear_ = 0;
	count_ = 0;

	evict_ = 0;
}

void CircularBuffer::print_all()
{
	for (uint32_t i = front_; i != rear_; i = (i + 1) % BUFFER_ENTRY)
		cout << hex << "0x" << buffer_[i] << endl;
}

void CircularBuffer::enqueue(uint64_t ip)
{
	buffer_[rear_++] = ip;
	rear_ %= BUFFER_ENTRY;
	if (is_full())
	{
		front_ = (front_ + 1) % BUFFER_ENTRY;
		evict_++;
	}
	else
		count_++;

	BDP(cout << "[" << name_ << "] " << __func__ << hex << " ip: 0x" << ip << dec << " index: " << ((rear_ == 0) ? BUFFER_ENTRY - 1 : rear_ - 1) << endl;
		print_all(););
}

uint64_t CircularBuffer::dequeue()
{
	if (count_ == 0)
		return 0;

	uint64_t ip = buffer_[front_++];
	front_ %= BUFFER_ENTRY;
	count_--;

	BDP(cout << "[" << name_ << "] " << __func__ << hex << " ip: 0x" << ip << dec << " index: " << ((front_ == 0) ? BUFFER_ENTRY - 1 : front_ - 1) << endl;
		print_all(););

	return ip;
}

pair<uint64_t *, uint32_t> CircularBuffer::dequeue_all()
{
	uint64_t *data = is_full() ? new uint64_t[BUFFER_ENTRY] : new uint64_t[count_];
	uint32_t data_count = count_;

	if (!is_full())
		for (uint32_t i = front_; i < rear_; i++)
			data[i] = buffer_[i];
	else
	{
		for (uint32_t i = front_; i < BUFFER_ENTRY; i++)
			data[i - front_] = buffer_[i];
		for (uint32_t i = 0; i < front_; i++)
			data[i + BUFFER_ENTRY - front_] = buffer_[i];
	}

	clear_buffer();

	return pair<uint64_t *, uint32_t>(data, data_count);
}

#endif
