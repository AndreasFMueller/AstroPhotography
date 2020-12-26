/*
 * CelestronMount.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CelestronMount.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <includes.h>
#include <stdexcept>
#include <AstroDevice.h>
#include <sstream>

namespace astro {
namespace device {
namespace celestron {

const int	CelestronMount::query_interval = 600;

/**
 * \brief run method for the celestron mount
 *
 * \param mount		the mount to run for
 */
void	CelestronMount::launch(CelestronMount *mount) noexcept {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start a thread");
	try {
		mount->run();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "run method failed: %s",
			x.what());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread ends");
}

/**
 * \brief Get the serial name from the properties
 *
 * \param devicename	name of the device
 */
static std::string	getserialname(const std::string& devicename) {
	DeviceName	dev(devicename);
	// if the unit name is just a number, we should look up the
	// associated serial device in the properties
	try {
		int	unit = std::stoi(dev.unitname());
		Properties	properties(devicename);
		std::string	serialdevicename
			= properties.getProperty("device");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found serial device for "
			"unit %d name: %s", unit, serialdevicename.c_str());
		return serialdevicename;
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"could not find the serial name: %s", x.what());
	}
	// if we get to this point, then the serial device path must be
	// encoded in the unit name
	return URL::decode(dev.unitname());
}

/**
 * \brief Format a string as HEX
 *
 * \param x	the string to format
 */
std::string	hexstring(const std::string& x) {
	std::string	result;
	for (auto c = x.begin(); c != x.end(); c++) {
		int	C = *c;
		result.append(stringprintf("%02X", C));
	}
	return result;
}

/**
 * \brief Construct a celestron mount object
 *
 * \param devicename	name of the device
 */
CelestronMount::CelestronMount(const std::string& devicename)
	: Mount(devicename), Serial(getserialname(devicename)) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating Celestron mount on %s",
		serialdevice().c_str());

	// lock the serial line
	std::unique_lock<std::recursive_mutex>	lock(_mount_mutex);

	// check communication
	write("Kx");
	std::string	k = read(1);
	getprompt();
	if (k != std::string("X")) {
		std::runtime_error("no echo received");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mount has responded to echo request");

	// ask for version
	write("V");
	std::string	v = readto('#');
	v = v.substr(0, 2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "version = '%s' (%d bytes, %s)",
		v.c_str(), v.size(), hexstring(v).c_str());
	version = 0;
	if (v.size() >= 2) {
		try {
			version = 100 * v[0] + v[1];
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot convert: %s",
				x.what());
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "version: %d", version);

	// initialize the time variables
	_last_time_offset = 0;
	_last_time_queried = 0;
	_last_location_queried = 0;
	_last_location_source = Mount::LOCAL;

	// start the thread
	start_thread();
}

void	CelestronMount::start_thread() {
	std::unique_lock<std::recursive_mutex>	lock(_mount_mutex);
	if (_running) {
		debug(LOG_ERR, DEBUG_LOG, 0, "thread already running");
		return;
	}
	// launch the thread
	_running = true;
	_mount_thread  = std::thread(CelestronMount::launch, this);
}

void	CelestronMount::stop_thread() {
	// if the mount is in GOTO mode, signal the thread to terminate
	// (leaving the mount alone)
	{
		std::unique_lock<std::recursive_mutex>	lock(_mount_mutex);
		_running = false;
		_mount_condition.notify_all();
	}

	// wait for the mount to terminate
	if (_mount_thread.joinable()) {
		_mount_thread.join();
	}
}

