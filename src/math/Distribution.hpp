#ifndef BLOBS_MATH_DISTRIBUTION_HPP_
#define BLOBS_MATH_DISTRIBUTION_HPP_

#include <cmath>


namespace blobs {
namespace math {

class Distribution {

public:
	Distribution(double mean = 0.0, double stddev = 1.0) noexcept
	: mean(mean)
	, stddev(stddev)
	{ }

public:
	void Mean(double m) noexcept { mean = m; }
	double Mean() const noexcept { return mean; }
	void StandardDeviation(double d) noexcept { stddev = d; }
	double StandardDeviation() const noexcept { return stddev; }
	double Variance() const noexcept { return stddev * stddev; }

	/// convert uniform random value in [-1,1] to fake normal distribution
	/// in [mean - 2 stdddev,mean + 2 stddev]
	double FakeNormal(double uniform) const noexcept {
		return mean + (uniform * uniform * stddev) + std::abs(uniform * uniform * uniform) * stddev;
	}

private:
	double mean;
	double stddev;

};

}
}

#endif
