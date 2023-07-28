#include "1test/Test.h"

#include "compiler/ProgramBuilder.h"
#include "vm/Vm.h"

TEST_GROUP(Vm)
{
	obj::Storage storage;
};

TEST(Vm, Sanity)
{
	comp::ProgramBuilder b;

	auto f = b.fun(comp::Type::integer(), {comp::Type::integer(), comp::Type::integer()});
	f->ret(f->addi(f->arg(0), f->arg(1)));

	CHECK(3 == vm::Vm(storage, b()).run({1, 2}).value().integer);
}
