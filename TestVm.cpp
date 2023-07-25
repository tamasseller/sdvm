#include "1test/Test.h"

#include "Vm.h"

TEST_GROUP(Vm)
{
	const Type emptyType = { .length = 0, .refOffs = {} };
	const Type minimalFrame = { .length = 2, .refOffs = {0} };
	Storage storage;
};

TEST(Vm, Sanity)
{
	Program p = {
		.staticType = &emptyType,
		.functions = {
			 {
				.frameType = &minimalFrame,
				.argOffsets = {},
				.retOffset = {},
				.code = {
					Instruction::literal(1),
					Instruction::literal(2),
					Instruction::binary(Instruction::BinaryOpType::AddI),
				}
			}
		}
	};

	Vm(storage, p).run({});
}
