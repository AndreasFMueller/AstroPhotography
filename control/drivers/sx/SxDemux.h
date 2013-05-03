/*
 * SxDemux.h -- classes for demultiplexing
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#ifndef _SxDemux_h
#define _SxDemux_h

#include <AstroImage.h>

using namespace astro::image;

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief A field to be demultiplexed later
 */
class	Field {
public:
	size_t	length;
	unsigned short	*data;
	Field(size_t l);
	~Field();
	void	rescale(double scale);
};

/**
 * \brief The demultiplexer base class
 */
class Demuxer {
	// parameters for the demultiplexing operation
protected:
	int	offset;
	int	perm[4];
	int	greenx;
	int	greeny;
	int	redx;
	int	redy;
	int	bluex;
	int	bluey;
	// some variables we need during the demultiplexing
	int	width;
	int	height;
	void	set_pixel(Image<unsigned short>& image, int x, int y,
			unsigned short value) const;
	void	PIXEL(Image<unsigned short>& image, int x, int y,
			unsigned short v);
	void	LEXIP(Image<unsigned short>& image, int x, int y,
			unsigned short v);
public:
	Demuxer();
	virtual	~Demuxer();
	virtual	void	operator()(Image<unsigned short>& image,
				const Field& field1, const Field& field2);
};

class DemuxerBinned : public Demuxer {
	void	set_quad(Image<unsigned short>& image, int x, int y,
		const Field& field, int offset);
	void	set_quad_back(Image<unsigned short>& image, int x, int y,
		const Field& field, int offset);
public:
	DemuxerBinned();
	virtual ~DemuxerBinned();
	virtual	void	operator()(Image<unsigned short>& image,
				const Field& field1, const Field& field2);
};

class DemuxerUnbinned : public Demuxer {
	void	set_quad(Image<unsigned short>& image, int x, int y,
		const Field& field, int offset);
	void	set_quad_back(Image<unsigned short>& image, int x, int y,
		const Field& field, int offset);
public:
	DemuxerUnbinned();
	virtual ~DemuxerUnbinned();
	virtual	void	operator()(Image<unsigned short>& image,
				const Field& field1, const Field& field2);
};


} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxDemux_h */
