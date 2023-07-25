#ifndef TYPE_H_
#define TYPE_H_

#include <vector>

struct Type
{
	size_t length;
	std::vector<size_t> refOffs;

	size_t getLength() const {
		return length;
	}

	std::vector<size_t> referenceOffsets() const {
		return refOffs;
	}
};

#endif /* TYPE_H_ */
