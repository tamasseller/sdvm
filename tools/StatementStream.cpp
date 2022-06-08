#include "StatementStream.h"

#include <cassert>

void StatementStream::InstructionSink::takeNonControl(Visa::Instruction isn)
{
	uint32_t balance;
	bool ok = Visa::stackBalance(isn.group, balance);
	assert(ok);
	stackLevel += balance;
	maxStack = std::max(maxStack, stackLevel);
	body.back().push_back(isn);
}

void StatementStream::InstructionSink::takeCall(uint32_t nCalleArgs, uint32_t nCalleRet)
{
	assert(nCalleArgs <= stackLevel);
	stackLevel += nCalleRet - nCalleArgs;
	maxStack = std::max(maxStack, stackLevel);
	body.back().push_back(Visa::Instruction::call());
}

void StatementStream::InstructionSink::takeRet(uint32_t nRet)
{
	assert(nRet == stackLevel);
	body.back().push_back(Visa::Instruction::ret());
}

StatementStream& StatementStream::operator <<(const Statement& stmt)
{
	code.push_back(stmt);
	return *this;
}

Program::Body StatementStream::render(uint32_t &maxStack) const
{
	InstructionSink s;

	for(const auto& w: code)
	{
		w(s);
	}

	maxStack = s.maxStack;
	return s.body;
}

