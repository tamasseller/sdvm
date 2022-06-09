#ifndef TOOLS_EXCEPTIONALERRORHANDLER_H_
#define TOOLS_EXCEPTIONALERRORHANDLER_H_

#include "Interpreter.h"
#include <stdexcept>

struct ExceptionalErrorHandler
{
	struct Exception: std::exception
	{
		const std::string str;

		inline Exception(const std::string &str): str(str) {}

		inline virtual const char* what() const noexcept override {
	    	return str.c_str();
	    }
	};

	template<class... Args>
	[[noreturn]] static inline void take(InterpreterError error, Args... args)
	{
		char buff[512];

		switch(error)
		{
#define X(sym, desc) case InterpreterError:: sym : sprintf(buff, desc, args...); break;
	X_INTERPRETER_ERRORS()
#undef X
		default: sprintf(buff, "Unknown error");
		}

		throw Exception(buff);
	}
};

#endif /* TOOLS_EXCEPTIONALERRORHANDLER_H_ */
