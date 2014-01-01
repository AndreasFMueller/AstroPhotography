#include "mainwindow.h"
#include <QApplication>
#include <module.hh>
#include <guider.hh>
#include <AstroDebug.h>
#include <cstdlib>

CORBA::ORB_ptr	orb;
Astro::GuiderFactory_var	guiderfactory;

int main(int argc, char *argv[]) {
	const char	*options[][2] = {
		{ "giopMaxMsgSize", "40000000" },
		{ 0, 0 }
	};
	char	*corbaargv[3];
	corbaargv[0] = strdup(argv[0]);
	corbaargv[1] = strdup("-ORBInitRef");
	//corbaargv[2] = strdup("NameService=corbaname::localhost");
	corbaargv[2] = strdup("NameService=corbaname::192.168.199.23");
	int	corbaargc = 3;
	orb = CORBA::ORB_init(corbaargc, corbaargv, "omniORB4", options);
	
	debuglevel = LOG_DEBUG;
	debugtimeprecision = 3;
	debugthreads = 1;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "corba initialized: %p", orb);

	CORBA::Object_var	obj;
	obj = orb->resolve_initial_references("NameService");
	CosNaming::NamingContext_var	rootContext
		= CosNaming::NamingContext::_narrow(obj);
	if (CORBA::is_nil(rootContext)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get root cotext");
		return EXIT_FAILURE;
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
		return EXIT_FAILURE;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a guider factory reference");

	QApplication a(argc, argv);
	MainWindow w;
	w.guiderfactory = guiderfactory;
	w.show();

	return a.exec();
}
