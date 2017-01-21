/*
 * Restart.h -- auxiliary class to restart a server
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */

namespace snowstar {

/**
 * \brief auxiliary class used for s
 */
class Restart {
	char	**arguments;
	Restart(const Restart& other);
	Restart&	operator=(const Restart& other);
static bool	_shutdown_instead;
public:
	Restart(int argc, char *argv[]);
	void	exec();
static bool	shutdown_instead() { return _shutdown_instead; }
static void	shutdown_instead(bool s);
};

} // namespace snowstar
