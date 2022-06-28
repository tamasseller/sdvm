#include "TestUtils.h"

#include "jit/Compiler.h"

class MockFunction
{
	Bytecode::FunctionInfo info;
	std::vector<Bytecode::Instruction> body;

public:
	inline MockFunction(const Bytecode::FunctionInfo &info, const std::vector<Bytecode::Instruction> &body): info(info), body(body)	{}

	class Reader: public Bytecode::InstructionStreamReader
	{
		decltype(body)::iterator it, end;

		static bool read(Bytecode::InstructionStreamReader *isr, Bytecode::Instruction& isn)
		{
			auto self = static_cast<Reader>(isr);

			if(it != end)
			{
				isn = *it++;
				return true;
			}

			return false;
		}
	public:
		inline Reader(MockFunction& f): InstructionStreamReader(&read), it(f.body.begin()), end(f.body.end()) {}
	};
};

TEST_GROUP(Compiler){};

TEST(Compiler, Sanity)
{
	uint16_t out[512];
	MockFunction f
	{
		Bytecode::FunctionInfo{
			.nLabels = 0,
			.nRet = 1,
			.hasNonTailCall = false
		},
		{
			Bytecode::ldArg(0),
			Bytecode::ldLoc(0),
			Bytecode::mul(),
			Bytecode::ret()
		}
	};
	MockFunction::Reader r(f);
	auto end = Compiler::compile(0, {out, sizeof(out) / sizeof(out[0])}, f.info, r);


}
