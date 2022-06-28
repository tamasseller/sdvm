#include "TestUtils.h"

#include "jit/Compiler.h"

#include "Disassembler.h"

class MockFunction
{
	std::vector<Bytecode::Instruction> body;

public:
	const Bytecode::FunctionInfo info;
	inline MockFunction(const Bytecode::FunctionInfo &info, const std::vector<Bytecode::Instruction> &body): body(body), info(info)	{}

	class Reader: public Bytecode::InstructionStreamReader
	{
		decltype(body)::const_iterator it, end;

		static bool read(Bytecode::InstructionStreamReader *isr, Bytecode::Instruction& isn)
		{
			auto self = static_cast<Reader*>(isr);

			if(self->it != self->end)
			{
				isn = *self->it++;
				return true;
			}

			return false;
		}
	public:
		inline Reader(const MockFunction& f): InstructionStreamReader(&read), it(f.body.cbegin()), end(f.body.cend()) {}
	};
};

TEST_GROUP(Compiler)
{
	static inline auto compile(const MockFunction& f)
	{
		uint16_t out[512];
		MockFunction::Reader r(f);
		auto end = Compiler::compile(0, {out, sizeof(out) / sizeof(out[0])}, f.info, r);
		return Disassembler::disassemble(out, end);
	}
};

TEST(Compiler, Sanity)
{
	auto result = compile(MockFunction
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
	});

}
