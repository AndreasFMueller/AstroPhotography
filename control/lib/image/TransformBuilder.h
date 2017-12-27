/*
 * TransformBuilder.h -- 
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _TransformBuilder_h
#define _TransformBuilder_h

#include <AstroTransform.h>

namespace astro {
namespace image {
namespace transform {

class TransformBuilder {
protected:
	void	showResiduals(const Transform& t,
			const std::vector<Point>& from,
			const std::vector<Point>& to) const;
public:
	TransformBuilder() { }
	virtual Transform	build(const std::vector<Point>& from,
			const std::vector<Point>& to,
			const std::vector<double>& weight) = 0;
};

class AffineTransformBuilder : public TransformBuilder {
public:
	AffineTransformBuilder() { }
	virtual Transform	build(const std::vector<Point>& from,
				const std::vector<Point>& to,
				const std::vector<double>& weight);
};

class RigidTransformBuilder : public TransformBuilder {
public:
	RigidTransformBuilder() { }
	virtual Transform	build(const std::vector<Point>& from,
				const std::vector<Point>& to,
				const std::vector<double>& weight);
};

} // namespace transform
} // namespace image
} // namespace astro

#endif /* _TransformBuilder_h */
