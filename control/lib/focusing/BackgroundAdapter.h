/*
 * BackgroundAdapter.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _BackgroundAdapter_h
#define _BackgroundAdapter_h

#include <AstroFocus.h>

namespace astro {
namespace focusing {

/**
 * \brief A class designed to find an approximation for the background
 */
class BackgroundAdapter : public ConstImageAdapter<float> {
	const ConstImageAdapter<float>&	_image;
	float	_limit;
public:
	float	limit() const { return _limit; }
	void	limit(float l) { _limit = l; }
	BackgroundAdapter(const ConstImageAdapter<float>& image, float l);
	virtual float	pixel(int x, int y) const;
};

} // namespace focusing
} // namespace astro

#endif /* _BackgroundAdapter_h */