void	CelestronMount::check_state() {
	// make sure we are in the right state
	if ((state() == Mount::GOTO) || (state() == Mount::IDLE)) {
		std::string	msg = stringprintf("bad state in %s: %s",
			name().toString().c_str(), 
			state2string(state()).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw MountBadState(msg);
	}
}

/**
 * \brief Destroy the mount object
 */
CelestronMount::~CelestronMount() {
	stop_thread();
}

/**
 * \brief Read the next prompt
 *
 * read a character and make sure it is a prompt character
 */
void	CelestronMount::getprompt() {
	std::string	s = read(1);
	if (s != "#") {
		throw std::runtime_error("prompt not received");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got # back");
}

/**
 * \brief Query the state of the mount
 */
Mount::state_type	CelestronMount::get_state() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locking for state command");
	std::unique_lock<std::recursive_mutex>	lock(_mount_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending J command to check alignment");
	write("J");
	std::string	s = readto('#');
	debug(LOG_DEBUG, DEBUG_LOG, 0, "response: %s (%d bytes, %s)",
		s.c_str(), s.size(), hexstring(s).c_str());
	state_type	result = IDLE;
	if (s[0] == 1) {
		result = TRACKING;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending L command");
	write("L");
	s = readto('#');
	debug(LOG_DEBUG, DEBUG_LOG, 0, "response: %s (%d bytes, %s)",
		s.c_str(), s.size(), hexstring(s).c_str());
	if (s == "1#") {
		result = GOTO;
	}
	return result;
}

/**
 * \brief Cancel a movement command
 */
void	CelestronMount::cancel() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locking for cancel command");
	std::unique_lock<std::recursive_mutex>	lock(_mount_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending cancel command");
	write("M");
	getprompt();
}

/**
 * \brief Go to a specific azm/alt setting
 *
 * \param azmalt	AzmAlt object for the position to move to
 */
void	CelestronMount::Goto(const AzmAlt& azmalt) {
	// make sure we are in the right state
	check_state();

	// lock before you launch a new thread
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locking for GOTO command");
	std::unique_lock<std::recursive_mutex>	lock(_mount_mutex);

	// send the GOTO command
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending GOTO AzmAtl command");
	std::string	cmd;
	if (version > 202) {
		cmd = stringprintf("b%08X,%08X",
			angle32(azmalt.azm()), angle32(azmalt.alt()));
	} else {
		cmd = stringprintf("B%04X,%04X",
			angle16(azmalt.azm()), angle16(azmalt.alt()));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command sent: %s", cmd.c_str());
	write(cmd);
	getprompt();

	// notify the thread that something has changed
	_mount_condition.notify_all();
}

/**
 * \brief Got to specific position in RA and DEC
 *
 * \param radec		sky position to move to
 */
void	CelestronMount::Goto(const RaDec& radec) {
	// make sure we are in the right state
	check_state();

	// lock before you launch a new thread
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locking for GOTO command");
	std::unique_lock<std::recursive_mutex>	lock(_mount_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending GOTO RaDec command");
	std::string	cmd;
	if (version > 106) {
		cmd = stringprintf("r%08X,%08X",
			angle32(radec.ra()), angle32(radec.dec()));
	} else {
		cmd = stringprintf("R%04X,%04X",
			angle16(radec.ra()), angle16(radec.dec()));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command sent: %s", cmd.c_str());
	write(cmd);
	getprompt();

	// notify the thread that something has changed
	_mount_condition.notify_all();
}

/**
 * \brief Parse the Celestron response and read two angles from it
 *
 * \param response	the response string to parse for angles
 */
std::pair<double, double>	CelestronMount::parseangles(
					const std::string& response) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parsing angle response: '%s'",
		response.c_str());
	uint32_t	a1, a2;
	if (2 != sscanf(response.c_str(), "%X,%X#", &a1, &a2)) {
		std::string	msg = stringprintf("cannot parse response '%s'",
			response.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	// long responses have a larger range, by shifting them we can now
	if (response.size() <= 10) {
		a1 <<= 16;
		a2 <<= 16;
	}
	// the second angle can be negative, if it is larger than 2^32, 
	// so we correct for that
	double	correction = 0;
	if (a2 >= 2147483648) {
		correction = - 2 * M_PI;
	}
	return std::pair<double, double>(angle(a1), angle(a2) + correction);
}

/**
 * \brief Retrieve the RA and DEC from the mount
 */
RaDec	CelestronMount::getRaDec() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locking for get command");
	std::unique_lock<std::recursive_mutex>	lock(_mount_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending get RaDec command");
	if (version >= 106) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "sending e command");
		write("e");
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "sending E command");
		write("E");
	}
	std::pair<double, double>	a = parseangles(readto('#'));
	if (a.second > M_PI) {
		a.second -= 2 * M_PI;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ra = %f, dec = %f",
		Angle(a.first).hours(), Angle(a.second).degrees());
	RaDec	result(Angle(a.first), Angle(a.second));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "radec = %s", result.toString().c_str());
	return result;
}

/*
 * \brief Retrieve the Azimuth and Altitude from the mount
 *
 * On GE mounts, Altitude und declination are essentially the same (except
 * the DEC is the result of an alignment modell), and Azimuth is essentially
 * the hour angle.
 */
AzmAlt	CelestronMount::getAzmAlt() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locking for get command");
	std::unique_lock<std::recursive_mutex>	lock(_mount_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending get AzmAlt (z) command");
	write("z");
	std::pair<double, double>	a = parseangles(readto('#'));
	return AzmAlt(Angle(a.first), Angle(a.second));
}

/**
 * \brief Find out whether the telescope is on the east or the west
 */
bool	CelestronMount::telescopePositionWest() {
	// XXX use the location on earth and the azm angle to find out
	// XXX on which side the telescope currently is, at least for
	// XXX GE mounts
	
#if 1
	// first query the mount to find out whether the telescope is 
	// actually equatorial by using the "t" command 
	bool	north = true; // XXX use t command

	// depending on the orientation, use the azm angle to decide whether
	// or not we are on the east/west side
	AzmAlt	azmalt = getAzmAlt();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got AzmAlt: %s",
		azmalt.toString().c_str());
	if (north) {
		return azmalt.azm() > Angle::right_angle;
	} else {
		return azmalt.azm() < Angle::right_angle;
	}
#endif

	// XXX until that is implemented, use the default method
	return Mount::telescopePositionWest();
}

/**
 * \brief Convert a 16 bit unsigned integer into an angle in radians
 *
 * \param a	the number to convert into a number of radians
 */
double	CelestronMount::angle(uint16_t a) {
	return 2 * M_PI * a / 65536.;
}

/**
 * \brief Convert a 32 bit unsigned integer into an angle in radians
 *
 * \param a	the number to convert into a number of radians
 */
double	CelestronMount::angle(uint32_t a) {
	return 2 * M_PI * a / 4294967296.;
}

/**
 * \brief Convert an angle into an unsigned 16 bit integer
 *
 * \param a	the angle to convert to a number
 */
uint16_t	CelestronMount::angle16(const Angle& a) {
	uint16_t	result = 65536 * a.reduced().radians() / (2 * M_PI);
	return result;
}

/**
 * \brief Convert an angle into an unsigned 32 bit integer
 *
 * \param a	the angle to convert to a number
 */
uint32_t	CelestronMount::angle32(const Angle& a) {
	uint32_t	result = 4294967296 * a.reduced().radians() / (2 * M_PI);
	return result;
}

/**
 * \brief Auxiliary function to hex encode a packet
 *
 * \param packet	raw data packet to encode
 * \return		hex encoded packet
 */
static std::string	packet2hex(const std::vector<uint8_t>& packet) {
	std::ostringstream	out;
	std::vector<uint8_t>::const_iterator	i;
	for (i = packet.begin(); i != packet.end(); i++) {
		if (i != packet.begin()) {
			out << ' ';
		}
		out << stringprintf("%02X", *i);
	}
	return out.str();
}

/**
 * \brief Send a GPS packet command
 *
 * The GPS packet command has two variable bytes, they are handed in as 
 * parameters a and b.
 *
 * \param a	byte index 3 in the packet
 * \param b	byte index 7 in the packet
 * \param l	size of the response
 */
std::vector<uint8_t>	CelestronMount::gps_command(uint8_t a, uint8_t b,
				size_t l) {
	std::unique_lock<std::recursive_mutex>	_lock(_mount_mutex);
	std::vector<uint8_t>	packet;
	packet.push_back('P');
	packet.push_back(1);
	packet.push_back(176);
	packet.push_back(a);
	packet.push_back(0);
	packet.push_back(0);
	packet.push_back(0);
	packet.push_back(b);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "write 8 bytes: %s",
		packet2hex(packet).c_str());
	writeraw(packet);
	std::vector<uint8_t>	result = readraw(l);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d byte response: %s",
		result.size(), packet2hex(result).c_str());
	getprompt();
	return result;
}

