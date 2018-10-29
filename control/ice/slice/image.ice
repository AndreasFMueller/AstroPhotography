//
// image.ice -- Interface definition for images access
//
// (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
//
#include <types.ice>

/**
 * \brief snowstar module captures all interfaces
 */
module snowstar {
	struct Metavalue {
		string	keyword;
		string	value;
		string	comment;
	};
	sequence<Metavalue>	Metadata;

	enum ImageEncoding {
		ImageEncodingFITS, ImageEncodingJPEG, ImageEncodingPNG
	};

	sequence<byte>	ImageFile;

	struct ImageBuffer {
		ImageEncoding	encoding;
		ImageFile	data;
	};
	/**
 	 * \brief Image base interface
	 *
	 * This interface allows access to properties of an image that are
	 * independent of the value of an individual pixel. It is not intended
	 * to give access to all the information contained in a FITS file,
	 * if that is desired, the FITS file should be used directly.
	 */
	interface Image {
		/**
		 * \brief Get the filename of the image
 		 */
		string	name();

		/**
		 * \brief Age of the file
		 */
		int	age();

		/**
		 * \brief Size of the image
		 */
		ImageSize	size();

		/**
		 * \brief Origin of the image
		 *
		 * If this image was taken by selecting a subrectangle of a
		 * larger CCD chip, then this method returns the origin of
		 * the subrectangle. For full size images, this is always
		 * (0,0).
		 */
		ImagePoint	origin();

		/**
		 * \brief Number of bytes per pixel
		 *
		 * This returns the number of bytes needed to store an
		 * individual pixel. RGB color pixels need three primitive data
		 * element per pixel for the three color channels. YUYV images
		 * (as returned by some web cams) need two primitive elements
		 * per pixel. Any unsigned integral type or floating point
		 * type can be used as primitive pixel type, but cameras usuall
		 * return either bytes or unsigned shorts.
		 */
		int	bytesPerPixel();

		/**
		 * \brief Number of planes.
		 * 
		 * This is usually 1, but for cameras that return color images
		 * it can be 2 (YUYV pixels) or 3 (RGB pixels).
		 */
		int	planes();

		/**
		 * \brief Pixel value type size
		 */
		int	bytesPerValue();

		/**
		 * \brief Retrieve the imagedata
		 *
		 * This method returns the contents of the FITS file the server
		 * collected.
		 */
		ImageFile	file(ImageEncoding encoding) throws NotImplemented;

		/**
		 * \brief get the file size
		 */
		int	filesize();

		bool	hasMeta(string key);

		/**
		 * \brief Get a metadata value from the image
		 */
		Metavalue	getMeta(string key);

		/**
		 * \brief Update or add a metadata value in the image
		 */
		void	setMetavalue(Metavalue value);
		void	setMetadata(Metadata values);

		/**
		 * \brief Save an image in a repository for later use
		 *
		 * This method tells the server to save the image in
		 * the name repository
		 */
		void	toRepository(string reponame) throws NotFound;

		/**
		 * \brief remove the image from the respository
		 */
		void	remove();
	};

	/**
	 * \brief An image with byte sized pixels
	 */
	sequence<byte>	ByteSequence;
	interface ByteImage extends Image {
		ByteSequence	getBytes();
	};

	/**
	 * \brief Images consist of an array of short pixel values
	 */
	sequence<short> ShortSequence;

	/**
	 * \brief An image with short sized pixels
	 */
	interface ShortImage extends Image {
		ShortSequence	getShorts();
	};

	sequence<int>	IntSequence;
	interface IntImage extends Image {
		IntSequence	getInts();
	};

	sequence<float>	FloatSequence;
	interface FloatImage extends Image {
		FloatSequence	getFloats();
	};

	sequence<double>	DoubleSequence;
	interface DoubleImage extends Image {
		DoubleSequence	getDoubles();
	};

	/**
	 * \brief Image database interface
	 *
	 * The server can keep a set of images on disk, this service gives
	 * access to these services
	 */
	sequence<string>	ImageList;
	interface Images {
		ImageList	listImages();
		int	imageSize(string name) throws NotFound;
		int	imageAge(string name) throws NotFound;
		Image*	getImage(string name) throws NotFound;
		void	remove(string name) throws NotFound;
		string	save(ImageFile image);
	};

	/**
	 * \brief Small image for callbacks
	 */
	struct SimpleImage {
		ImageSize	size;
		ShortSequence	imagedata;
	};

	/**
	 * \brief Callback to send images in a sequence to clients
	 *
	 * The guider and the focusing process both send image updates to
	 * a monitoring client. They share the callback interface below
	 */
	interface ImageMonitor extends Callback {
		void	update(SimpleImage image);
	};
};
