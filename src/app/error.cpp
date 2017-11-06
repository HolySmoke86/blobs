#include "error.hpp"

#include <alut.h>
#include <SDL.h>
#include <SDL_net.h>
#include <GL/glew.h>

using std::string;
using std::runtime_error;


namespace {

string sdl_error_append(string msg) {
	const char *error = SDL_GetError();
	if (*error != '\0') {
		msg += ": ";
		msg += error;
		SDL_ClearError();
	}
	return msg;
}

string net_error_append(string msg) {
	const char *error = SDLNet_GetError();
	if (*error != '\0') {
		msg += ": ";
		msg += error;
	}
	return msg;
}

string alut_error_append(ALenum num, string msg) {
	const char *error = alutGetErrorString(num);
	if (*error != '\0') {
		msg += ": ";
		msg += error;
	}
	return msg;
}

string error_append(string msg, const char *err) {
	if (err && *err) {
		msg += ": ";
		msg += err;
	}
	return msg;
}

}

namespace blobs {
namespace app {

AlutError::AlutError(ALenum num)
: runtime_error(alutGetErrorString(num)) {
}

AlutError::AlutError(ALenum num, const string &msg)
: runtime_error(alut_error_append(num, msg)) {
}


GLError::GLError(const char *msg)
: runtime_error(error_append(msg, reinterpret_cast<const char *>(gluErrorString(glGetError())))) {
}


NetError::NetError()
: runtime_error(SDLNet_GetError()) {
}

NetError::NetError(const string &msg)
: runtime_error(net_error_append(msg)) {
}


SDLError::SDLError()
: runtime_error(SDL_GetError()) {
}

SDLError::SDLError(const string &msg)
: runtime_error(sdl_error_append(msg)) {
}

}
}
