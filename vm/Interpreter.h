#ifndef VM_INTERPRETER_H_
#define VM_INTERPRETER_H_

#include "Program.h"

#include <memory>

class Interpreter
{
	Program program;

	class Frame
	{
		const Program& p;
		const Program::Function& f;
		std::unique_ptr<uint32_t[]> locals, stack;
		uint32_t* sp;

		inline uint32_t pop();
		inline uint32_t load(uint32_t);
		inline void push(uint32_t);
		inline void store(uint32_t, uint32_t);
		inline Program::Body::const_iterator processBlock(Program::Body::const_iterator);

	public:
		inline Frame(const Program& p, const Program::Function& f, const uint32_t* begin, const uint32_t* end);
		inline std::vector<uint32_t> run();
	};

public:
	Interpreter(const Program& program): program(program) {}
	std::vector<uint32_t> run(const std::vector<uint32_t> &args);
};

#endif /* VM_INTERPRETER_H_ */
