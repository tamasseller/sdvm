#include "TestUtils.h"

#include "jit/Immediate.h"

TEST_GROUP(Immediate)
{
	enum class Type
	{
		Mov,
		TwoStep
	};

	uint32_t executePlan(ImmediateFabricationPlan plan)
	{
		uint32_t r = plan.imm;

		switch(plan.op)
		{
		case ImmediateFabricationPlan::LastOp::Add:
			r += plan.param;
			break;

		case ImmediateFabricationPlan::LastOp::Shift:
			r <<= plan.param;
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

	void checkResult(uint32_t x, bool singleStep)
	{
		ImmediateFabricationPlan plan;
		CHECK(ImmediateFabricationPlan::make(x, plan));
		CHECK(executePlan(plan) == x);

		if(singleStep)
		{
			CHECK(plan.op == ImmediateFabricationPlan::LastOp::None);
		}
		else
		{
			CHECK(plan.op != ImmediateFabricationPlan::LastOp::None);
		}
	}
};

TEST(Immediate, Mov)
{
	for(uint32_t i = 0; i < 256; i++)
	{
		checkResult(i, true);
	}
}

TEST(Immediate, Not)
{
	for(uint32_t i = 1; i <= 256; i++)
	{
		checkResult(-i, false);
	}
}

TEST(Immediate, Add)
{
	for(uint32_t i = 256; i < 511; i++)
	{
		checkResult(i, false);
	}
}

TEST(Immediate, Shift)
{
	for(uint32_t i = 9; i < 32; i++)
	{
		checkResult(1 << i, false);
	}

	for(uint32_t i = 0; i < 23; i++)
	{
		checkResult(0b1000'0001'00 << i, false);
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

TEST(Immediate, Unsolvable)
{
	ImmediateFabricationPlan plan;
	CHECK(!ImmediateFabricationPlan::make(0x12345678, plan));

	for(uint32_t i = 1; i < 0x100; i++)
	{
		CHECK(!ImmediateFabricationPlan::make(0x80000000 | i, plan));
	}
}
