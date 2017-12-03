#ifndef BLOBS_UI_STRING_HPP_
#define BLOBS_UI_STRING_HPP_

#include "../math/glm.hpp"

#include <string>

namespace blobs {
namespace ui {

std::string DecimalString(double n, int p);
std::string LengthString(double m);
std::string MassString(double kg);
std::string NumberString(int n);
std::string PercentageString(double n);
std::string TimeString(double s);
std::string VectorString(const glm::dvec3 &, int p);
std::string VectorString(const glm::ivec2 &);

}
}

#endif
