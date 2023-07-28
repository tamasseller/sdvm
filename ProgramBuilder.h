#ifndef PROGRAMBUILDER_H_
#define PROGRAMBUILDER_H_

#include "Vm.h"

#include <functional>

enum SourceTypeKind {
	Reference, Value
};

enum ValueType {
	Integer, Floating, Logical, Native
};

struct SourceType {
	SourceTypeKind kind;

	union {
		ValueType primitiveType;
		ObjectType* referenceType;
	};

	static inline auto integer() { return SourceType{ .kind = SourceTypeKind::Value, .primitiveType = ValueType::Integer}; }
};

struct RValue {
	SourceType type;
	std::function<void()> manifest;
};

class FunctionBuilder
{
	friend class ProgramBuilder;

	std::optional<SourceType> retType;
	std::vector<SourceType> args;
	size_t nextLocal;
	std::vector<size_t> frameRefIndices;
	std::vector<Instruction> code;

	int stackDepth = 0, maxStackDepth = 0;
	bool hasCall = false;

	struct FrameType: ObjectType
	{
		FrameType(size_t nextLocal, std::vector<size_t> frameRefIndices, size_t opStackDepth):
			ObjectType(&Program::Function::Frame::base, nextLocal + opStackDepth, std::move(frameRefIndices)) {}

		inline virtual std::vector<size_t> referenceOffsets(Storage* storage, Storage::Ref instance) const {
			auto ret = this->ObjectType::referenceOffsets(storage, instance);

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

	FunctionBuilder(const FunctionBuilder&) = delete;
	FunctionBuilder(FunctionBuilder&&) = delete;

	inline FunctionBuilder(std::optional<SourceType> ret, std::vector<SourceType> args):
			retType(ret), args(args), nextLocal(args.size())
	{
		for(auto i = 0u; i < args.size(); i++) {
			if(args[i].kind == SourceTypeKind::Reference)
			{
				frameRefIndices.push_back(i);
			}
		}
	}

	Program::Function operator()(std::vector<std::unique_ptr<ObjectType>> &holder)
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

public:
	auto arg(size_t n) {
		assert(n < args.size());
		return RValue {
			.type = args[n],
			.manifest = [this, n](){write(Instruction::readValueLocal(n));}
		};
	}

	void ret(const RValue& v)
	{
		v.manifest();
		write(v.type.kind == SourceTypeKind::Reference ? Instruction::retRef() : Instruction::retVal());
	}

	void ret() {
		write(Instruction::ret());
	}

	RValue addi(const RValue& a, const RValue& b)
	{
		assert(a.type.kind == SourceTypeKind::Value);
		assert(a.type.primitiveType == ValueType::Integer);
		assert(b.type.kind == SourceTypeKind::Value);
		assert(b.type.primitiveType == ValueType::Integer);

		return RValue {
			.type = SourceType::integer(),
			.manifest = [this, a, b](){
				b.manifest();
				a.manifest();
				write(Instruction::binary(Instruction::BinaryOpType::AddI));
			}
		};
	}
};

class ClassBuilder
{

};

class ProgramBuilder: public Program
{
	std::vector<std::unique_ptr<ObjectType>> types;
	static inline const ObjectType emptyType = { .base = nullptr, .length = 0, .refOffs = {} };

public:
	inline ProgramBuilder() {
		staticType = &emptyType;
	}

	template<class C>
	inline void fun(std::optional<SourceType> ret, std::vector<SourceType> args, C&& c) {
		auto fb = FunctionBuilder(ret, args);
		c(fb);
		functions.push_back(fb(types));
	}
};

#endif /* PROGRAMBUILDER_H_ */
