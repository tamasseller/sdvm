#include "1test/Test.h"

#include "vm/Vm.h"

#include <tuple>

static const std::pair<int, int> factorialTestVectors[] =
{
	{0, 1},
	{1, 1},
	{2, 2},
	{3, 6},
	{4, 24},
	{5, 120},
	{6, 720},
	{7, 5040},
	{8, 40320},
	{9, 362880}
};

static const prog::TypeInfo listElement(0, 1, 1);

TEST_GROUP(Vm)
{
	vm::Storage storage;

	void binaryTest(prog::Instruction isn, vm::Value arg1, vm::Value arg2, vm::Value ret)
	{
		prog::Program p =
		{
			.types = {prog::TypeInfo::empty},
			.functions =
			{
				prog::Function
				{
					.nRefs = 0,
					.nScalars = 2,
					.code = {
						isn,
						prog::Instruction::ret(0, 1)
					}
				}
			}
		};

		CHECK(ret.integer == vm::Vm(storage, p).run({}, {arg1, arg2}).second.front().integer);
	}

	void unaryTest(prog::Instruction isn, vm::Value arg, vm::Value ret)
	{
		prog::Program p =
		{
			.types = {prog::TypeInfo::empty},
			.functions =
			{
				prog::Function
				{
					.nRefs = 0,
					.nScalars = 1,
					.code = {
						isn,
						prog::Instruction::ret(0, 1)
					}
				}
			}
		};

		CHECK(ret.integer == vm::Vm(storage, p).run({}, {arg}).second.front().integer);
	}

	void conditionalTest(prog::Instruction isn, vm::Value arg1, vm::Value arg2, bool ret)
	{
		prog::Program p =
		{
			.types = {prog::TypeInfo::empty},
			.functions =
			{
				prog::Function
				{
					.nRefs = 0,
					.nScalars = 2,
					.code = {
						isn,
						prog::Instruction::lit({}, 0),
						prog::Instruction::ret(0, 1),
						prog::Instruction::lit({}, 1),
						prog::Instruction::ret(0, 1)
					}
				}
			}
		};

		CHECK(ret == vm::Vm(storage, p).run({}, {arg1, arg2}).second.front().logical);
	}
};

TEST(Vm, Sanity)
{
	prog::Program p = {
		.types = {prog::TypeInfo::empty},
		.functions =
		{
			prog::Function
			{
				.nRefs = 0,
				.nScalars = 1,
				.code = {
					prog::Instruction::lit({}, 420),
					prog::Instruction::ret(0, 1)
				}
			}
		}
	};

	CHECK(420 == vm::Vm(storage, p).run({}, {}).second.front().integer);
}

TEST(Vm, BinaryOpAddI) { binaryTest(prog::Instruction::addI({}, {}, {}), 1, 2, 3); }
TEST(Vm, BinaryOpSubI) { binaryTest(prog::Instruction::subI({}, {}, {}), 2, 3, -1); }
TEST(Vm, BinaryOpMulI) { binaryTest(prog::Instruction::mulI({}, {}, {}), 3, 4, 12); }
TEST(Vm, BinaryOpDivI) { binaryTest(prog::Instruction::divI({}, {}, {}), 12, 3, 4); }
TEST(Vm, BinaryOpModI) { binaryTest(prog::Instruction::mod ({}, {}, {}), 10, 3, 1); }
TEST(Vm, BinaryOpShlI) { binaryTest(prog::Instruction::shlI({}, {}, {}), 3, 3, 24); }
TEST(Vm, BinaryOpShrI1) { binaryTest(prog::Instruction::shrI({}, {}, {}), 23, 2, 5); }
TEST(Vm, BinaryOpShrI2) { binaryTest(prog::Instruction::shrI({}, {}, {}), -23, 2, -6); }
TEST(Vm, BinaryOpShrU1) { binaryTest(prog::Instruction::shrU({}, {}, {}), 345, 5, 10); }
TEST(Vm, BinaryOpShtU2) { binaryTest(prog::Instruction::shrU({}, {}, {}), -345, 5, 134217717); }
TEST(Vm, BinaryOpAndI) { binaryTest(prog::Instruction::andI({}, {}, {}), 6, 12, 4); }
TEST(Vm, BinaryOpOrI) { binaryTest(prog::Instruction::orI ({}, {}, {}), 6, 12, 14); }
TEST(Vm, BinaryOpXorI) { binaryTest(prog::Instruction::xorI({}, {}, {}), 6, 12, 10); }
TEST(Vm, BinaryOpAddF) { binaryTest(prog::Instruction::addF({}, {}, {}), 0.125f, 2.0f, 2.125f); }
TEST(Vm, BinaryOpMulF) { binaryTest(prog::Instruction::mulF({}, {}, {}), 1.25f, 0.625f, 0.78125f); }
TEST(Vm, BinaryOpSubF) { binaryTest(prog::Instruction::subF({}, {}, {}), 3.75f, 2.625f, 1.125f); }
TEST(Vm, BinaryOpDivF) { binaryTest(prog::Instruction::divF({}, {}, {}), 13.0f, 0.875f, 14.8571424f); }

