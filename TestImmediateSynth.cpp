#include "TestUtils.h"

#include "jit/Immediate.h"

TEST_GROUP(Immediate)
{
	enum class Type
	{
		Mov,
		TwoStep,
		ThreeStep
	};

	uint32_t executePlan(ImmediateFabricationPlan plan)
	{
		uint32_t r = plan.imm;
		r <<= plan.shift;

		switch(plan.op)
		{
		case ImmediateFabricationPlan::LastOp::Add:
			r += plan.param;
			break;

		case ImmediateFabricationPlan::LastOp::Sub:
			r -= plan.param;
			break;

		case ImmediateFabricationPlan::LastOp::Not:
			r = ~r;
		}

		return r;
	}

	void checkResult(uint32_t x)
	{
		ImmediateFabricationPlan plan;
		CHECK(ImmediateFabricationPlan::make(x, plan));
		CHECK(executePlan(plan) == x);
	}

	void checkResult(uint32_t x, Type type)
	{
		ImmediateFabricationPlan plan;
		CHECK(ImmediateFabricationPlan::make(x, plan));
		CHECK(executePlan(plan) == x);

		switch(type)
		{
		case Type::Mov:
			CHECK(plan.shift == 0 && plan.op == ImmediateFabricationPlan::LastOp::None);
			break;
		case Type::TwoStep:
			CHECK((plan.shift != 0 && plan.op == ImmediateFabricationPlan::LastOp::None)
					|| (plan.shift == 0 && plan.op != ImmediateFabricationPlan::LastOp::None));
			break;
			CHECK(plan.shift == 0 && plan.op == ImmediateFabricationPlan::LastOp::Not);
			break;
		case Type::ThreeStep:
			CHECK(plan.shift != 0 && plan.op != ImmediateFabricationPlan::LastOp::None);
			break;
		}
	}
};

TEST(Immediate, Mov)
{
	for(uint32_t i = 0; i < 256; i++)
	{
		checkResult(i, Type::Mov);
	}
}

TEST(Immediate, Not)
{
	for(uint32_t i = 1; i <= 256; i++)
	{
		checkResult(-i, Type::TwoStep);
	}
}

TEST(Immediate, Add)
{
	for(uint32_t i = 256; i < 511; i++)
	{
		checkResult(i, Type::TwoStep);
	}
}

TEST(Immediate, Shift)
{
	for(uint32_t i = 9; i < 32; i++)
	{
		checkResult(1 << i, Type::TwoStep);
	}

	for(uint32_t i = 0; i < 23; i++)
	{
		checkResult(0b1000'0001'00 << i, Type::TwoStep);
	}
}

TEST(Immediate, ShiftAndAdd)
{
	for(uint32_t i = 0; i < 23; i++)
	{
		checkResult((0b1000'0001'00 << i) | 1, Type::ThreeStep);
	}
}

TEST(Immediate, ShiftAndSub)
{
	for(uint32_t i = 8; i < 15; i++)
	{
		checkResult((0b11 << i) - 1, Type::ThreeStep);
	}
}

TEST(Immediate, ValidateSingleByteShifted)
{
	for(uint32_t i = 0; i < 24; i++)
	{
		for(uint32_t j = 0; i < 256; i++)
		{
			checkResult(j << i);
		}
	}
}

TEST(Immediate, Validate64k)
{
	for(uint32_t i = 0; i < 0x10000; i++)
	{
		checkResult(i);
	}
}

TEST(Immediate, ValidateOneByteShiftedAndAnother)
{
	for(uint32_t i = 8; i < 24; i++)
	{
		for(uint32_t j = 0; i < 256; i++)
		{
			for(uint32_t k = 0; k < 256; k++)
			{
				checkResult(j << i + k);
				checkResult(j << i - k);
			}
		}
	}
}

TEST(Immediate, Unsolvable)
{
	ImmediateFabricationPlan plan;
	CHECK(!ImmediateFabricationPlan::make(0x12345678, plan));

	for(uint32_t i = 0; i < 0x10000; i++)
	{
		CHECK(!ImmediateFabricationPlan::make(0x8000 | i, plan));
	}
}
