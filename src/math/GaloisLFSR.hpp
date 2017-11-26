#ifndef BLOBS_MATH_GALOISLFSR_HPP_
#define BLOBS_MATH_GALOISLFSR_HPP_

#include <cassert>
#include <cstdint>
#include <limits>


namespace blobs {
namespace math {

class GaloisLFSR {

public:
	// seed should be non-zero
	explicit GaloisLFSR(std::uint64_t seed) noexcept
	: state(seed) {
		if (state == 0) {
			state = 1;
		}
	}

	// get the next bit
	bool operator ()() noexcept {
		bool result = state & 1;
		state >>= 1;
		if (result) {
			state |= 0x8000000000000000;
			state ^= mask;
		} else {
			state &= 0x7FFFFFFFFFFFFFFF;
		}
		return result;
	}

	template<class T>
	T operator ()(T &out) noexcept {
		constexpr int num_bits =
			std::numeric_limits<T>::digits +
			std::numeric_limits<T>::is_signed;
		for (int i = 0; i < num_bits; ++i) {
			operator ()();
		}
		return out = static_cast<T>(state);
	}

	/// special case for randrom(boolean), since static_cast<bool>(0b10) == true
	bool operator ()(bool &out) noexcept {
		return out = operator ()();
	}

	template<class T>
	T Next() noexcept {
		T next;
		return (*this)(next);
	}

	double SNorm() noexcept {
		return 2.0 * UNorm() - 1.0;
	}

	double UNorm() noexcept {
		return double(Next<std::uint64_t>()) * (1.0 / double(std::numeric_limits<std::uint64_t>::max()));
	}

	template<class Container>
	typename Container::reference From(Container &c) {
		assert(c.size() > 0);
		return c[Next<typename Container::size_type>() % c.size()];
	}
	template<class Container>
	typename Container::const_reference From(const Container &c) {
		assert(c.size() > 0);
		return c[Next<typename Container::size_type>() % c.size()];
	}

private:
	std::uint64_t state;
	// bits 64, 63, 61, and 60 set to 1 (counting from 1 lo to hi)
	static constexpr std::uint64_t mask = 0xD800000000000000;

};

}
}

#endif
