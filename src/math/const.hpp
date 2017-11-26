#ifndef BLOBS_MATH_CONST_HPP_
#define BLOBS_MATH_CONST_HPP_


namespace blobs {

constexpr double PI = 3.141592653589793238462643383279502884;
constexpr double PI_0p25 = PI * 0.25;
constexpr double PI_0p5 = PI * 0.5;
constexpr double PI_1p5 = PI * 1.5;
constexpr double PI_2p0 = PI * 2.0;

constexpr double PI_inv = 1.0 / PI;
constexpr double PI_0p5_inv = 1.0 / PI_0p5;

/// gravitational constant
constexpr double G = 6.674e-11; // m³kg¯¹s¯²

}

#endif
