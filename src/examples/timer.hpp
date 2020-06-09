#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <chrono>
#include <string>

class Timer {
public:
	Timer() {
		start_time_ = std::chrono::high_resolution_clock::now();
		last_time_ = start_time_;
	}
	void Set(void) {
		last_time_ = std::chrono::high_resolution_clock::now();
	}
	template<typename Duration = std::chrono::milliseconds>
	int64_t Escape(void)
	{
		auto duration = std::chrono::duration_cast<Duration>(
			std::chrono::high_resolution_clock::now() - last_time_).count();
		last_time_ = std::chrono::high_resolution_clock::now();
		return duration;
	}
	int64_t EscapeSecond(void) {
		return Escape<std::chrono::seconds>();
	}
	int64_t EscapeMicro(void) {
		return Escape<std::chrono::microseconds>();
	}
	int64_t EscapeNano(void) {
		return Escape<std::chrono::nanoseconds>();
	}
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
	std::chrono::time_point<std::chrono::high_resolution_clock> last_time_;
};

#endif // !__TIMER_HPP__