/**
 * \brief Find out whether GPS is available
 */
bool	CelestronMount::gps_linked() {
	std::vector<uint8_t>	x = gps_command(55, 1, 1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "gps linked: %s",
		(x[0] > 0) ? "yes" : "no");
	return (x[0] > 0);
}

/**
 * \brief Find the GPS longitude
 */
Angle	CelestronMount::gps_longitude() {
	std::vector<uint8_t>	xyz = gps_command(2, 3, 3);
	Angle	longitude(2 * M_PI * (65536 * xyz[0] + 256 * xyz[1] + xyz[2])
		/ 16777216.);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GPS longitude: %s",
		longitude.dms().c_str());
	return longitude;
}

/**
 * \brief Find the GPS latitude
 */
Angle	CelestronMount::gps_latitude() {
	std::vector<uint8_t>	xyz = gps_command(1, 3, 3);
	Angle	latitude(2 * M_PI * (65536 * xyz[0] + 256 * xyz[1] + xyz[2])
		/ 16777216.);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GPS latitude: %s",
		latitude.dms().c_str());
	return latitude;
}

/**
 * \brief Get the GPS date
 */
CelestronMount::gps_date_t	CelestronMount::gps_date() {
	std::vector<uint8_t>	xy = gps_command(3, 2, 2);
	gps_date_t	result;
	result.month = xy[0];
	result.day = xy[1];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GPS date: %d. %d.",
		result.day, result.month);
	return result;
}

/**
 * \brief Get the GPS year
 */
