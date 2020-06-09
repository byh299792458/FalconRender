#pragma once

namespace flr {
	namespace math 
	{
		// scientific constants 
		constexpr double PI = 3.1415926;


		// convert to degrees(½Ç¶È)
		inline float toDegrees(float radians)noexcept {
			return radians / PI * 180;
		}
		inline double toDegrees(double radians)noexcept {
			return radians / PI * 180;
		}
		// convert to radians(»¡¶È)
		inline float toRadians(float degrees)noexcept {
			return degrees / 180 * PI;
		}
		inline double toRadians(double degrees)noexcept {
			return degrees / 180 * PI;
		}

		template<typename T>
		inline T clamp(T min, T max, T value) {
			return std::min(std::max(min, value), max);
		}
	}
}
