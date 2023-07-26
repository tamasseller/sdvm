#ifndef PROGRAMBUILDER_H_
#define PROGRAMBUILDER_H_

#include "Vm.h"

#include <functional>

template<class Type> struct IsRef { static constexpr bool value = false; };
template<> struct IsRef<Storage::Ref> { static constexpr bool value = true; };

template<class Type> struct Val {
	std::function<void()> manifest;
};

template<size_t n, class... Types> struct Nth;
template<class First, class... Rest> struct Nth<0, First, Rest...> { using Type = First; };
template<size_t n, class First, class... Rest> struct Nth<n, First, Rest...> { using Type = typename Nth<n - 1, Rest...>::Type; };

template<class Sgn> class FunctionBuilder;
template<class Ret, class... Args> class FunctionBuilder<Ret(Args...)>
{
	size_t nextLocal;
	std::vector<size_t> frameRefIndices;
	std::vector<Instruction> code;

	int stackDepth = 0, maxStackDepth = 0;
	bool hasCall = false;

	struct FrameType: Type
	{
		FrameType(size_t nextLocal, std::vector<size_t> frameRefIndices, size_t opStackDepth):
			Type(&Program::Function::Frame::base, nextLocal + opStackDepth, std::move(frameRefIndices)) {}

		inline virtual std::vector<size_t> referenceOffsets(Storage* storage, Storage::Ref instance) const {
			auto ret = this->Type::referenceOffsets(storage, instance);

			// TODO walk up the opstack

			return ret;
		}
	};

	void write(Instruction isn)
	{
		hasCall = hasCall || isn.op == Instruction::Operation::Call || isn.op == Instruction::Operation::CallV;
		stackDepth += isn.stackBalance();
		assert(0 <= stackDepth);
		maxStackDepth = std::max(maxStackDepth, stackDepth);
		code.push_back(isn);
	}

public:
	FunctionBuilder(const FunctionBuilder&) = delete;
	FunctionBuilder(FunctionBuilder&&) = delete;

	constexpr inline FunctionBuilder(): nextLocal(sizeof...(Args)) {
		constexpr bool isRefs[] = {IsRef<Args>::value...};
		for(auto i = 0u; i < sizeof...(Args); i++) {
			if(isRefs[i])
			{
				frameRefIndices.push_back(i);
			}
		}
	}

	Program::Function operator()(std::vector<std::unique_ptr<Type>> &holder)
	{
		auto h = std::make_unique<FrameType>(nextLocal, std::move(frameRefIndices), maxStackDepth);
		auto t = h.get();
		holder.push_back(std::move(h));

		Program::Function ret =
		{
			.frame = Program::Function::Frame {
				.opStackOffset = Program::Function::Frame::offsetToLocals + nextLocal + (hasCall ? Program::Function::Frame::callerStackExtra : 0),
				.frameType = t
			},
			.code = std::move(code)
		};

		return ret;
	}

	template<size_t n> auto argVal() {
		return Val<typename Nth<n, Args...>::Type> {
			[this](){write(Instruction::readValueLocal(n));}
		};
	}

	template<size_t n> auto argRef() {
		return Val<typename Nth<n, Args...>::Type> {
			[this](){write(Instruction::readRefenceLocal(n));}
		};
	}

	void ret(const Val<Ret>& v)
	{
		v.manifest();
		write(IsRef<Ret>::value ? Instruction::retRef() : Instruction::retVal());
	}

	void ret() {
		write(Instruction::ret());
	}

	Val<int> add(const Val<int>& a, const Val<int>& b) {
		return Val<int> {
			[this, a, b](){
				b.manifest();
				a.manifest();
				write(Instruction::binary(Instruction::BinaryOpType::AddI));
			}
		};
	}
};

class ProgramBuilder: public Program
{
	std::vector<std::unique_ptr<Type>> types;
	static inline const Type emptyType = { .base = nullptr, .length = 0, .refOffs = {} };

public:
	inline ProgramBuilder() {
		staticType = &emptyType;
	}

	template<class Sgn, class C>
	inline void fun(C&& c) {
		auto fb = FunctionBuilder<Sgn>{};
		c(fb);
		functions.push_back(fb(types));
	}
};

#endif /* PROGRAMBUILDER_H_ */
