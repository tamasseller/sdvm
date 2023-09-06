#ifndef COMPILER_IR_IRBUILDER_H_
#define COMPILER_IR_IRBUILDER_H_

#include "Operations.h"
#include "Terminations.h"

#include "Overloaded.h"

#include "assert.h"

#include <functional>
#include <set>

namespace comp {
namespace ir {

struct IrBuilder
{
	struct LoopInfo
	{
		std::shared_ptr<BasicBlock> start;
		std::vector<std::function<void(std::shared_ptr<BasicBlock>)>> endConsumers;

		inline LoopInfo(decltype(start) start): start(start) {}
	};

	std::shared_ptr<BasicBlock> entry, last;
	std::map<const void*, LoopInfo> loops;

	inline IrBuilder(): entry(std::make_shared<BasicBlock>()), last(entry) {}

private:
	std::pair<std::shared_ptr<BasicBlock>, std::shared_ptr<BasicBlock>> cut()
	{
		auto old = last;
		last = std::make_shared<BasicBlock>();
		return {old, last};
	}

	static inline void join(std::shared_ptr<BasicBlock> from, std::shared_ptr<BasicBlock> to, bool isBackEdge = false) {
		from->termination = std::make_shared<Always>(to, isBackEdge);
	}

	static inline void join(std::shared_ptr<BasicBlock> from, Conditional::Condition condition,
			std::shared_ptr<Temporary> first, std::shared_ptr<Temporary> second,
			std::shared_ptr<BasicBlock> then, std::shared_ptr<BasicBlock> otherwise)
	{
		from->termination = std::make_shared<Conditional>(condition, first, second, then, otherwise);
	}

	static inline void join(std::shared_ptr<BasicBlock> from, std::vector<std::shared_ptr<Temporary>> retvals) {
		from->termination = std::make_shared<Leave>(retvals);
	}

public:
	void addOp(std::shared_ptr<Operation> stmt) {
		last->code.push_back(stmt);
	}

	void addLiteral(std::shared_ptr<Variable> target, int v)
	{
		last->code.push_back(std::make_shared<Copy>(target, std::make_shared<Constant>(target->type, v)));
	}

	template<class C>
	auto genOp(ast::ValueType t, C&& c)
	{
		auto ret = std::make_shared<Variable>(t);
		last->code.push_back(c(ret));
		return ret;
	}

	auto genLiteral(ast::ValueType t, int v) {
		return genOp(t, [&](const auto &r){
			return std::make_shared<Copy>(r, std::make_shared<Constant>(t, v));
		});
	}

	std::shared_ptr<Variable> condToBool(Conditional::Condition condition, std::shared_ptr<Temporary> first, std::shared_ptr<Temporary> second)
	{
		auto ret = std::make_shared<Variable>(ast::ValueType::logical());

		auto ifThenPoint = cut();
		addOp(std::make_shared<Copy>(ret, std::make_shared<Constant>(ast::ValueType::logical(), 1)));
		auto thenElsePoint = cut();
		addOp(std::make_shared<Copy>(ret, std::make_shared<Constant>(ast::ValueType::logical(), 0)));
		auto endifPoint = cut();

		join(ifThenPoint.first, condition, first, second, ifThenPoint.second, thenElsePoint.second);
		join(thenElsePoint.first, endifPoint.second);
		join(endifPoint.first, endifPoint.second);

		return ret;
	}

	template<class T, class O>
	void branch(std::shared_ptr<Variable> decisionInput, T&& then, O&& otherwise)
	{
		auto ifThenPoint = cut();
		then();
		auto thenElsePoint = cut();
		otherwise();
		auto endifPoint = cut();

		join(ifThenPoint.first, Conditional::Condition::Eq, decisionInput, std::make_shared<Constant>(ast::ValueType::logical(), 1), ifThenPoint.second, thenElsePoint.second);
		join(thenElsePoint.first, endifPoint.second);
		join(endifPoint.first, endifPoint.second);
	}

	template<class C>
	void loop(const void* loopIdentity, C&& c)
	{
		auto startPoint = cut();
		join(startPoint.first, startPoint.second);

		auto l = loops.insert({loopIdentity, LoopInfo(startPoint.second)});
		assert(l.second);

		c();

		auto endPoint = cut();
		join(endPoint.first, startPoint.second, true);

		for(const auto& cb: l.first->second.endConsumers)
		{
			cb(endPoint.second);
		}

		loops.erase(l.first);
	}

	void continueLoop(const void* loopIdentity)
	{
		auto it = loops.find(loopIdentity);
		assert(it != loops.end()); // TODO compiler error: continue outside loop

		auto cutPoint = cut();
		join(cutPoint.first, it->second.start, true);
	}

	void breakLoop(const void* loopIdentity)
	{
		auto it = loops.find(loopIdentity);
		assert(it != loops.end()); // TODO compiler error: break outside loop

		auto cutPoint = cut();
		it->second.endConsumers.push_back([first{cutPoint.first}](auto end){
			join(first, end);
		});
	}

	void leave(std::vector<std::shared_ptr<Temporary>> retvals) {
		join(cut().first, retvals);
	}

	auto build()
	{
		assert(last->code.empty());
		assert(!last->termination);

		return entry;
	}
};

} // namespace ir
} // namespace comp

#endif /* COMPILER_IR_IRBUILDER_H_ */
