#include "Builder.h"

Builder::Scope::Scope(): offset(0), parent(nullptr) {}

inline Builder::Scope::Scope(const std::shared_ptr<Scope>& parent): offset(parent->offset + parent->nLocals), parent(parent) {}

inline uint32_t Builder::Scope::getMaxLocals() const
{
	uint32_t max = 0;

	for(const auto& ch: children)
	{
		max = std::max(max, ch->getMaxLocals());
	}

	return max + nLocals;
}

Builder::ProtoFunction::ProtoFunction(uint32_t nRet): nRet(nRet), nArgs(0), root(std::make_shared<Scope>()), current(root) {}

inline void Builder::ProtoFunction::enterScope()
{
	auto n = std::make_shared<Scope>(current);
	current->children.push_back(n);
	current = n;
}

inline void Builder::ProtoFunction::leaveScope()
{
	current = current->parent;
	assert(current);
}

inline uint32_t Builder::ProtoFunction::countLocals() const {
	return root->getMaxLocals();
}

void Builder::ProtoFunction::doRet(uint32_t n)
{
	assert(n == nRet);

	isns << [nRet{nRet}](StatementStream::InstructionSink& b){
		b.takeRet(nRet);
	};
}

Program Builder::make()
{
	std::vector<Program::Function> fs;

	for(const auto& f: functions)
	{
		uint32_t maxStack;
		const auto b = f.isns.render(maxStack);
		fs.push_back({{maxStack, f.countLocals(), f.nArgs, f.nRet}, std::move(b)});
	}

	return {fs};
}

