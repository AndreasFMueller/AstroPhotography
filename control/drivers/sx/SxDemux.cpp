/*
 * SxDemux.cpp -- demultiplexing stuff
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperwil
 */
#include <SxDemux.h>

namespace astro {
namespace camera {
namespace sx {

//////////////////////////////////////////////////////////////////////
// Field implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Create a field object.
 */
Field::Field(size_t l) : length(l) {
	data = new unsigned short[length];
}

/**
 * \brief destroy the field object.
 */
Field::~Field() {
	delete [] data;
	data = NULL;
}

/**
 * \brief Rescale a field
 *
 * This method scales the pixels of the field with the factor. The factor
 * must be >1 because otherwise saturated pixels become unsaturated by
 * the scaling operation, leading to wrong colors.
 * \param field		field object to rescale
 * \param scale
 */
void	Field::rescale(double scale) {
	for (size_t i = 0; i < length; i++) {
		unsigned long	rescaled = data[i] * scale;
		if (rescaled > 0xffff) {
			data[i] = 0xffff;
		} else {
			data[i] = rescaled;
		}
	}
}


//////////////////////////////////////////////////////////////////////
// Demuxer base class implementation
//////////////////////////////////////////////////////////////////////

Demuxer::Demuxer() {
}

Demuxer::~Demuxer() {
}

void	Demuxer::set_pixel(Image<unsigned short>& image, int x, int y,
		unsigned short value) const {
	if (x < 0) {
		return;
	}
	if (x >= width) {
		return;
	}
	if (y < 0) {
		return;
	}
	if (y >= height) {
		return;
	}
	image.pixel(x, y) = value;
}

void	Demuxer::operator()(Image<unsigned short>& image,
			const Field& field1, const Field& field2) {
	width = image.size.width;
	height = image.size.height;
}

void	Demuxer::PIXEL(Image<unsigned short>& image, int x, int y,
		unsigned short v) {
	set_pixel(image, x, y, v);
}

void	Demuxer::LEXIP(Image<unsigned short>& image, int x, int y,
		unsigned short v) {
	set_pixel(image, width - x, y, v);
}

//////////////////////////////////////////////////////////////////////
// DemuxerBinned base class implementation
//////////////////////////////////////////////////////////////////////
DemuxerBinned::DemuxerBinned() {
	offset = 0;
	perm[0] = 1;
	perm[1] = 3;
	perm[2] = 0;
	perm[3] = 2;
	greenx = 0; greeny = 0;
	redx = 0; redy = 0;
	bluex = 0; bluey = 0;
}

DemuxerBinned::~DemuxerBinned() {
}

void	DemuxerBinned::set_quad(Image<unsigned short>& image, int x, int y,
		const Field& field, int off) {
	int     p;
	p = perm[0] << 1; PIXEL(image, x + 0, y + 0, field.data[off + p]);
	p = perm[1] << 1; PIXEL(image, x + 0, y + 2, field.data[off + p]);
	p = perm[2] << 1; PIXEL(image, x + 2, y + 0, field.data[off + p]);
	p = perm[3] << 1; PIXEL(image, x + 2, y + 2, field.data[off + p]);
}

void	DemuxerBinned::set_quad_back(Image<unsigned short>& image,
		int x, int y, const Field& field, int off) {
	int     p;
	p = perm[2] << 1; LEXIP(image, x + 0, y + 0, field.data[off + p]);
	p = perm[3] << 1; LEXIP(image, x + 0, y + 2, field.data[off + p]);
	p = perm[1] << 1; LEXIP(image, x + 2, y + 2, field.data[off + p]);
	p = perm[0] << 1; LEXIP(image, x + 2, y + 0, field.data[off + p]);
}

void	DemuxerBinned::operator()(Image<unsigned short>& image,
		const Field& field1, const Field& field2) {
	Demuxer::operator()(image, field1, field2);

	int	off = 2 * offset + 1;
	for (int x = 0; x < width; x += 4) {
		for (int y = 0; y < height; y += 4) {
			set_quad_back(image,
				x + 1 + bluex - 2, y + 1 + bluey,
				field1, off);
		}
	}

	off = 2 * offset;
	for (int x = 0; x < width; x += 4) {
		for (int y = 0; y < height; y += 4) {
			set_quad(image,
				x + 1, y + 0,
				field1, off);
		}
	}

	off = 2 * offset + 1;
	for (int x = 0; x < width; x += 4) {
		for (int y = 0; y < height; y += 4) {
			set_quad_back(image,
				x + redx - 2, y + redy,
				field2, off);
		}
	}

	off = 2 * offset;
	for (int x = 0; x < width; x += 4) {
		for (int y = 0; y < height; y += 4) {
			set_quad(image,
				x + 0 + greenx + 2,
				y + 1 + greeny,
				field2, off);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// DemuxerUnbinned base class implementation
//////////////////////////////////////////////////////////////////////

DemuxerUnbinned::DemuxerUnbinned() {
	offset = 0;
	perm[0] = 0;
	perm[1] = 1;
	perm[2] = 2;
	perm[3] = 3;
	greenx = 0; greeny = 0;
	redx = 0; redy = 0;
	bluex = 0; bluey = 0;
}

DemuxerUnbinned::~DemuxerUnbinned() {
}

#define GREENSHIFTX     1
#define GREENSHIFTY     -1
#define BLUESHIFTX      0
#define BLUESHIFTY      0
#define GBSHIFTX        0
#define GBSHIFTY        0
#define REDSHIFTX       0
#define REDSHIFTY       0
#define GRSHIFTX        0
#define GRSHIFTY        0

void	DemuxerUnbinned::set_quad(Image<unsigned short>& image, int x, int y,
		const Field& field, int off) {
	int     p;
	p = perm[0] << 1; PIXEL(image, x + 0, y + 0, field.data[off + p]);
	p = perm[1] << 1; PIXEL(image, x + 2, y + 0, field.data[off + p]);
	p = perm[2] << 1; PIXEL(image, x + 0, y + 2, field.data[off + p]);
	p = perm[3] << 1; PIXEL(image, x + 2, y + 2, field.data[off + p]);
}

void	DemuxerUnbinned::set_quad_back(Image<unsigned short>& image,
		int x, int y, const Field& field, int off) {
	int     p;
	p = perm[0] << 1; LEXIP(image, x + 0, y + 0, field.data[off + p]);
	p = perm[1] << 1; LEXIP(image, x + 2, y + 0, field.data[off + p]);
	p = perm[2] << 1; LEXIP(image, x + 0, y + 2, field.data[off + p]);
	p = perm[3] << 1; LEXIP(image, x + 2, y + 2, field.data[off + p]);
}

void	DemuxerUnbinned::operator()(Image<unsigned short>& image,
		const Field& field1, const Field& field2) {
	Demuxer::operator()(image, field1, field2);

	int	off = 2 * offset + 1;
	for (int x = 0; x < width; x += 4) {
		for (int y = 0; y < height; y += 4) {
			set_quad_back(image,
				x + 1 + BLUESHIFTX, y + 1 + BLUESHIFTY,
				field1, off);
		}
	}

	off = 2 * offset + 2;
	for (int x = 0; x < width; x += 4) {
		for (int y = 0; y < height; y += 4) {
			set_quad(image,
				x + 1 + GBSHIFTX + GREENSHIFTX,
				y + 0 + GBSHIFTY + GREENSHIFTY,
				field1, off);
		}
	}

	off = 2 * offset + 1;
	for (int x = 0; x < width; x += 4) {
		for (int y = 0; y < height; y += 4) {
			set_quad_back(image,
				x + REDSHIFTX, y + REDSHIFTY,
				field2, off);
		}
	}

	off = 2 * offset + 2;
	for (int x = 0; x < width; x += 4) {
		for (int y = 0; y < height; y += 4) {
			set_quad(image,
				x + 0 + GRSHIFTX + GREENSHIFTX,
				y + 1 + GRSHIFTY + GREENSHIFTY,
				field2, off);
		}
	}
}

} // namespace sx
} // namespace camera
} // namespace astro

