#include "mainwindow.h"
#include <QApplication>
#include <module.hh>
#include <guider.hh>
#include <AstroDebug.h>
#include <cstdlib>
#include <pthread.h>
#include <cassert>
#include <cerrno>

CORBA::ORB_ptr	orb;
Astro::GuiderFactory_var	guiderfactory;

pthread_t	orbthread;
pthread_cond_t	orbcond;
pthread_mutex_t	orbmutex;

void	*orb_main(void *params) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "orb thread starting up");
	// lock the mutex, i.e. wait until the main thread is ready and
	// releases this thread
	pthread_mutex_lock(&orbmutex);

	// initialization of the orb
	const char	*options[][2] = {
		{ "giopMaxMsgSize", "40000000" },
		{ 0, 0 }
	};
	char	*corbaargv[3];
	corbaargv[0] = strdup("Guiding");
	corbaargv[1] = strdup("-ORBInitRef");
	corbaargv[2] = strdup("NameService=corbaname::localhost");
	//corbaargv[2] = strdup("NameService=corbaname::192.168.199.23");
	int	corbaargc = 3;
	orb = CORBA::ORB_init(corbaargc, corbaargv, "omniORB4", options);
	
	// orb is initialized
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ORB initialized: %p", orb);

	// get the naming service
	CORBA::Object_var	obj;
	obj = orb->resolve_initial_references("NameService");
	CosNaming::NamingContext_var	rootContext
		= CosNaming::NamingContext::_narrow(obj);
	if (CORBA::is_nil(rootContext)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get root cotext");
		return NULL;
	}

	CosNaming::Name	name;
	name.length(2);
	name[0].id = "Astro";
	name[0].kind = "context";
	name[1].id = "GuiderFactory";
	name[1].kind = "object";

	obj = rootContext->resolve(name);
	guiderfactory = Astro::GuiderFactory::_narrow(obj);
	if (CORBA::is_nil(guiderfactory)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get root context");
		return NULL;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a guider factory reference");

	// Get the root poa
	CORBA::Object_var	poaobj
		= orb->resolve_initial_references("RootPOA");
	PortableServer::POA_var	root_poa = PortableServer::POA::_narrow(poaobj);
	assert(!CORBA::is_nil(root_poa));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got root POA");

	// POA manager
	PortableServer::POAManager_var	pman = root_poa->the_POAManager();
	pman->activate();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "POAManager activated");

	// now everything is ready, and we can signal the main thread that
	// we are ok
	pthread_mutex_unlock(&orbmutex);
	pthread_cond_signal(&orbcond);

	// now run the ORB
	debug(LOG_DEBUG, DEBUG_LOG, 0, "run orb");
	orb->run();
	orb->destroy();

	// we're done
	return NULL;
}

int main(int argc, char *argv[]) {
	// unsure we are logging debug messages
	debuglevel = LOG_DEBUG;
	debugtimeprecision = 3;
	debugthreads = 1;

	// initialize the mutex and cond variable for the orb thread
	pthread_condattr_t	cattr;
	pthread_condattr_init(&cattr);
	pthread_cond_init(&orbcond, &cattr);
	pthread_mutexattr_t	mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&orbmutex, &mattr);

	// lock the mutex, thereby preventing the thread that we are
	// going to create shortly from running
	pthread_mutex_lock(&orbmutex);

	// run a separate thread for the ORB
	pthread_attr_t	orbthreadattr;
	pthread_attr_init(&orbthreadattr);
	if (pthread_create(&orbthread, &orbthreadattr, orb_main, NULL)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start the orb thread: %s",
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	// the thread is now read to run, but it will not run because
	// it will block on the orbmutex. We atomically release it and
	// wait for the completion signal from the orb thread
	pthread_cond_wait(&orbcond, &orbmutex);

	// now we start Qt initialization
	QApplication a(argc, argv);
	MainWindow w;
	w.guiderfactory = guiderfactory;
	w.show();

	return a.exec();
}
