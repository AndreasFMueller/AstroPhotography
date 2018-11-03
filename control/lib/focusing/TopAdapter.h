/*
 * TopAdapter.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _TopAdapter_h
#define _TopAdapter_h

#include <AstroImage.h>
#include <AstroFocus.h>

namespace astro {
namespace focusing {

class TopAdapter : public image::ConstImageAdapter<float> {
	FocusableImage	_imageptr;
	const image::ConstImageAdapter<float>& _image;
	float	_top;
public:
	float	top() const { return _top; }
	TopAdapter(FocusableImage imageptr, float top);
	virtual float	pixel(int x, int y) const;
};

} // namespace focusing
} // namespace astro

#endif /* _TopAdapter_h */
