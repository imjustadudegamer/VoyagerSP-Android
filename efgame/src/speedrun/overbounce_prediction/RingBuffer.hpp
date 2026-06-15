#pragma once

#include <atomic>
#include <cstddef>
#include <vector>


template<class T>
class RingBuffer {
public:
	RingBuffer(std::size_t const capacity, double const read_reset_forget_ratio)
	: buffer(capacity), insertionIndex(0), readIndex(0),
	  READ_RESET_FORGET_RATIO(read_reset_forget_ratio) {
	}

	void setReadIndex(std::size_t num_read_values) {
		readIndex = (readIndex + num_read_values) % buffer.size();
	}

	void resetReadIndex() {
		std::size_t num_forget = static_cast<std::size_t>(buffer.size() * READ_RESET_FORGET_RATIO);
		readIndex = (insertionIndex + num_forget) % buffer.size();
	}

	std::vector<T> getLastValues() const {
		std::size_t const read_index = readIndex;
		std::size_t const insertion_index = insertionIndex;

		if (insertion_index >= read_index) {
			return std::vector<T>(buffer.cbegin() + read_index,
			                      buffer.cbegin() + insertion_index);
		}

		std::vector<T> last_values(buffer.cbegin() + read_index, buffer.cend());
		last_values.insert(last_values.end(),
		                   buffer.cbegin(), buffer.cbegin() + insertion_index);
		return last_values;
	}

	void add(T value) {
		buffer[insertionIndex] = std::move(value);
		insertionIndex = (insertionIndex + 1) % buffer.size();
	}

private:
	std::vector<T> buffer;
	std::atomic<std::size_t> insertionIndex;
	std::atomic<std::size_t> readIndex;
	double const READ_RESET_FORGET_RATIO;
};
