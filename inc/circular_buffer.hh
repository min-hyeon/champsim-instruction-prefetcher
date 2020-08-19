#ifndef CIRCULAR_BUFFER_HH
#define CIRCULAR_BUFFER_HH

#include <iostream>
#include <string>
#include <list>

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

	list<uint64_t> buffer_;

	uint64_t evict_;

	CircularBuffer(string name) : name_(name), evict_(0) {}

	uint8_t is_full();
	void clear_buffer();
	void print_all();

	void enqueue(uint64_t ip);
	uint64_t dequeue();

	list<uint64_t> *dequeue_all();
};

uint8_t CircularBuffer::is_full()
{
	return buffer_.size() == BUFFER_ENTRY;
}

void CircularBuffer::clear_buffer()
{
	buffer_.clear();

	evict_ = 0;
}

void CircularBuffer::print_all()
{
	for (list<uint64_t>::iterator iter = buffer_.begin(); iter != buffer_.end(); iter++)
		cout << hex << "0x" << *iter << endl;
}

void CircularBuffer::enqueue(uint64_t ip)
{
	if (is_full())
	{
		buffer_.pop_front();
		evict_++;
	}
	buffer_.push_back(ip);

	BDP(cout << "[" << name_ << "] " << __func__ << hex << " ip: 0x" << ip << dec << " index: " << buffer_.size() - 1 + evict_ << endl;
		print_all(););
}

uint64_t CircularBuffer::dequeue()
{
	if (buffer_.size() == 0)
		return 0;

	uint64_t ip = buffer_.front();
	buffer_.pop_front();

	BDP(cout << "[" << name_ << "] " << __func__ << hex << " ip: 0x" << ip << dec << " index: " << evict_ << endl;
		print_all(););

	return ip;
}

list<uint64_t> *CircularBuffer::dequeue_all()
{
	list<uint64_t> *data = new list<uint64_t>(buffer_);

	clear_buffer();

	return data;
}

#endif
