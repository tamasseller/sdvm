#ifndef TOOLS_INMEMORYPROGRAMBUILDER_H_
#define TOOLS_INMEMORYPROGRAMBUILDER_H_

#include "Expression.h"

#include <memory>
#include <cassert>
#include <optional>

struct Builder
{
	using Label = std::shared_ptr<std::optional<uint32_t>>;

	struct Scope
	{
		const uint32_t offset;
		const std::shared_ptr<Scope> parent;

		std::vector<std::shared_ptr<Scope>> children;
		uint32_t nLocals = 0;

		inline uint32_t getMaxLocals() const;

		inline Scope(const std::shared_ptr<Scope>& parent);
		Scope();
	};

	class ProtoFunction
	{
		friend Builder;

		const uint32_t nRet;
		uint32_t nArgs = 0;

		StatementStream isns;
		std::shared_ptr<Scope> root = std::make_shared<Scope>(), current = root;

		ProtoFunction(uint32_t nRet);
		inline ProtoFunction(const ProtoFunction&) = delete;

		inline void enterScope();
		inline void leaveScope();

		inline uint32_t countLocals() const;
		void doRet(uint32_t);

	public:
		inline ProtoFunction(ProtoFunction&&) = default;

		inline Expression::Argument arg() {return {nArgs++};}
		inline Expression::Local local() {return {current->offset + current->nLocals++};}

		inline void set(const Expression::LValue& l, const Expression::RValue& r) {
			isns << r << l;
		}

		template<class... Args>
		inline void ret(const Args&... args)
		{
			const Expression::RValue* rArgs[] = {&args...};

			for(const auto& ra: rArgs)
			{
				isns << *ra;
			}

			doRet(sizeof...(args));
		}
	};

	inline ProtoFunction &function(uint32_t nRet) {
		functions.push_back({nRet});
		return functions.back();
	}

	Program make();
private:
	std::vector<ProtoFunction> functions;
};

#endif /* TOOLS_INMEMORYPROGRAMBUILDER_H_ */
