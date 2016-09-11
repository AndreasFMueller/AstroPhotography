/*
 * RepositoryConversions.cpp -- conversions between ice and astro
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceConversions.h>
#include <includes.h>
#include <AstroIO.h>
#include <AstroUtils.h>

namespace snowstar {

ImageInfo	convert(const astro::project::ImageEnvelope& envelope) {
	ImageInfo	result;
	result.id = envelope.id();
	result.uuid = envelope.uuid();
	result.filename = envelope.filename();
	result.project = envelope.project();
	result.createdago = converttime(envelope.created());
	result.instrument = envelope.camera();
	result.size = convert(envelope.size());
	result.binning = convert(envelope.binning());
	result.exposuretime = envelope.exposuretime();
	result.temperature = envelope.temperature();
	result.purpose = astro::camera::Exposure::purpose2string(envelope.purpose());
	result.bayer = envelope.bayer();
	result.filter = envelope.filter();
	result.observationago = converttime(envelope.observation());
	return result;
}

astro::project::ImageEnvelope	convert(const ImageInfo& info) {
	astro::project::ImageEnvelope	envelope(info.id);
	envelope.uuid(info.uuid);
	envelope.filename(info.filename);
	envelope.project(info.project);
	envelope.created(converttime(info.createdago));
	envelope.camera(info.instrument);
	envelope.size(convert(info.size));
	envelope.binning(convert(info.binning));
	envelope.exposuretime(info.exposuretime);
	envelope.temperature(info.temperature);
	envelope.purpose(astro::camera::Exposure::string2purpose(info.purpose));
	envelope.observation(converttime(info.observationago));
	envelope.bayer(info.bayer);
	envelope.filter(info.filter);
	return envelope;
}

} // namespace snowstar
