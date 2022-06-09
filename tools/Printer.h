#ifndef TOOLS_PRINTER_H_
#define TOOLS_PRINTER_H_

#include "Program.h"

#include <string>

struct Printer
{
	static std::string print(const Program& p);
};

#endif /* TOOLS_PRINTER_H_ */
