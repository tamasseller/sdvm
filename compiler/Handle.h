#ifndef COMPILER_HANDLE_H_
#define COMPILER_HANDLE_H_

#include <vector>
#include <memory>

namespace comp {

class ProgramBuilder;

template<class Target>
class Handle
{
	const std::vector<std::unique_ptr<Target>> *target = nullptr;
	friend ProgramBuilder;

	inline Handle(const std::vector<std::unique_ptr<Target>> &target, const size_t idx): target(&target), idx(idx) {}

public:
	size_t idx = -1u;
	inline Handle() = default;

	inline auto operator ->() const { return (*target)[idx].get(); }
};

} //namespace comp



#endif /* COMPILER_HANDLE_H_ */
