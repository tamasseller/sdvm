#ifndef CONCEPT_ANNOTATIONS_H_
#define CONCEPT_ANNOTATIONS_H_

#include "meta/Annotation.h"

#include "compiler/ir/BasicBlock.h"

#include <string>
#include <functional>

namespace comp {

struct IrComment: AnnotationBase<IrComment>
{
	const std::function<std::string(ir::BasicBlock::DumpContext&)> genText;

	inline IrComment(decltype(genText) genText): genText(genText) {}
};

} // namespace comp


#endif /* CONCEPT_ANNOTATIONS_H_ */