int	CelestronMount::gps_year() {
	std::vector<uint8_t>	xy = gps_command(4, 2, 2);
	int	year = (256 * xy[0] + xy[1]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GPS year: %d", year);
	return year;
}

/**
 * \brief Get the GPS time
 */
CelestronMount::gps_time_t	CelestronMount::gps_time() {
	std::vector<uint8_t>	xyz = gps_command(51, 3, 3);
	gps_time_t	result;
	result.hour = xyz[0];
	result.minute = xyz[1];
	result.seconds = xyz[2];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GPS time: %02d:%02d:%02d",
		result.hour, result.minute, result.seconds);
	return result;
}

/**
 * \brief Get the mount location
 */
LongLat	CelestronMount::location() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "location request");
	time_t	now;
	::time(&now);
	if (now > (_last_location_queried + CelestronMount::query_interval)) {
		if (gps_linked()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "have to read location");
			_last_location_queried = now;
			_last_location_source = Mount::GPS;
			LongLat	loc = LongLat(gps_longitude(), gps_latitude());
			Mount::location(loc);
		} else {
			// XXX should get location from HandControl, i.e. using
			// XXX the w command
			_last_location_source = Mount::LOCAL;
		}
	}
	return Mount::location();
}

/**
 * \brief Get the location source
 */
Mount::location_source_type	CelestronMount::location_source() {
	return _last_location_source;
}

/**
 * \find out whether a we can talk to the mount
 */
bool	CelestronMount::queriable(time_t last) {
	// if in goto, don't bother
	if (state() == Mount::GOTO) {
		return false;
	}
	time_t	now;
	::time(&now);
	return (now < (last + CelestronMount::query_interval));
}

/**
 * \brief Get the GPS time of the mount
 */
time_t	CelestronMount::time() {
	time_t	now;
	// if the last request is not too far back, use the offset to 
	// compute the current time, or if the mount is currently in
	// GOTO mode
	if (!queriable(_last_time_queried)) {
		::time(&now);
		return now + _last_time_offset;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "offset too old, retrieving GPS time");

	// if we have GPS, get the GPS time
	if (gps_linked()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "querying GPS time");
		struct tm	t;
		gps_time_t	h = gps_time();
		t.tm_sec = h.seconds;
		t.tm_min = h.minute;
		t.tm_hour = h.hour;

		gps_date_t	d = gps_date();
		t.tm_mday = d.day;
		t.tm_mon = d.month - 1;
		t.tm_year = gps_year() - 1900;

		t.tm_wday = 0; // ignored by timegm
		t.tm_yday = 0;
		t.tm_isdst = 0;
		t.tm_zone = NULL;
		t.tm_gmtoff = 0;

		// log what you have read from the mount
		char	buffer[40];
		strftime(buffer, sizeof(buffer), "%F %T", &t);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "date read from mount: %s",
			buffer);

		// convert into a time
		time_t	result = timegm(&t);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "GPS time found: %s",
			ctime_r(&result, buffer));

		// recompute the time offset
		::time(&now);
		_last_time_queried = now;
		_last_time_offset = (long long)result - (long long)now;

		// return the current time
		return result;
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no GPS link available");
		// XXX should read time from the hand control, i.e. using
		// XXX the h command
	}
	// if not linked, use the superclass time
	return Mount::time();
}

/**
 * \brief Find out whether we have guide rates
 */
bool	CelestronMount::hasGuideRates() {
	return true;
}

/**
 * \brief Get the guide rates
 */
RaDec	CelestronMount::getGuideRates() {
	double	rate = 0.5;
	double	frequency = 1/86400.;
	Angle	guiderate = rate * frequency * 4 * Angle::right_angle;
	return RaDec(guiderate, guiderate);
}

/**
 * \brief Run method for a move 
 */
void	CelestronMount::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mount thread starting");
	std::unique_lock<std::recursive_mutex>	lock(_mount_mutex);
	RaDec	position = getRaDec();
	while (_running) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "checking for state");
		// periodically check the mount state
		state_type	newstate = get_state();
		if (newstate != state()) {
			state(newstate);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "new state: %s",
				Mount::state2string(newstate).c_str());
		}

		// check for the mount position
		RaDec	newposition = getRaDec();
		if (position != newposition) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "new position: %s",
				newposition.toString().c_str());
			callback(newposition);
			position = newposition;
		}

		// if the GOTO has completed, terminate
		auto	delay = std::chrono::seconds(5);
		if (newstate == Mount::GOTO) {
			delay = std::chrono::seconds(1);
		}
		_mount_condition.wait_for(lock, delay);
	}
}

} // namespace celestron
} // namespace device
} // namespace astro