TEST(Vm, UnaryOpMov) { unaryTest(prog::Instruction::mov({}, {}), 69, 69); }
TEST(Vm, UnaryOpNeg) { unaryTest(prog::Instruction::neg({}, {}), 69, ~69); }
TEST(Vm, UnaryOpI2f) { unaryTest(prog::Instruction::i2f({}, {}), 69, 69.0f); }
TEST(Vm, UnaryOpF2i) { unaryTest(prog::Instruction::f2i({}, {}), 69.0f, 69); }

TEST(Vm, ConditionalEq1) { conditionalTest(prog::Instruction::jEq({}, {}, 3), 69, 69, true); }
TEST(Vm, ConditionalEq2) { conditionalTest(prog::Instruction::jEq({}, {}, 3), 69, 420, false); }
TEST(Vm, ConditionalNe1) { conditionalTest(prog::Instruction::jNe({}, {}, 3), 69, 69, false); }
TEST(Vm, ConditionalNe2) { conditionalTest(prog::Instruction::jNe({}, {}, 3), 69, 420, true); }

TEST(Vm, ConditionalLtI1) { conditionalTest(prog::Instruction::jLtI({}, {}, 3), 69, 69, false); }
TEST(Vm, ConditionalLtI2) { conditionalTest(prog::Instruction::jLtI({}, {}, 3), 69, 420, true); }
TEST(Vm, ConditionalLtI3) { conditionalTest(prog::Instruction::jLtI({}, {}, 3), 420, 69, false); }
TEST(Vm, ConditionalLtI4) { conditionalTest(prog::Instruction::jLtI({}, {}, 3), -1, 0, true); }

TEST(Vm, ConditionalGtI1) { conditionalTest(prog::Instruction::jGtI({}, {}, 3), 69, 69, false); }
TEST(Vm, ConditionalGtI2) { conditionalTest(prog::Instruction::jGtI({}, {}, 3), 69, 420, false); }
TEST(Vm, ConditionalGtI3) { conditionalTest(prog::Instruction::jGtI({}, {}, 3), 420, 69, true); }
TEST(Vm, ConditionalGtI4) { conditionalTest(prog::Instruction::jGtI({}, {}, 3), 0, -1, true); }

TEST(Vm, ConditionalLeI1) { conditionalTest(prog::Instruction::jLeI({}, {}, 3), 69, 69, true); }
TEST(Vm, ConditionalLeI2) { conditionalTest(prog::Instruction::jLeI({}, {}, 3), 69, 420, true); }
TEST(Vm, ConditionalLeI3) { conditionalTest(prog::Instruction::jLeI({}, {}, 3), 420, 69, false); }
TEST(Vm, ConditionalLeI4) { conditionalTest(prog::Instruction::jLeI({}, {}, 3), -1, 0, true); }

