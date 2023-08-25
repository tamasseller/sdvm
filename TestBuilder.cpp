#include "1test/Test.h"

#include "compiler/builder/FunctionBuilder.h"
#include "compiler/builder/ClassBuilder.h"
#include "compiler/builder/Helpers.h"

#include <iostream>

TEST_GROUP(Builder) {};

TEST(Builder, Sanity)
{
	auto uut = comp::FunctionBuilder::make({}, {});

	auto i = uut <<= comp::declaration(comp::ValueType::integer(), 0);
	uut <<= i = i + 1;

	CHECK(std::string("\n") + uut.build().dumpAst() == R"(
struct c0
{
};

void f0()
{
    int l0 = 0;
    l0 = l0 + 1;
}
)");
}

TEST(Builder, Call)
{
	auto h = comp::FunctionBuilder::make({comp::ValueType::integer()}, {});
	h <<= comp::ret(12);

	auto g = comp::FunctionBuilder::make({comp::ValueType::integer()}, {comp::ValueType::integer()});

	g <<= comp::ret(g[0] + 3);

	auto f = comp::FunctionBuilder::make({}, {});
	auto i = f <<= comp::declaration(g(h()));
	f <<= i * 2;
	f <<= comp::ret();

	CHECK(std::string("\n") + f.build().dumpAst() == R"(
struct c0
{
};

void f0()
{
    int l0 = f2(f1());
    l0 * 2;
    return;
}

int f1()
{
    return 12;
}

int f2(int a0)
{
    return a0 + 3;
}
)");
}

TEST(Builder, ObjectUsage)
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

	CHECK(std::string("\n") + f.build().dumpAst() == R"(
struct c0
{
  c1* f0;
};

struct c1
{
  static c1* s0;
  int f0;
  c1* f1;
};

void f0()
{
    c1* l0 = new c1;
    l0->f1 = nullptr;
    l0->f0 = 123;
    c1::s0 = l0;
    return;
}
)");
}

TEST(Builder, Ternary)
{
	auto f = comp::FunctionBuilder::make({comp::ValueType::integer()}, {comp::ValueType::integer()});
	f <<= comp::ret(comp::ternary(f[0] >= 2, f(f[0] - 1) * f[0], 1));

	CHECK(std::string("\n") + f.build().dumpAst() == R"(
struct c0
{
};

int f0(int a0)
{
    return a0 >= 2 ? f0(a0 - 1) * a0 : 1;
}
)");
}

TEST(Builder, Conditional)
{
	auto f = comp::FunctionBuilder::make({comp::ValueType::integer()}, {comp::ValueType::integer()});

	f <<= comp::conditional(f[0] > 2);
	f <<= 	comp::ret(f(f[0] - 1) * f[0]);
	f <<= comp::otherwise();
	f <<= 	comp::ret(1);
	f <<= comp::endBlock();

	CHECK(std::string("\n") + f.build().dumpAst() == R"(
struct c0
{
};

int f0(int a0)
{
    if(a0 > 2)
    {
        return f0(a0 - 1) * a0;
    }
    else
    {
        return 1;
    }
}
)");
}

TEST(Builder, Loop)
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

	CHECK(std::string("\n") + f.build().dumpAst() == R"(
struct c0
{
};

int f0(int a0)
{
    int l0 = 1;
    while(true)
    {
        if(!(a0 > 2))
        {
            break;
        }
        l0 = l0 * a0;
        a0 = a0 - 1;
    }
    return l0;
}
)");
}
