#ifndef TOOLS_STATEMENTSTREAM_H_
#define TOOLS_STATEMENTSTREAM_H_

#include "Program.h"

#include <functional>

struct StatementStream
{
	class InstructionSink
	{
		friend StatementStream;

		uint32_t stackLevel = 0, maxStack = 0;
		Program::Body body = {{}};
		InstructionSink() = default;

	public:
		void takeNonControl(Visa::Instruction isn);
		void takeCall(uint32_t nCalleArgs, uint32_t nCalleRet);
		void takeRet(uint32_t nRet);
	};

	using Statement = std::function<void(InstructionSink&)>;

	StatementStream& operator <<(const Statement& stmt);
	Program::Body render(uint32_t &maxStack) const;

private:
	std::vector<Statement> code;
};

#endif /* TOOLS_STATEMENTSTREAM_H_ */
