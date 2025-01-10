/*
 * abadaper.h
 *
 * (c) 2024 Prof Dr Andreas Müller
 */
#if 0
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroAdapter.h>
#include <deque>
#include <vector>

namespace astro {
namespace adapter {

/**
 * \brief A template to crate an aberration inspector
 *
 * This is essentially a WindowsAdapter that uses a special grid layout
 * of the subwindows.
 */
template<typename Pixel>
class AberrationInspectorFactory {
	ImageSize	_targetsize;
	int	_hwindows;
public:
	int	hwindows() const { return _hwindows; }
	void	hwindows(int h) { _hwindows = h; }
private:
	int	_vwindows;
public:
	int	vwindows() const { return _vwindows; }
	void	vwindows(int v) { _vwindows = v; }
private:
	int	_gap;
public:
	int	gap() const { return _gap; }
	void	gap(int g) { _gap = g; }

	AberrationInspectorFactory(const ImageSize& targetsize);
	WindowsAdapter<Pixel>	*operator()(
		const ConstImageAdapter<Pixel>& source, bool _mosaic) const;
};

/**
 * \brief Constructor for the AberrationInspectorFactory
 *
 * \param targetsize	the size of the inspector to be created
 */
template<typename Pixel>
AberrationInspectorFactory<Pixel>::AberrationInspectorFactory(
	const ImageSize& targetsize) : _targetsize(targetsize) {
	_hwindows = 3;
	_vwindows = 3;
	_gap = 2;
}

/**
 * \brief Operator to extract a new WindowsAdapter
 *
 * This method creates a WindowsAdapter with a layout constructed from
 * a AberrationInspectorLayout. The caller has to delete this adapter.
 *
 * \param source	the source image adapter to take the image data from
 * \param _mosaic	whether the image is a Bayer mosaic
 */
template<typename Pixel>
WindowsAdapter<Pixel>	*AberrationInspectorFactory<Pixel>::operator()(
	const ConstImageAdapter<Pixel>& source,
	bool _mosaic) const {
	AberrationInspectorLayout	abl(_targetsize,
		source.getSize(), _mosaic);
	abl.layout(_hwindows, _vwindows, _gap);
	WindowsAdapter<Pixel>	*wa = new WindowsAdapter<Pixel>(source,
		_targetsize);
	for (size_t i = 0; i < abl.nwindows(); i++) {
		auto	p = abl.window(i);
		wa->add(p.second, p.first);
	}
	return wa;
}

} // namespace adapter
} // namespace astro
#endif