TEST(Vm, ConditionalGeI1) { conditionalTest(prog::Instruction::jGeI({}, {}, 3), 69, 69, true); }
TEST(Vm, ConditionalGeI2) { conditionalTest(prog::Instruction::jGeI({}, {}, 3), 69, 420, false); }
TEST(Vm, ConditionalGeI3) { conditionalTest(prog::Instruction::jGeI({}, {}, 3), 420, 69, true); }
TEST(Vm, ConditionalGeI4) { conditionalTest(prog::Instruction::jGeI({}, {}, 3), 0, -1, true); }

TEST(Vm, ConditionalLtU1) { conditionalTest(prog::Instruction::jLtU({}, {}, 3), 69, 69, false); }
TEST(Vm, ConditionalLtU2) { conditionalTest(prog::Instruction::jLtU({}, {}, 3), 69, 420, true); }
TEST(Vm, ConditionalLtU3) { conditionalTest(prog::Instruction::jLtU({}, {}, 3), 420, 69, false); }
TEST(Vm, ConditionalLtU4) { conditionalTest(prog::Instruction::jLtU({}, {}, 3), -1, 0, false); }

TEST(Vm, ConditionalGtU1) { conditionalTest(prog::Instruction::jGtU({}, {}, 3), 69, 69, false); }
TEST(Vm, ConditionalGtU2) { conditionalTest(prog::Instruction::jGtU({}, {}, 3), 69, 420, false); }
TEST(Vm, ConditionalGtU3) { conditionalTest(prog::Instruction::jGtU({}, {}, 3), 420, 69, true); }
TEST(Vm, ConditionalGtU4) { conditionalTest(prog::Instruction::jGtU({}, {}, 3), 0, -1, false); }

TEST(Vm, ConditionalLeU1) { conditionalTest(prog::Instruction::jLeU({}, {}, 3), 69, 69, true); }
TEST(Vm, ConditionalLeU2) { conditionalTest(prog::Instruction::jLeU({}, {}, 3), 69, 420, true); }
TEST(Vm, ConditionalLeU3) { conditionalTest(prog::Instruction::jLeU({}, {}, 3), 420, 69, false); }
TEST(Vm, ConditionalLeU4) { conditionalTest(prog::Instruction::jLeU({}, {}, 3), -1, 0, false); }

TEST(Vm, ConditionalGeU1) { conditionalTest(prog::Instruction::jGeU({}, {}, 3), 69, 69, true); }
TEST(Vm, ConditionalGeU2) { conditionalTest(prog::Instruction::jGeU({}, {}, 3), 69, 420, false); }
TEST(Vm, ConditionalGeU3) { conditionalTest(prog::Instruction::jGeU({}, {}, 3), 420, 69, true); }
TEST(Vm, ConditionalGeU4) { conditionalTest(prog::Instruction::jGeU({}, {}, 3), 0, -1, false); }

TEST(Vm, ConditionalLtF1) { conditionalTest(prog::Instruction::jLtF({}, {}, 3), 69.0f, 69.0f, false); }
TEST(Vm, ConditionalLtF2) { conditionalTest(prog::Instruction::jLtF({}, {}, 3), 69.0f, 420.0f, true); }
TEST(Vm, ConditionalLtF3) { conditionalTest(prog::Instruction::jLtF({}, {}, 3), 420.0f, 69.0f, false); }
TEST(Vm, ConditionalLtF4) { conditionalTest(prog::Instruction::jLtF({}, {}, 3), -1.0f, 0.0f, true); }

TEST(Vm, ConditionalGtF1) { conditionalTest(prog::Instruction::jGtF({}, {}, 3), 69.0f, 69.0f, false); }
TEST(Vm, ConditionalGtF2) { conditionalTest(prog::Instruction::jGtF({}, {}, 3), 69.0f, 420.0f, false); }
TEST(Vm, ConditionalGtF3) { conditionalTest(prog::Instruction::jGtF({}, {}, 3), 420.0f, 69.0f, true); }
TEST(Vm, ConditionalGtF4) { conditionalTest(prog::Instruction::jGtF({}, {}, 3), 0.0f, -1.0f, true); }

