#pragma once

#include "RingBuffer.hpp"
#include <atomic>
#include <cstddef>
#include <thread>
#include <vector>


enum class OverbounceTestResult {
	OVERBOUNCE,
	NORMAL_LANDING,
	STILL_IN_AIR
};

class OverbouncePrediction {
public:
	static void reportLastFrametime(float frametime);

	OverbouncePrediction();
	~OverbouncePrediction();
	void start();
	void stop();
	void setParameters(float height_difference, float gravity, float jump_velocity);
	double getProbabilityForGo() const;
	double getProbabilityForJump() const;

private:
	static RingBuffer<float> frametimes;

	std::atomic<float> heightDifference;
	std::atomic<float> gravity;
	std::atomic<float> jumpVelocity;

	std::atomic<std::size_t> successful_tries_for_go;
	std::atomic<std::size_t> successful_tries_for_jump;
	std::atomic<std::size_t> total_tries_for_go;
	std::atomic<std::size_t> total_tries_for_jump;
	std::atomic<bool> terminateThread;
	std::thread workerThread;

	void calculateProbabilities();
	OverbounceTestResult checkForOverbounce(
		float const initial_velocity,
		std::vector<float>::const_iterator frametime_begin,
		std::vector<float>::const_iterator frametime_end) const;
};
