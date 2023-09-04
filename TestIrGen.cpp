#include "1test/Test.h"

#include "compiler/builder/FunctionBuilder.h"
#include "compiler/builder/ClassBuilder.h"
#include "compiler/builder/Helpers.h"

#include <iostream>

TEST_GROUP(Tacify) {
	static inline constexpr auto flags =
			comp::Compiler::Options::doJumpOptimizations |
			comp::Compiler::Options::propagateConstants;
};

#if 0
TEST(Tacify, Sanity)
{
	auto uut = comp::FunctionBuilder::make({comp::ast::ValueType::integer()}, {comp::ast::ValueType::integer()});
	uut <<= comp::ret(uut[0] * (uut[0] + 1) / 2);

	std::cout << uut.build().dumpCfg() << std::endl;
}

TEST(Tacify, Lhs)
{
	auto uut = comp::FunctionBuilder::make({}, {});
	auto c = comp::ClassBuilder::make();
	auto f = c.addField(c);

	auto i = uut <<= comp::declaration(c());
	uut <<= i[f] = i;
	uut <<= i[f][f][f] = i[f][f];
	uut <<= comp::ret();

	std::cout << uut.build().dumpCfg() << std::endl;
}

TEST(Tacify, Ternary)
{
	auto uut = comp::FunctionBuilder::make({comp::ast::ValueType::integer()}, {comp::ast::ValueType::integer()});
	uut <<= comp::ret(comp::ternary(uut[0] >= 2, uut(uut[0] - 1) * uut[0], 1));

	std::cout << uut.build().dumpCfg() << std::endl;
}

TEST(Tacify, Conditional)
{
	auto uut = comp::FunctionBuilder::make({comp::ast::ValueType::integer()}, {comp::ast::ValueType::integer()});
	uut <<= comp::conditional(uut[0] < 0);
	uut <<= comp::ret(-uut[0]);
	uut <<= comp::otherwise();
	uut <<= comp::ret(uut[0]);
	uut <<= comp::endBlock();

	std::cout << uut.build().dumpCfg(flags) << std::endl;
}
#endif
#if 1
TEST(Tacify, Loop)
{
	auto uut = comp::FunctionBuilder::make({comp::ast::ValueType::integer()}, {comp::ast::ValueType::integer()});

	auto r = uut <<= comp::declaration(1);
	uut <<= comp::loop();
	uut <<= 	comp::conditional(!(uut[0] > 2));
	uut <<= 		comp::exitLoop();
	uut <<= 	comp::endBlock();

	uut <<= 	r = r * uut[0];
	uut <<= 	uut[0] = uut[0] - 1;
	uut <<= comp::endBlock();
	uut <<= comp::ret(r);

	std::cout << uut.build().dumpCfg(flags) << std::endl;
}
#endif