TEST(Vm, ConditionalLeF1) { conditionalTest(prog::Instruction::jLeF({}, {}, 3), 69.0f, 69.0f, true); }
TEST(Vm, ConditionalLeF2) { conditionalTest(prog::Instruction::jLeF({}, {}, 3), 69.0f, 420.0f, true); }
TEST(Vm, ConditionalLeF3) { conditionalTest(prog::Instruction::jLeF({}, {}, 3), 420.0f, 69.0f, false); }
TEST(Vm, ConditionalLeF4) { conditionalTest(prog::Instruction::jLeF({}, {}, 3), -1.0f, 0.0f, true); }

TEST(Vm, ConditionalGeF1) { conditionalTest(prog::Instruction::jGeF({}, {}, 3), 69.0f, 69.0f, true); }
TEST(Vm, ConditionalGeF2) { conditionalTest(prog::Instruction::jGeF({}, {}, 3), 69.0f, 420.0f, false); }
TEST(Vm, ConditionalGeF3) { conditionalTest(prog::Instruction::jGeF({}, {}, 3), 420.0f, 69.0f, true); }
TEST(Vm, ConditionalGeF4) { conditionalTest(prog::Instruction::jGeF({}, {}, 3), 0.0f, -1.0f, true); }

TEST(Vm, LoopFactorial)
{
	prog::Program p = {
		.types = {prog::TypeInfo::empty},
		.functions =
		{
			prog::Function
			{
				.nRefs = 0,
				.nScalars = 3,
				.code = {
					/* 0 */ prog::Instruction::lit({}, 1), // arg: 0, ret: 1

					/* 1 */ prog::Instruction::lit({}, 0),
					/* 2 */ prog::Instruction::jNe(0, {}, 4),

					/* 3 */prog::Instruction::ret(0, 1),

					/* 4 */ prog::Instruction::mulI(1, 1, 0),

					/* 5 */ prog::Instruction::lit({}, 1),
					/* 6 */ prog::Instruction::subI(0, 0, {}),

					/* 7 */ prog::Instruction::jump(1),
				}
			}
		}
	};

	for(const auto& v: factorialTestVectors)
	{
		CHECK(v.second == vm::Vm(storage, p).run({}, {v.first}).second.front().integer);
	}
}

TEST(Vm, RecursiveFactorial)
{
	prog::Program p = {
		.types = {prog::TypeInfo::empty},
		.functions =
		{
			prog::Function
			{
				.nRefs = 0,
				.nScalars = 3,
				.code = {
					/* 0 */ prog::Instruction::lit({}, 0),
					/* 1 */ prog::Instruction::jEq(0, {}, 8),

					/* 2 */ prog::Instruction::lit({}, 1),
					/* 3 */ prog::Instruction::subI({}, 0, {}),

					/* 4 */ prog::Instruction::lit({}, 0),
					/* 5 */prog::Instruction::call(0, 1),

					/* 6 */ prog::Instruction::mulI({}, 0, {}),
					/* 7 */prog::Instruction::ret(0, 1),

					/* 8 */ prog::Instruction::lit({}, 1),
					/* 9 */prog::Instruction::ret(0, 1),
				}
			}
		}
	};

	for(const auto& v: factorialTestVectors)
	{
		CHECK(v.second == vm::Vm(storage, p).run({}, {v.first}).second.front().integer);
	}
}

