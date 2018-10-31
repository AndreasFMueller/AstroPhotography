//
// focusing.ice -- Interface to an autonomous focusing process
//
// (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
//
#include <camera.ice>
#include <Ice/Identity.ice>

module snowstar {

/**
 * \brief Focusing state
 */
enum FocusState {
	// an idle focuser just does not do anything. Not configured with
	// CCD or focuser is also idle
	FocusIDLE,
	// the focuser is currently moving, i.e. it is not possible to take
	// pictures
	FocusMOVING,
	// the camera is exposing, i.e. 
	FocusMEASURING,
	// the measurements have completed, we are waiting for the computation
	// of the focus point and the focuser moving there
	FocusMEASURED,
	// the focusing process completed, i.e. an optimal focus position was
	// found and the focuser has been moved to that position
	FocusFOCUSED,
	// the focusing process failed
	FocusFAILED
};

/**
 * \brief structures for the focusing history
 */
struct FocusPoint {
	int	position;
	double	value;
};
sequence<FocusPoint>	FocusHistory;

struct FocusElement {
	int	position;
	double	value;
	string	method;
	ImageBuffer	raw;
	ImageBuffer	evaluated;
};

interface FocusCallback {
	void	addFocusElement(FocusElement element);
	void	addPoint(FocusPoint point);
	void	changeState(FocusState state);
};

sequence<string>	FocusMethods;
sequence<string>	FocusSolvers;

/**
 * \brief Focusing interface
 *
 * This interface gives access to the 
 */
interface Focusing {
	/**
	 * \brief Get the current focusing state
	 */
	FocusState	status();

	/**
	 * \brief Get the method used to quantify focus quality
	 */
	string	method();

	/**
	 * \brief Set the focus quantification method
	 */
	void	setMethod(string m);

	string	solver();
	void	setSolver(string s);

	/**
	 * \brief get the exposure setting  used to find the focus
	 */
	Exposure	getExposure();

	/**
 	 * \brief set the exposure settings for the focusing process
	 */
	void	setExposure(Exposure e);

	/**
 	 * \brief Get the number of steps to be taken within the interval
	 */
	int	steps();

	/**
	 * \brief Set the number of steps into which to devide the interval
	 */
	void	setSteps(int s);

	/**
 	 * \brief start a new 
	 */
	void	start(int min, int max);

	/**
	 * \brief Cancel the current focusing process
	 */
	void	cancel();

	/**
	 * \brief 
	 *
	 * Get a proxy to the ccd. This allows to query the current exposure
	 * state easily.
	 */
	Ccd*	getCcd();

	/**
 	 * \brief Focuser used in the focusing process
	 *
	 * This method is provided so that it becomes easier to access the 
	 * current focuser position
	 */
	Focuser*	getFocuser();

	/**
 	 * \brief Get the focusing history
	 */
	FocusHistory	history();

	// if the repository name is set, then all images sent to the
	// callback will be added to the repository
	void	setRepositoryName(string reponame) throws NotFound;
	string	getRepositoryName();

	/**
	 * \brief add a callback to the server
	 */
	void	registerCallback(Ice::Identity callbackidentity);

	/**
	 * \brief remove a callback from the server
	 */
	void	unregisterCallback(Ice::Identity callbackidentity);
};

/**
 * \brief A factory to retrive Focusing classes
 *
 * Focusing classes are identified by a ccd and a focuser. Internally in the
 * server, the factory keeps a running list of ids and creates proxies using
 * these ids.
 */
interface FocusingFactory {
	Focusing*	get(string ccd, string focuser);
	FocusMethods	getMethods();
	FocusSolvers	getSolvers();

};

};
