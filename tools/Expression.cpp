#include "Expression.h"

Expression::BinaryOperator::BinaryOperator(Visa::BinaryOperation op, const RValue &l, const RValue &r):
	writer(
		[
			 op,
			 wa{(StatementStream::Statement)l},
			 wb{(StatementStream::Statement)r}
		]
		(StatementStream::InstructionSink& b)
		{
			wa(b);
			wb(b);
			Visa::Instruction isn = {.group = Visa::OperationGroup::Binary};
			isn.binOp = op;
			b.takeNonControl(isn);
		}
) {}

Expression::BinaryOperator::operator StatementStream::Statement() const {
	return writer;
}
