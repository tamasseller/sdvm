#include "TestUtils.h"

#include "Builder.h"

#include <sstream>

TEST_GROUP(Builder) {};

TEST(Builder, DoublePlusThree)
{
	Builder b;

	auto &f = b.function(1);
	auto l = f.local();
	auto m = f.local();
	auto a = f.arg();

	f.set(l, a + a);
	f.set(l, l + 3);
	f.set(m, 3 + l);
	f.ret(l);

	doRunTest({1}, {5}, b.make());
}
