#ifndef BLOBS_APP_ERROR_HPP_
#define BLOBS_APP_ERROR_HPP_

#include <al.h>
#include <stdexcept>


namespace blobs {
namespace app {

class AlutError
: public std::runtime_error {

public:
	explicit AlutError(ALenum);
	AlutError(ALenum, const std::string &);

};

class GLError
: public std::runtime_error {

public:
	explicit GLError(const char *msg);

};

class SDLError
: public std::runtime_error {

public:
	SDLError();
	explicit SDLError(const std::string &);

};

class NetError
: public std::runtime_error {

public:
	NetError();
	explicit NetError(const std::string &);

};

}
}

#endif
