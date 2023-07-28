#include "1test/Test.h"

#include "ProgramBuilder.h"

TEST_GROUP(Vm)
{
	Storage storage;
};

TEST(Vm, Sanity)
{
	ProgramBuilder b;

	b.fun(SourceType::integer(), {SourceType::integer(), SourceType::integer()}, [](auto& fb){
		fb.ret(fb.addi(fb.arg(0), fb.arg(1)));
	});

	CHECK(3 == Vm(storage, b).run({1, 2}).value().integer);
}
