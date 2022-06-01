#include "Interpreter.h"

#include <stdexcept>

inline Interpreter::Frame::Frame(const Program& p, const Program::Function& f, const uint32_t* argsBegin, const uint32_t* argsEnd):
	p(p), f(f), locals(new uint32_t[f.nLocals + f.nArgs]), stack(new uint32_t[f.maxStack]), sp(stack.get())
{
	if(const auto nArgsAct = argsEnd - argsBegin; nArgsAct != f.nArgs)
	{
		throw std::runtime_error("Argument number mismatch at invocation (" + std::to_string(nArgsAct) + " instead of " + std::to_string(f.nArgs) + ")");
	}

	std::copy(argsBegin, argsEnd, locals.get());
}

inline uint32_t Interpreter::Frame::pop()
{
	if(sp <= stack.get())
	{
		throw std::runtime_error("Stack underflow");
	}

	return *--sp;
}

inline uint32_t Interpreter::Frame::load(uint32_t idx)
{
	if(	const auto limit = f.nLocals + f.nArgs; limit <= idx)
	{
		throw std::runtime_error("Local load out of bounds (" + std::to_string(idx) + " should be below " + std::to_string(limit) + ")");
	}

	return locals[idx];
}

inline void Interpreter::Frame::push(uint32_t v)
{
	if(const auto idx = sp - stack.get(); f.maxStack <= idx)
	{
		throw std::runtime_error("Stack overflow");
	}

	*sp++ = v;
}

inline void Interpreter::Frame::store(uint32_t idx, uint32_t v)
{
	if(	const auto limit = f.nLocals + f.nArgs; limit <= idx)
	{
		throw std::runtime_error("Local store out of bounds (" + std::to_string(idx) + " should be below " + std::to_string(limit) + ")");
	}

	locals[idx] = v;
}

inline Program::Body::const_iterator Interpreter::Frame::processBlock(Program::Body::const_iterator bit)
{
	for(auto iit = bit->cbegin(); iit != bit->cend(); ++iit)
	{
		switch(iit->group)
		{
			case Program::OperationGroup::Immediate:
				push(iit->value);
				break;

			case Program::OperationGroup::Duplicate:
			{
				const auto tos = pop();
				push(tos);
				push(tos);
				break;
			}

			case Program::OperationGroup::LoadStore:
				switch(iit->lsOp)
				{
					case Program::LoadStoreOperation::LoadLocal:
						push(load(iit->varIdx));
						break;
					case Program::LoadStoreOperation::StoreLocal:
						store(iit->varIdx, pop());
						break;
					default: throw std::runtime_error("Unknown memory operation");
				}
				break;

			case Program::OperationGroup::Binary:
			{
				const auto arg2 = pop();
				const auto arg1 = pop();

				switch(iit->binOp)
				{
					case Program::BinaryOperation::Add: push(arg1 + arg2); break;
					case Program::BinaryOperation::Sub: push(arg1 - arg2); break;
					case Program::BinaryOperation::Mul: push(arg1 * arg2); break;
					case Program::BinaryOperation::Div: push(arg1 / arg2); break;
					case Program::BinaryOperation::Mod: push(arg1 % arg2); break;
					case Program::BinaryOperation::And: push(arg1 & arg2); break;
					case Program::BinaryOperation::Ior: push(arg1 | arg2); break;
					case Program::BinaryOperation::Xor: push(arg1 ^ arg2); break;
					case Program::BinaryOperation::Lsh: push(arg1 << arg2); break;
					case Program::BinaryOperation::Rsh: push(arg1 >> arg2); break;
					default: throw std::runtime_error("Unknown binary operation");
				}
				break;
			}

			case Program::OperationGroup::Conditional:
			{
				const auto arg2 = pop();
				const auto arg1 = pop();

				bool doIt = false;

				switch(iit->cond)
				{
					case Program::BranchCondition::IfEqual:      doIt = arg1 == arg2; break;
					case Program::BranchCondition::IfGreater:    doIt = arg1 > arg2; break;
					case Program::BranchCondition::IfLess:       doIt = arg1 < arg2; break;
					case Program::BranchCondition::IfNotEqual:   doIt = arg1 != arg2; break;
					case Program::BranchCondition::IfNotGreater: doIt = arg1 <= arg2; break;
					case Program::BranchCondition::IfNotLess:    doIt = arg1 >= arg2; break;
					default: throw std::runtime_error("Unknown branch condition");
				}

				if(doIt)
				{
					if(bit->size() <= iit->targetBlockIdx)
					{
						throw std::runtime_error("Jump to unknown block: " + std::to_string(iit->targetBlockIdx) + " (should be below " + std::to_string(bit->size()));
					}

					return f.body.cbegin() + iit->targetBlockIdx;
				}
				break;
			}

			case Program::OperationGroup::Call:
			{
				auto idx = pop();

				if(p.functions.size() <= idx)
				{
					throw std::runtime_error("Unknown function called: " + std::to_string(idx) + " (should be below " + std::to_string(p.functions.size()));
				}

				const auto &f = p.functions[idx];

				if(const auto nActArgs = sp - stack.get(); nActArgs < f.nArgs)
				{
					throw std::runtime_error("Function called with insufficient number of arguments: " + std::to_string(nActArgs) + " (should be " + std::to_string(f.nArgs) + ")");
				}

				auto ret = Frame(p, f, sp - f.nArgs, sp).run();

				sp -= f.nArgs;
				for(const auto v: ret)
				{
					push(v);
				}

				break;
			}

			case Program::OperationGroup::Jump:
				if(bit->size() <= iit->targetBlockIdx)
				{
					throw std::runtime_error("Jump to unknown block: " + std::to_string(iit->targetBlockIdx) + " (should be below " + std::to_string(bit->size()));
				}

				return f.body.cbegin() + iit->targetBlockIdx;

			default:
				throw std::runtime_error("Unknown instruction group");
		}
	}

	return bit + 1;
}

inline std::vector<uint32_t> Interpreter::Frame::run()
{
	for(auto blockIt = f.body.cbegin(); blockIt != f.body.cend(); blockIt = processBlock(blockIt)){}
	return {stack.get(), sp};
}

std::vector<uint32_t> Interpreter::run(const std::vector<uint32_t> &args)
{
	return Frame(program, program.functions[0], args.data(), args.data() + args.size()).run();
}
