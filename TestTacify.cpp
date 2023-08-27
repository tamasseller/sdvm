#include "1test/Test.h"

#include "compiler/builder/FunctionBuilder.h"
#include "compiler/builder/ClassBuilder.h"
#include "compiler/builder/Helpers.h"

#include <iostream>

TEST_GROUP(Tacify) {};
#if 0
TEST(Tacify, Sanity)
{
	auto uut = comp::FunctionBuilder::make({comp::ValueType::integer()}, {comp::ValueType::integer()});
	uut <<= comp::ret(uut[0] * (uut[0] + 1) / 2);

	std::cout << uut.build().dumpTac() << std::endl;
}

TEST(Tacify, Lhs)
{
	auto uut = comp::FunctionBuilder::make({}, {});
	auto c = comp::ClassBuilder::make();
	auto f = c.addField(c);

	auto i = uut <<= comp::declaration(c());
	uut <<= i[f] = i;
	uut <<= i[f][f][f] = i[f][f];

	std::cout << uut.build().dumpTac() << std::endl;
}
#endif

TEST(Tacify, Ternary)
{
	auto uut = comp::FunctionBuilder::make({comp::ValueType::integer()}, {comp::ValueType::integer()});
	uut <<= comp::ret(comp::ternary(uut[0] >= 2, uut(uut[0] - 1) * uut[0], 1));

	std::cout << uut.build().dumpTac() << std::endl;
}
