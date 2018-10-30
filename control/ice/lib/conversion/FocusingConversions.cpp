/*
 * FocusingConversions.cpp -- conversions between ice and astro
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceConversions.h>
#include <includes.h>
#include <AstroIO.h>
#include <AstroUtils.h>

namespace snowstar {

FocusState	convert(astro::focusing::Focus::state_type s) {
	switch (s) {
	case astro::focusing::Focus::IDLE:
		return FocusIDLE;
	case astro::focusing::Focus::MOVING:
		return FocusMOVING;
	case astro::focusing::Focus::MEASURING:
		return FocusMEASURING;
	case astro::focusing::Focus::MEASURED:
		return FocusMEASURED;
	case astro::focusing::Focus::FOCUSED:
		return FocusFOCUSED;
	case astro::focusing::Focus::FAILED:
		return FocusFAILED;
	}
	throw std::runtime_error("unknown focus state");
}

astro::focusing::Focus::state_type	convert(FocusState s) {
	switch (s) {
	case FocusIDLE:
		return astro::focusing::Focus::IDLE;
	case FocusMOVING:
		return astro::focusing::Focus::MOVING;
	case FocusMEASURING:
		return astro::focusing::Focus::MEASURING;
	case FocusMEASURED:
		return astro::focusing::Focus::MEASURED;
	case FocusFOCUSED:
		return astro::focusing::Focus::FOCUSED;
	case FocusFAILED:
		return astro::focusing::Focus::FAILED;
	}
	throw std::runtime_error("unknown focus state");
}

std::string	focusingstate2string(FocusState s) {
	return astro::focusing::Focus::state2string(convert(s));
}

FocusState	focusingstring2state(const std::string& s) {
	return convert(astro::focusing::Focus::string2state(s));
}

FocusPoint      convert(const astro::focusing::FocusItem& fi) {
	FocusPoint	result;
	result.position = fi.position();
	result.value = fi.value();
	return result;
}

astro::focusing::FocusItem      convert(const FocusPoint& fp) {
	astro::focusing::FocusItem	result(fp.position, fp.value);
	return result;
}

FocusElementPtr	convert(const astro::focusing::FocusElement& fe, 
                        astro::image::Format::type_t type) {
	FocusElement	*result = new FocusElement();
	result->position = fe.pos();
	result->value = fe.value;
	result->method = fe.method;

	// copy the raw image
	result->raw.encoding = convert(type);
	unsigned char	*b = NULL;
	size_t		bs = 0;
	Format	f;
	f.write(fe.raw_image, type, (void **)&b, &bs);
	std::copy(b, b + bs, std::back_inserter(result->raw.data));
	free(b);

	// copy the processed image
	result->evaluated.encoding = convert(type);
	b = NULL;
	bs = 0;
	f.write(fe.processed_image, type, (void **)&b, &bs);
	std::copy(b, b + bs, std::back_inserter(result->evaluated.data));
	free(b);

	return FocusElementPtr(result);
}

static int	conversion_counter = 0;

FocusElementPtr convert(const astro::focusing::FocusElementCallbackData& fe,
                        astro::image::Format::type_t type) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "convert %s", fe.toString().c_str());
	FocusElement	*result = new FocusElement();
	result->position = fe.position();
	result->value = fe.value();
	result->method = fe.method();

	// copy the raw image
	result->raw.encoding = convert(type);
	unsigned char	*b = NULL;
	size_t		bs = 0;
	Format	f;
	f.write(fe.raw_image(), type, (void **)&b, &bs);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: buffer=%p, size=%d",
		fe.raw_image()->info().c_str(),  b, bs);
	std::copy(b, b + bs, std::back_inserter(result->raw.data));
#if 0
	{
		int	fd = open(astro::stringprintf("conv-%d.png",
				conversion_counter++).c_str(),
				O_CREAT | O_TRUNC | O_WRONLY, 0666);
		if (fd < 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot open: %s",
				strerror(errno));
		}
		ssize_t	l = ::write(fd, b, bs);
		if (l < 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot write: %s",
				strerror(errno));
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d bytes written", l);
		close(fd);
	}
#endif
	free(b);

	// copy the processed image
	result->evaluated.encoding = convert(type);
	b = NULL;
	bs = 0;
	f.write(fe.processed_image(), type, (void **)&b, &bs);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: buffer=%p, size=%d",
		fe.processed_image()->info().c_str(),  b, bs);
	std::copy(b, b + bs, std::back_inserter(result->evaluated.data));
	free(b);

	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"raw.data.size()=%d, evaluated.data.size()=%d",
		result->raw.data.size(), result->evaluated.data.size());

	return FocusElementPtr(result);
}


astro::focusing::FocusElementPtr	convert(const FocusElement& fe) {
	astro::focusing::FocusElement	*result
		= new astro::focusing::FocusElement(fe.position);
	result->value = fe.value;
	result->method = fe.method;

	{
		int fd = open(astro::stringprintf("e-%d.png",
				conversion_counter++).c_str(),
				O_CREAT | O_TRUNC | O_WRONLY, 0666);;
		::write(fd, (void *)fe.raw.data.data(), fe.raw.data.size());
		close(fd);
	}

	// raw image
	Format	f;
	result->raw_image = f.read(convert(fe.raw.encoding),
		(void *)fe.raw.data.data(), fe.raw.data.size());

	// evaluated image
	result->processed_image = f.read(convert(fe.evaluated.encoding),
		(void *)fe.evaluated.data.data(), fe.evaluated.data.size());

	return astro::focusing::FocusElementPtr(result);
}

} // namespace snowstar