TEST(Vm, MakeList)
{
	prog::Program p = {
		.types = {
			prog::TypeInfo::empty,
			listElement
		},
		.functions =
		{
			prog::Function
			{
				.nRefs = 3,
				.nScalars = 2,
				.code = {
					/*  0 */ prog::Instruction::lit({}, 0),     // s0 -> i
					/*  1 */ prog::Instruction::make({}, 0),    // r0 -> head
					/*  2 */ prog::Instruction::movr({}, 0),    // r1 -> tail

					/*  3 */ prog::Instruction::lit({}, 10),	// if(i == 10) return head;
					/*  4 */ prog::Instruction::jNe(0, {}, 7),

					/*  5 */ prog::Instruction::drop(1, 0),

					/*  6 */ prog::Instruction::ret(1, 0),

					/*  7 */ prog::Instruction::make({}, 1),    // r2 -> next
					/*  8 */ prog::Instruction::puts(0, 2, 0),  // next->s0 = i;

					/*  9 */ prog::Instruction::jNul(1, 12),    // if(tail != vm::null) {

					/* 10 */ prog::Instruction::putr(2, 1, 0),  // tail->r0 = next;

					/* 11 */ prog::Instruction::jump(13),       // } else {

					/* 12 */ prog::Instruction::movr(0, 2),     // head = next; }

					/* 13 */ prog::Instruction::movr(1, {}),     // tail = next;

					/* 14 */ prog::Instruction::lit({}, 1),    // i++
					/* 15 */ prog::Instruction::addI(0, 0, {}),

					/* 16 */ prog::Instruction::jump(3),       // while(true);
				}
			}
		}
	};

	auto h = vm::Vm(storage, p).run({}, {}).first.front();

	for(int i = 0; i < 10 ; i++)
	{
		CHECK(h != vm::null);
		CHECK(i == storage.reads(h, 0).integer);
		h = storage.readr(h, 0);
	}

	CHECK(h == vm::null);
}

TEST(Vm, SumList)
{
	prog::Program p = {
		.types = {
			prog::TypeInfo::empty,
			listElement
		},
		.functions =
		{
			prog::Function
			{
				.nRefs = 2,
				.nScalars = 2,
				.code = {
					/* 0 */ prog::Instruction::lit({}, 0),     // s0 -> ret
					/* 1 */ prog::Instruction::jNnl(0, 3),     // if(p == null /* arg:r0 */) {
					/* 2 */ prog::Instruction::ret(0, 1),      //   return ret;

					/* 3 */ prog::Instruction::gets({}, 0, 0), //   ret += p->s0;
					/* 4 */ prog::Instruction::addI(0, 0, {}),

					/* 5 */ prog::Instruction::getr(0, 0, 0), //    ret = p->r0;

					/* 6 */ prog::Instruction::jump(1),       // while(true);
				}
			}
		}
	};

	vm::Reference tail = vm::null, head = tail;
	for(int i = 0; i < 10 ; i++)
	{
		auto next = storage.create(listElement);
		storage.writes(next, 0, i);

		if(tail != vm::null)
		{
			storage.writer(tail, 0, next);
		}
		else
		{
			head = next;
		}

		tail = next;
	}

	CHECK(45 == vm::Vm(storage, p).run({head}, {}).second.front().integer);
}

TEST(Vm, StaticAccess)
{
	prog::Program p = {
		.types = {
			prog::TypeInfo(0, 1, 1),
			prog::TypeInfo(0, 0, 1)
		},
		.functions =
		{
			prog::Function
			{
				.nRefs = 1,
				.nScalars = 1,
				.code = {
					prog::Instruction::mov(prog::Instruction::Reg::global(0), {}),
					prog::Instruction::lit({}, 1),
					prog::Instruction::call(0, 0),
					prog::Instruction::movr({}, prog::Instruction::Reg::global(0)),
					prog::Instruction::ret(1, 0),
				}
			},
			prog::Function
			{
				.nRefs = 1,
				.nScalars = 1,
				.code = {
					prog::Instruction::make({}, 1),
					prog::Instruction::mov({}, prog::Instruction::Reg::global(0)),
					prog::Instruction::puts({}, 0, 0),
					prog::Instruction::movr(prog::Instruction::Reg::global(0), {}),
					prog::Instruction::ret(0, 0),
				}
			}
		}
	};

	auto r = vm::Vm(storage, p).run({}, {420}).first.front();

	CHECK(r != vm::null);
	CHECK(420 == storage.reads(r, 0).integer);
}
