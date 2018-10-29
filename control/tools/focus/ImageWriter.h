/*
 * ImageWriter.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

namespace astro {
namespace focusing {

class ImageWriter : public FocusElementCallback {
public:
	typedef enum format_e { FITS, JPEG, PNG } format_t;
private:
	std::string	_prefix;
	format_t	_format;
	std::string	filename(FocusElementCallbackData& fe,
				const std::string& which) const;
	void	write(ImagePtr image, const std::string& name) const;
public:
	ImageWriter(const std::string& prefix, format_t format = FITS)
		: _prefix(prefix), _format(format) { }
	virtual void	handle(FocusElementCallbackData& fe) const;
};

} // namespace focusing
} // namespace astro
