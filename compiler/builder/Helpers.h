#ifndef COMPILER_HELPERS_H_
#define COMPILER_HELPERS_H_

#include "ValueWrappers.h"

#include "StatementSink.h"

#include "compiler/model/Break.h"
#include "compiler/model/Loop.h"
#include "compiler/model/Field.h"
#include "compiler/model/Unary.h"
#include "compiler/model/Binary.h"
#include "compiler/model/Create.h"
#include "compiler/model/Return.h"
#include "compiler/model/Ternary.h"
#include "compiler/model/Literal.h"
#include "compiler/model/Continue.h"
#include "compiler/model/Dereference.h"
#include "compiler/model/ExpressionStatement.h"

#include "assert.h"

namespace comp {

inline RValWrapper null(std::make_shared<Create>(nullptr));

inline auto declaration(ValueType type, const RValWrapper& initializer)
{
	return [type, initializer{initializer.val}](std::shared_ptr<StatementSink>& sink) -> LValWrapper
	{
		auto ret = sink->addLocal(type);
		sink->add(std::make_shared<Declaration>(ret, initializer));
		return {ret};
	};
}

inline auto declaration(const RValWrapper& initializer) {
	return declaration(initializer.val->getType(), initializer);
}

inline RValWrapper ternary(const RValWrapper& condition, const RValWrapper& then, const RValWrapper& otherwise) {
	return {std::make_shared<Ternary>(condition.val, then.val, otherwise.val)};
}

inline auto conditional(const RValWrapper& condition)
{
	return [condition{condition.val}](std::shared_ptr<StatementSink>& sink)
	{
		auto conditional = std::make_shared<Conditional>(condition);
		sink->add(conditional);
		sink = std::make_shared<ConditionalSink>(conditional, sink);
	};
}

inline auto otherwise()
{
	return [](std::shared_ptr<StatementSink>& sink)
	{
		auto csink = std::dynamic_pointer_cast<ConditionalSink>(sink);
		assert(csink != nullptr);
		sink = std::make_shared<BlockSink>(csink->conditional->otherwise, csink->parent);
	};
}

inline auto endBlock()
{
	return [](std::shared_ptr<StatementSink>& sink)
	{
		sink = sink->parent;
	};
}

inline auto loop()
{
	return [](std::shared_ptr<StatementSink>& sink)
	{
		auto loop = std::make_shared<Loop>();
		sink->add(loop);
		sink = std::make_shared<LoopSink>(loop, sink);
	};
}

static inline auto findLoop(std::shared_ptr<StatementSink> current)
{
	std::shared_ptr<LoopSink> lsink;

	while(current)
	{
		lsink = std::dynamic_pointer_cast<LoopSink>(current);

		if(lsink) {
			break;
		}

		current = current->parent;
	}

	assert(lsink != nullptr);
	return lsink->loop;
}

inline auto exitLoop()
{
	return [](std::shared_ptr<StatementSink>& sink) {
		sink->add(std::make_shared<Break>(findLoop(sink)));
	};
}

inline auto restartLoop()
{
	return [](std::shared_ptr<StatementSink>& sink) {
		sink->add(std::make_shared<Continue>(findLoop(sink)));
	};
}

inline auto ret()
{
	return [](std::shared_ptr<StatementSink>& sink)
	{
		return sink->add(std::make_shared<Return>());
	};
}

inline auto ret(const RValWrapper& val)
{
	return [val{val.val}](std::shared_ptr<StatementSink>& sink)
	{
		std::vector<std::shared_ptr<RValue>> a;
		a.push_back(val);
		return sink->add(std::make_shared<Return>(a));
	};
}

} // namespace comp

#endif /* COMPILER_HELPERS_H_ */
