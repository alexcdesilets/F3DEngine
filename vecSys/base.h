#pragma once

#include <cmath>
#include <algorithm>
#include <limits>
#include <array>
#include <numbers>
#include <ostream>
#include <string>
#include <format>
#include <random>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <tuple>

namespace vecSys {
    constexpr double default_Tolerance = 1e-6;
    constexpr double infinity = std::numeric_limits<double>::infinity();
    constexpr double pi = std::numbers::pi_v<double>;
    constexpr double hpi = pi/2.0;
    constexpr double qpi = pi/4.0;
    constexpr double tau = pi*2.0;

    [[nodiscard]] inline constexpr double safe_divide(const double num, const double den) {
        if (den == 0.0) { 
            if (num == 0.0) return 0.0; 
            return num > 0.0 ? INFINITY : -INFINITY; 
        }
        else if (std::isinf(num)) { 
            if (std::isinf(den)) { 
                return (std::signbit(num) == std::signbit(den)) ? 1.0 : -1.0;
            }
            else { 
                return std::copysign(INFINITY, num * den); 
            }
        }
        else if (std::isinf(den)) {
            return 0.0; 
        }
        else { 
            return num / den;
        }
    }

    [[nodiscard]] inline constexpr double safe_sqrt(const double num) {
        if (num == infinity || num == -infinity) return num;
        else if (num < 0.0) return std::copysign(std::sqrt(std::abs(num)), num);
        return std::sqrt(num);
    }

    [[nodiscard]] inline constexpr double quantizeToSteps(const double value, const double stepSize) {
        return std::round(value / stepSize) * stepSize;
    }

    [[nodiscard]] inline constexpr double wrapAngle(const double radians, const unsigned int steps = 0) {
        double angle = std::abs(radians);
        double mod = std::fmod(angle, pi);
        int divs = static_cast<int>(angle / pi);

        if (mod == 0.0) {
            angle =  (divs % 2 == 1) ? std::copysign(pi, radians) : 0.0;
        }
        else {
            angle = (divs % 2 == 1) ? -std::copysign(mod, radians) : std::copysign(mod, radians);
        }

        if (steps == 0) return angle;
        double stepSize = pi / steps;
        return quantizeToSteps(angle, stepSize);
    }

    [[nodiscard]] inline double wrapRange(double val, double r_min, double r_max, unsigned int steps = 0) {
        if (r_min == r_max) return r_min;
        else if (r_min > r_max) {
            std::swap(r_min, r_max);
        }

        double range = r_max - r_min;
        double relative_val = val - r_min;
        double wrapped_relative = std::fmod(relative_val, range);

        if (wrapped_relative < 0.0) {
            wrapped_relative += range;
        }

        if (steps == 0) { return r_min + wrapped_relative; }

        double stepSize = range / steps;
        
        if (stepSize == 0.0) {
            std::cerr << std::format("Invalid Inputs to wrapRange -> You can't fit a {} steps in an f64 range of {}", steps, range);
            std::cerr.flush();
            std::abort();
        }
        else if (stepSize == std::numeric_limits<double>::denorm_min()) {
            return r_min + wrapped_relative;
        }
        
        double quantized_relative = quantizeToSteps(wrapped_relative, stepSize);
        return r_min + quantized_relative;
    }

} // namespace vecSys