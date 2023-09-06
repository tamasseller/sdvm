#include "Compiler.h"

using namespace comp;

void Compiler::optimizeIr(std::shared_ptr<ir::Function> ir, Options opt)
{
	bool changed;
	while(changed)
	{
		changed = false;

		if((opt & Options::doJumpOptimizations) && (changed = mergeBasicBlocks(ir))) continue;
		if((opt & Options::doJumpOptimizations) && (changed = removeEmptyBasicBlocks(ir))) continue;
		if((opt & Options::propagateConstants) && (changed = propagateConstants(ir))) continue;
		if((opt & Options::eliminateDeadCode) && (changed = eliminateDeadCode(ir))) continue;
	}
}
