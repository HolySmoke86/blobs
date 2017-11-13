#ifndef BLOBS_IO_TOKENIZER_HPP_
#define BLOBS_IO_TOKENIZER_HPP_

#include "Token.hpp"

#include <iosfwd>


namespace blobs {
namespace io {

class Tokenizer {

public:

public:
	explicit Tokenizer(std::istream &in);

	bool HasMore();
	const Token &Next();
	const Token &Current() const noexcept { return current; }

private:
	void ReadToken();

	void ReadNumber();
	void ReadString();
	void ReadComment();
	void ReadIdentifier();

	std::istream &in;
	Token current;

};

}
}

#endif
