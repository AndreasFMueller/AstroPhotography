/*
 * connectiondialog.h -- ConnectionDialog implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "connectiondialog.h"
#include "ui_connectiondialog.h"
#include <pthread.h>
#include <AstroDebug.h>
#include <cassert>
#include <cerrno>

CORBA::ORB_ptr	ConnectionDialog::orb;
CosNaming::NamingContext_var	ConnectionDialog::namingcontext;
QString	ConnectionDialog::servername;

// the following pthread resources serve to synchronize orb and application
static pthread_t	orbthread;
static pthread_cond_t	orbcond;
static pthread_mutex_t	orbmutex;
static pthread_cond_t	orbwait;

/**
 * \brief Main function for the ORB thread
 *
 * This thread never exists, and handles all CORBA stuff
 */
void	*orb_main(void *params) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "orb thread starting up");
	// lock the mutex, i.e. wait until the main thread is ready and
	// releases this thread
	pthread_mutex_lock(&orbmutex);

	// the params pointer points to the server name
	char	*hostname = (char *)params;
	char	corbaname[1024];
	snprintf(corbaname, sizeof(corbaname), "NameService=corbaname::%s",
		hostname);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using name service %s", corbaname);

	// initialization of the orb
	const char	*options[][2] = {
		{ "giopMaxMsgSize", "40000000" },
		{ 0, 0 }
	};
	char	*corbaargv[3];
	corbaargv[0] = strdup("Guiding");
	corbaargv[1] = strdup("-ORBInitRef");
	corbaargv[2] = strdup(corbaname);
	int	corbaargc = 3;
	ConnectionDialog::orb = CORBA::ORB_init(corbaargc, corbaargv,
		"omniORB4", options);
	
	// orb is initialized
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ORB initialized: %p",
		ConnectionDialog::orb);

	// get the naming service
	CORBA::Object_var	obj;
	obj = ConnectionDialog::orb->resolve_initial_references("NameService");
	ConnectionDialog::namingcontext = CosNaming::NamingContext::_narrow(obj);
	if (CORBA::is_nil(ConnectionDialog::namingcontext)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get root cotext");
		return NULL;
	}

	// Get the root poa
	CORBA::Object_var	poaobj
		= ConnectionDialog::orb->resolve_initial_references("RootPOA");
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
	ConnectionDialog::orb->run();
	ConnectionDialog::orb->destroy();

	// we're done
	return NULL;
}

/**
 * \brief Create a ConnectionDialog
 */
ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog)
{
	ui->setupUi(this);

	// populate the selection
	// XXX this should come from a configuration file or some other
	//     resource for application defaults
	ui->comboBox->addItem("localhost");

}

/**
 * \brief Build the connection
 *
 * This method initializes the pthread mutex and condition variable
 */
void	ConnectionDialog::buildconnection(const QString _servername) {
	// remember the servername, the accept method might
	// find it useful
	servername = _servername;

	// convert the servername into a c string
	QByteArray ba = servername.toLatin1();
	const char *cservername = ba.data();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating connection to %s",
		cservername);

	// build a connection, i. e. start a corba thread 
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "orbmutex locked");

	// run a separate thread for the ORB
	pthread_attr_t	orbthreadattr;
	pthread_attr_init(&orbthreadattr);
	if (pthread_create(&orbthread, &orbthreadattr, orb_main,
		(void *)cservername)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start the orb thread: %s",
			strerror(errno));
		exit(EXIT_FAILURE);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "orb thread created");

	// the thread is now read to run, but it will not run because
	// it will block on the orbmutex. We atomically release it and
	// wait for the completion signal from the orb thread
	pthread_cond_wait(&orbcond, &orbmutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "orb thread completed initialization");
}

void	ConnectionDialog::accept() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "accept host %s",
		ui->comboBox->currentText().data());
	// get the selected server name from the combo box
	buildconnection(ui->comboBox->currentText());
}

ConnectionDialog::~ConnectionDialog()
{
    delete ui;
}
