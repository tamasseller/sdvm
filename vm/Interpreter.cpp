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
	if(f.nLocals + f.nArgs <= idx)
	{
		throw std::runtime_error("Local load out of bounds (" + std::to_string(idx) + " should be below " + std::to_string(f.nLocals) + ")");
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
	if(f.nLocals + f.nArgs <= idx)
	{
		throw std::runtime_error("Local store out of bounds (" + std::to_string(idx) + " should be below " + std::to_string(f.nLocals) + ")");
	}

	locals[idx] = v;
}

inline Program::Body::const_iterator Interpreter::Frame::processBlock(Program::Body::const_iterator bit)
{
	for(auto iit = bit->cbegin(); iit != bit->cend(); ++iit)
	{
		switch(iit->op)
		{
			case Program::Operation::Immediate:
				push(iit->arg);
				break;
			case Program::Operation::Duplicate:
			{
				const auto tos = pop();
				push(tos);
				push(tos);
				break;
			}
			case Program::Operation::Load:
				push(load(iit->arg));
				break;
			case Program::Operation::Store:
				store(iit->arg, pop());
				break;
			case Program::Operation::Add:
				push(pop() + pop());
				break;
			case Program::Operation::Sub:
			{
				const auto arg2 = pop();
				const auto arg1 = pop();
				push(arg1 - arg2);
				break;
			}
			case Program::Operation::Mul:
				push(pop() * pop());
				break;
			case Program::Operation::Div:
			{
				const auto arg2 = pop();
				const auto arg1 = pop();
				push(arg1 / arg2);
				break;
			}
			case Program::Operation::Mod:
			{
				const auto arg2 = pop();
				const auto arg1 = pop();
				push(arg1 % arg2);
				break;
			}
			case Program::Operation::And:
				push(pop() & pop());
				break;
			case Program::Operation::Or:
				push(pop() | pop());
				break;
			case Program::Operation::Xor:
				push(pop() ^ pop());
				break;
			case Program::Operation::Lshift:
			{
				const auto arg2 = pop();
				const auto arg1 = pop();
				push(arg1 << arg2);
				break;
			}
			case Program::Operation::Rshift:
			{
				const auto arg2 = pop();
				const auto arg1 = pop();
				push(arg1 >> arg2);
				break;
			}
			case Program::Operation::SkipIfTrue:
				if(pop())
				{
					++iit;

					if(iit == bit->cend())
					{
						throw std::runtime_error("Skip at end of block");
					}
				}
				break;
			case Program::Operation::Jump:
				return f.body.cbegin() + iit->arg;
			case Program::Operation::Call:
			{
				const auto &f = p.functions[pop()];
				Frame(p, f, sp - f.nArgs, sp).run();
				break;
			}
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
