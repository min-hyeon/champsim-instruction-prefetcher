#include <memory>
#include <iostream>

#define BUFFER_DEGUG_PRINT
#ifdef BUFFER_DEGUG_PRINT
#define BUFFER_DP(x) x
#else
#define BUFFER_DP(x)
#endif 

#define MAX_BUFFER_SIZE 5

namespace CB {
	template <typename T>
	class CircularBuffer {
	private:
		std::shared_ptr<T[]> buffer_;

		size_t front_ = 0;
		size_t rear_ = 0;
		size_t num_ = 0;
		int num_of_evict_ = 0;

	public:
		CircularBuffer<T>() :
		buffer_(std::shared_ptr<T[]>(new T[MAX_BUFFER_SIZE])) {};

		void enqueue(T new_entry) {
			if (is_full())
			{
				num_of_evict_++;
				front_ = (front_ + 1) % MAX_BUFFER_SIZE;
			}
			buffer_[rear_] = new_entry;
			num_++;
			BUFFER_DP(
				std::cout << "[ENQUEUE] " << buffer_[rear_] << " to " << rear_ << std::endl;
				print_all();
			)
			rear_ = (rear_ + 1) % MAX_BUFFER_SIZE;
		}

		T dequeue() {
			T return_entry = buffer_[front_];
			num_--;
			BUFFER_DP(
				std::cout << "[DEQUEUE] " << buffer_[front_] << " from " << front_ << std::endl;
				print_all();
			)

			front_ = (front_ + 1) % MAX_BUFFER_SIZE;
			return return_entry;
		}

		std::pair<std::shared_ptr<T[]>, size_t> dequeue_all() {
			std::shared_ptr<T[]> return_ptr (new T[MAX_BUFFER_SIZE]);
			if (front_ == 0 && !is_full())
			{
				for (size_t i = 0; i < rear_; i++)
				{	
					return_ptr[i] = buffer_[i];
					BUFFER_DP(
						std::cout << "[DEQUEUE] " << buffer_[i] << " from " << i << std::endl;
					)
				}
			}
			else
			{
				for (size_t i = front_; i < MAX_BUFFER_SIZE; i++)
				{
					return_ptr[i] = buffer_[i];
					BUFFER_DP(
						std::cout << "[DEQUEUE] " << buffer_[i] << " from " << i << std::endl;
					)
				}
				for (size_t i = 0; i < front_; i++)
				{
					return_ptr[i] = buffer_[i];
					BUFFER_DP(
						std::cout << "[DEQUEUE] " << buffer_[i] << " from " << i << std::endl;
					)
				}
			}
			size_t return_num_of_entry = is_full() ? MAX_BUFFER_SIZE : num_ % MAX_BUFFER_SIZE;
			clear_buffer();
			return std::make_pair(return_ptr, return_num_of_entry);
		}

		void clear_buffer() {
			for (size_t i = 0; i < MAX_BUFFER_SIZE; i++)
			{
				buffer_[i] = 0;
			}
			front_ = 0;
			rear_ = 0;
			num_ = 0;
			num_of_evict_ = 0;
		}

		bool is_full() {
			return num_ != 0 && front_ == rear_;
		}

		BUFFER_DP(void print_all() {
			for (size_t i = 0; i < ((num_of_evict_ == 0) ? num_ : MAX_BUFFER_SIZE); i++)
				std::cout << buffer_[i] << std::endl;
		})

	};
}