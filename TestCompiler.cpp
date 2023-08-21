#include "1test/Test.h"

#include "compiler/FunctionBuilder.h"
#include "compiler/Helpers.h"

TEST_GROUP(Compiler) {};

TEST(Compiler, Sanity)
{
	auto uut = comp::FunctionBuilder::make({}, {});

	auto i = uut <<= comp::declaration(comp::ValueType::integer());
	uut <<= i = i + 1;

	auto p = uut.compile();
}

TEST(Compiler, Call)
{
	auto g = comp::FunctionBuilder::make({}, {});
	g <<= comp::ret();

	auto f = comp::FunctionBuilder::make({}, {});
	f <<= g();
	f <<= comp::ret();

	auto p = f.compile();
}
