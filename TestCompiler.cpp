#include "1test/Test.h"

#include "compiler/builder/FunctionBuilder.h"
#include "compiler/builder/ClassBuilder.h"
#include "compiler/builder/Helpers.h"

TEST_GROUP(Compiler) {};

TEST(Compiler, Sanity)
{
	auto uut = comp::FunctionBuilder::make({}, {});

	auto i = uut <<= comp::declaration(comp::ValueType::integer(), 0);
	uut <<= i = i + 1;

	auto p = uut.compile();

//	CHECK(p.functions.size() == 1);
}

TEST(Compiler, Call)
{
	auto h = comp::FunctionBuilder::make({comp::ValueType::integer()}, {});
	h <<= comp::ret(12);

	auto g = comp::FunctionBuilder::make({comp::ValueType::integer()}, {comp::ValueType::integer()});

	g <<= comp::ret(g[0] + 3);

	auto f = comp::FunctionBuilder::make({}, {});
	auto i = f <<= comp::declaration(g(h()));
	f <<= i * 2;
	f <<= comp::ret();

	auto p = f.compile();

//	CHECK(p.functions.size() == 2);
}

TEST(Compiler, ObjectUsage)
{
	auto c = comp::ClassBuilder::make();
	auto sfInst = c.addStaticField(c);
	auto fData = c.addField(comp::ValueType::integer());
	auto fNext = c.addField(c);

	auto f = comp::FunctionBuilder::make({}, {});
	auto l = f <<= comp::declaration(c());
	f <<= l[fNext] = comp::null;
	f <<= l[fData] = 123;
	f <<= c[sfInst] = l;
	f <<= comp::ret();

	auto p = f.compile();

//	CHECK(p.functions.size() == 2);
}

TEST(Compiler, Ternary)
{
	auto f = comp::FunctionBuilder::make({comp::ValueType::integer()}, {comp::ValueType::integer()});
	f <<= comp::ret(comp::ternary(f[0] > 2, f(f[0] - 1) * f[0], 1));
	auto p = f.compile();

//	CHECK(p.functions.size() == 2);
}

TEST(Compiler, Conditional)
{
	auto f = comp::FunctionBuilder::make({comp::ValueType::integer()}, {comp::ValueType::integer()});

	f <<= comp::conditional(f[0] > 2);
	f <<= 	comp::ret(f(f[0] - 1) * f[0]);
	f <<= comp::otherwise();
	f <<= 	comp::ret(1);
	f <<= comp::endBlock();

	auto p = f.compile();

//	CHECK(p.functions.size() == 2);
}

TEST(Compiler, Loop)
{
	auto f = comp::FunctionBuilder::make({comp::ValueType::integer()}, {comp::ValueType::integer()});

	auto ret = f <<= comp::declaration(1);
	f <<= comp::loop();
	f <<= 	comp::conditional(!(f[0] > 2));
	f <<= 		comp::exitLoop();
	f <<= 	comp::endBlock();

	f <<= 	ret = ret * f[0];
	f <<= 	f[0] = f[0] - 1;
	f <<= comp::endBlock();
	f <<= comp::ret(ret);

	auto p = f.compile();

//	CHECK(p.functions.size() == 2);
}
