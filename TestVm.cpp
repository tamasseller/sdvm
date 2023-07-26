#include "1test/Test.h"

#include "ProgramBuilder.h"

TEST_GROUP(Vm)
{
	Storage storage;
};

TEST(Vm, Sanity)
{
	ProgramBuilder b;

	b.fun<int(int, int)>([](auto& fb){
		fb.ret(fb.add(fb.template argVal<0>(), fb.template argVal<1>()));
	});

	CHECK(3 == Vm(storage, b).run({1, 2}).value().integer);
}
