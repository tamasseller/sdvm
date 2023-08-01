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

	auto f = b.fun(comp::ValueType::integer(), {comp::ValueType::integer(), comp::ValueType::integer()});
	f->ret(f->addi(f->arg(0), f->arg(1)));

	CHECK(3 == vm::Vm(storage, b()).run({1, 2}).value().integer);
}
/*
TEST(Vm, ClassBuilder)
{
	comp::ProgramBuilder b;

	auto t = b.type();
	auto v = t->addField(comp::ValueType::integer());
	auto n = t->addField(t);

	auto f = b.fun({}, {});
	auto l = f->addLocal(t);
	f->setLocal(l, f->create(t));

	const auto p = b();
	CHECK(3 == vm::Vm(storage, p).run({}).value().integer);
}
*/
