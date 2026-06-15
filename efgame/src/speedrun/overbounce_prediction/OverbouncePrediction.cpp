#include "OverbouncePrediction.hpp"
#include <chrono>


constexpr std::chrono::milliseconds WORKER_SLEEP_TIME(5);
constexpr std::size_t FRAMETIME_BUFFER_SIZE = 1250;
constexpr double FRAMETIME_FORGET_RATIO = 0.75;
RingBuffer<float> OverbouncePrediction::frametimes(
	FRAMETIME_BUFFER_SIZE, FRAMETIME_FORGET_RATIO);

void OverbouncePrediction::reportLastFrametime(float const frametime) {
	frametimes.add(frametime);
}

OverbouncePrediction::OverbouncePrediction()
: heightDifference(0.0f), gravity(0.0f), jumpVelocity(0.0f),
  successful_tries_for_go(0), successful_tries_for_jump(0),
  total_tries_for_go(0), total_tries_for_jump(0), terminateThread(false) {
}

OverbouncePrediction::~OverbouncePrediction() {
	stop();
}

void OverbouncePrediction::start() {
	terminateThread = false;
	if (!workerThread.joinable()) {
		workerThread = std::thread(&OverbouncePrediction::calculateProbabilities, this);
	}
}

void OverbouncePrediction::stop() {
	terminateThread = true;
	if (workerThread.joinable()) {
		workerThread.join();
	}
}

void OverbouncePrediction::setParameters(float const new_height_difference,
                                         float const new_gravity,
                                         float const jump_velocity) {
	if (heightDifference != new_height_difference || gravity != new_gravity) {
		heightDifference = new_height_difference;
		gravity = new_gravity;
		successful_tries_for_go = 0;
		total_tries_for_go = 0;
		successful_tries_for_jump = 0;
		total_tries_for_jump = 0;
		frametimes.resetReadIndex();
	}

	if (jumpVelocity != jump_velocity) {
		jumpVelocity = jump_velocity;
		successful_tries_for_jump = 0;
		total_tries_for_jump = 0;
	}
}

double OverbouncePrediction::getProbabilityForGo() const
{
	if (total_tries_for_go == 0) {
		return 0.0;
	}
	return static_cast<double>(successful_tries_for_go) / total_tries_for_go;
}

double OverbouncePrediction::getProbabilityForJump() const
{
	if (total_tries_for_jump == 0) {
		return 0.0;
	}
	return static_cast<double>(successful_tries_for_jump) / total_tries_for_jump;
}

OverbounceTestResult OverbouncePrediction::checkForOverbounce(
		float const initial_velocity,
		std::vector<float>::const_iterator const frametime_begin,
		std::vector<float>::const_iterator const frametime_end) const {
	float height = heightDifference;
	float velocity = initial_velocity;
	float previous_velocity;

	for (auto frametime = frametime_begin; frametime != frametime_end; ++frametime) {
		if (terminateThread) {
			return OverbounceTestResult::STILL_IN_AIR;
		}
		if (height <= 0.25f) {
			if (height >= 0.0f) {
				return OverbounceTestResult::OVERBOUNCE;
			} else {
				return OverbounceTestResult::NORMAL_LANDING;
			}
		}

		previous_velocity = velocity;
		velocity -= gravity * (*frametime);
		height += (velocity + previous_velocity) / 2.0f * (*frametime);
	}
	return OverbounceTestResult::STILL_IN_AIR;
}

void OverbouncePrediction::calculateProbabilities() {
    while (!terminateThread) {
		std::this_thread::sleep_for(WORKER_SLEEP_TIME);

		std::vector<float> last_frametimes = frametimes.getLastValues();

		std::size_t offset;
		for (offset = 0; offset < last_frametimes.size(); offset += 1) {
			auto result_go = checkForOverbounce(0.0f,
				last_frametimes.cbegin() + offset, last_frametimes.cend());
			auto result_jump = checkForOverbounce(jumpVelocity,
				last_frametimes.cbegin() + offset, last_frametimes.cend());
			if (result_go == OverbounceTestResult::STILL_IN_AIR ||
				result_jump == OverbounceTestResult::STILL_IN_AIR) {
				break;
			}

			total_tries_for_go += 1;
			if (result_go == OverbounceTestResult::OVERBOUNCE) {
				successful_tries_for_go += 1;
			}
			total_tries_for_jump += 1;
			if (result_jump == OverbounceTestResult::OVERBOUNCE) {
				successful_tries_for_jump += 1;
			}
		}

		if (offset > 0) {
			frametimes.setReadIndex(offset);
		}
	}
}
