#ifndef COMPILER_TAC_CFG_BASICBLOCK_H_
#define COMPILER_TAC_CFG_BASICBLOCK_H_

#include "compiler/ast/Values.h"
#include "compiler/ast/Statements.h"

#include <memory>

namespace comp {

struct BasicBlock
{
	std::shared_ptr<ast::Block> code = std::make_shared<ast::Block>();
	std::shared_ptr<const ast::RValue> decisionInput;
	std::shared_ptr<BasicBlock> then, otherwise;
};

//std::string comp::dumpCfg(const ProgramObjectSet& gi, std::shared_ptr<Function> fn)
//{
//	std::stringstream ss;
//	std::map<const Local*, size_t> locals;
//	writeFunctionHeader(gi, ss, fn);
//
//	const auto rawCfg = tacify(fn);
//	auto getIdx = [&](auto i) { return std::find(rawCfg.begin(), rawCfg.end(), i) - rawCfg.begin(); };
//
//	for(auto i = 0u; i < rawCfg.size(); i++)
//	{
//		const auto& bb = rawCfg[i];
//
//		ss << i << ":" << std::endl;
//		dumpStatementAst(gi, locals, ss, bb->code, 1);
//
//		if(bb->decisionInput)
//		{
//			ss << "if " << dumpExpressionAst(gi, locals, bb->decisionInput, OpPrecedence::Root) << " then goto " << getIdx(bb->then) << " else goto " << getIdx(bb->otherwise) << std::endl;
//		}
//		else if(bb->then)
//		{
//			ss << "goto " << getIdx(bb->then) << std::endl;
//		}
//		else
//		{
//			ss << "done" << std::endl;
//		}
//	}
//
//	const auto ret = ss.str();
//	return ret;
//}

}  // namespace comp

#endif /* COMPILER_TAC_CFG_BASICBLOCK_H_ */

