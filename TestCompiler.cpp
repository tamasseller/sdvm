#include "1test/Test.h"

#include "compiler/FunctionBuilder.h"

TEST_GROUP(Compiler) {};

TEST(Compiler, Sanity)
{
	auto uut = comp::FunctionBuilder::make({}, {});

	auto i = uut(comp::declaration(comp::ValueType::integer()));
	uut(i = i + 1);
}
