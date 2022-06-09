#include "Printer.h"

#include <sstream>

std::string Printer::print(const Program& p)
{
	std::stringstream ss;

	Visa::FrameInfo fi;

	for(int i = 0; i < p.functions.size(); i++)
	{
		const auto& f = p.functions[i];

		ss << "# fun#" << i << std::endl;
		ss << "#    stack:  " << f.info.maxStack << std::endl;
		ss << "#    locals: " << f.info.nLocals << std::endl;
		ss << "#    args:   " << f.info.nArgs << std::endl;
		ss << "#    ret:    " << f.info.nRet << std::endl;

		for(int j = 0; j < f.body.size(); j++)
		{
			ss << "l" << j << ":" << std::endl;

			const auto& b = f.body[j];
			for(int k = 0; k < b.size(); k++)
			{
				ss << "  ";
				const auto isn = b[k];

				switch(isn.group)
				{
				case Visa::OperationGroup::Binary:
					switch(isn.binOp)
					{
#define X(sym, op, mn) case Visa::BinaryOperation:: sym : ss << mn << std::endl; break;
X_BINARY_OPERATOR_LIST()
#undef X
					}

					break;

				case Visa::OperationGroup::Move:
					switch(isn.dest)
					{
					case Visa::MoveDetails::FromArgument:
						ss << "larg";
						break;

					case Visa::MoveDetails::FromLocal:
						ss << "lloc";
						break;

					case Visa::MoveDetails::ToArgument:
						ss << "sarg";
						break;

					case Visa::MoveDetails::ToLocal:
						ss << "sloc";
						break;
					}

					ss << " " << std::dec <<isn.varIdx << std::endl;
					break;

				case Visa::OperationGroup::Conditional:
					ss << "jcond " << std::dec << isn.targetBlockIdx << std::endl;
					break;

				case Visa::OperationGroup::Jump:
					ss << "jmp l" << std::dec << isn.targetBlockIdx << std::endl;
					break;

				case Visa::OperationGroup::Call:
					ss << "call " << std::endl;
					break;

				case Visa::OperationGroup::Return:
					ss << "ret " << std::endl;
					break;

				case Visa::OperationGroup::Immediate:
					ss << "imm 0x" << std::hex << isn.value << std::endl;
					break;

				case Visa::OperationGroup::Duplicate:
					ss << "dup" << std::endl;
					break;
				}
			}
		}

		ss << std::endl;
	}

	return ss.str();
}
