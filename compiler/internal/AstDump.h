#ifndef COMPILER_ASTDUMP_H_
#define COMPILER_ASTDUMP_H_

#include "compiler/model/StatementTypes.h"

#include "GlobalIdentifiers.h"
#include "Overloaded.h"

#include <map>
#include <sstream>
#include <algorithm>

namespace comp {

std::string dumpCfg(const GlobalIdentifiers& gi, std::shared_ptr<Function> fn);
std::string dumpClassAst(const GlobalIdentifiers& gi, std::shared_ptr<Class> cl);
std::string dumpFunctionAst(const GlobalIdentifiers& gi, std::shared_ptr<Function> fn);

} // namespace comp

#endif /* COMPILER_ASTDUMP_H_ */
