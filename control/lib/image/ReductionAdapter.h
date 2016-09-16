/*
 * ReductionAdapter.h
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ReductionAdapter_h
#define _ReductionAdapter_h

namespace astro {
namespace image {
namespace transform {

class ReductionAdapter : public ConstImageAdapter<double> {
	const ConstImageAdapter<double>&	_image;
	double	_min;
	double	_max;
public:
	ReductionAdapter(const ConstImageAdapter<double>& image,
		double min, double max)
		: ConstImageAdapter<double>(image.getSize()), _image(image),
		  _min(min), _max(max) {
	}
	virtual double	pixel(int x, int y) const {
		double	v = _image.pixel(x, y);
		if (v < _min) { return 0.; }
		if (v > _max) { return _max - _min; }
		return v - _min;
	}
};

} // namespace transform
} // namespace image
} // namespace astro

#endif /* _ReductionAdapter_h */
