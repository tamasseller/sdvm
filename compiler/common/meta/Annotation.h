#ifndef CONCEPT_META_ANNOTATION_H_
#define CONCEPT_META_ANNOTATION_H_

#include <utility>

namespace comp {

#define _ANNOTATION_TYPES() \
	X(IrComment) \

#define X(n) class n;
_ANNOTATION_TYPES()
#undef X

struct AnnotationVisitor
{
#define X(n) virtual void visit(const n&) const = 0;
_ANNOTATION_TYPES()
#undef X

	inline virtual ~AnnotationVisitor() = default;
};

template<class C>
struct LambdaAnnotationVisitor: AnnotationVisitor
{
	C c;

	LambdaAnnotationVisitor(C&& c): c(std::forward<C>(c)) {}
	inline virtual ~LambdaAnnotationVisitor() = default;

#define X(n) virtual void visit(const n &v) const override final { c(v); };
_ANNOTATION_TYPES()
#undef X
};

struct Annotation
{
	virtual void accept(const AnnotationVisitor& v) const = 0;

	template<class C>
	inline void accept(C&& c) const {
		const LambdaAnnotationVisitor<C> v(std::forward<C>(c));
		this->accept((const AnnotationVisitor&)v);
	}

	inline virtual ~Annotation() = default;
};

template<class Child>
struct AnnotationBase: Annotation
{
	inline virtual void accept(const AnnotationVisitor& v) const override final {
		v.visit(*static_cast<const Child*>(this));
	}

	inline virtual ~AnnotationBase() = default;
};

} // namespace comp

#endif /* CONCEPT_META_ANNOTATION_H_ */
