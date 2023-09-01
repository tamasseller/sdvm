#ifndef COMPILER_IR_TERMINATIONS_H_
#define COMPILER_IR_TERMINATIONS_H_

#include "meta/Termination.h"

#include "Temporary.h"

#include <vector>
#include <memory>

namespace comp {
namespace ir {

class BasicBlock;

struct Always: TerminationBase<Always>
{
	const std::shared_ptr<BasicBlock> continuation;

	inline Always(decltype(continuation) continuation): continuation(continuation) {}
};

struct Conditional: TerminationBase<Conditional>
{
	enum class Condition
	{
		Eq, Ne,
		LtI, GtI, LeI, GeI,
		LtU, GtU, LeU, GeU,
		LtF, GtF, LeF, GeF,
	};

	const Condition condition;
	const std::shared_ptr<Temporary> first, second;
	const std::shared_ptr<BasicBlock> then, otherwise;

	inline Conditional(decltype(condition) condition, decltype(first) first, decltype(second) second,
			decltype(then) then, decltype(otherwise) otherwise):
		condition(condition), first(first), second(second), then(then), otherwise(otherwise) {}
};

struct Leave: TerminationBase<Leave>
{
	const std::vector<std::shared_ptr<Temporary>> ret;

	inline Leave(decltype(ret) ret): ret(ret) {}
};

} // namespace ir
} // namespace comp

#endif /* COMPILER_IR_TERMINATIONS_H_ */
