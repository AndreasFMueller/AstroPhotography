//
// types.ice -- common type definitions
//
// (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
//

/**
 * \brief snowstar module captures all interfaces
 */
module snowstar {
	// Image related data structures
	/**
	 * \brief Pixel coordinates of a pixel in an image
	 *
	 * To be consistent with FITS, the origin of the coordinate system
	 * is in the lower left corner of the image.
 	 */
	struct ImagePoint {
		int	x;
		int	y;
	};

	/**
	 * \brief Size of an image in pixels
	 */
	struct ImageSize {
		int	width;
		int	height;
	};

	/**
	 * \brief Rectangle inside an image
	 */
	struct ImageRectangle {
		ImagePoint	origin;
		ImageSize	size;
	};

	/**
	 * \brief generic point
	 *
	 * This type is used for guiding, where subpixel resolution is
	 * required, which leads to using floating coordinates
	 */
	struct Point {
		float	x;
		float	y;
	};

	/**
	 * \brief sky point specification for equatorial mounts
	 */
	struct RaDec {
		float	ra;	// right ascension in hours
		float	dec;	// declination in degrees
	};

	/**
	 * \brief sky point specification for AltAzimut mounts
	 */
	struct AzmAlt {
		float	azm;	// azimut in degrees
		float	alt;	// altitude above horizon in degrees
	};

	/**
	 * \brief base class for all callbacks interfaces
	 *
	 * Callbacks monitor some process, so they all need notifcation when
	 * that process has completed. They may want to extend that facility
	 * e.g. to report the final state.
	 */
	interface Callback {
		void	stop();
	};
};
