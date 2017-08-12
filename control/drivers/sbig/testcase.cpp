/*
 * testcase.cpp -- verify a problem in the temp status command
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_LPARDRV_H
#include <lpardrv.h>
#else
#ifdef HAVE_SBIGUDRV_LPARDRV_H
#include <SBIGUDrv/lpardrv.h>
#endif /* HAVE_SBIGUDRV_LPARDRV_H */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void	perror(const char *errormsg, short errorcode) {
	GetErrorStringParams    params;
	params.errorNo = errorcode;
	GetErrorStringResults   results;

	SBIGUnivDrvCommand(CC_GET_ERROR_STRING, &params, &results);
	fprintf(stderr, "%s: %s\n", errormsg, results.errorString);
}

int	main(int /* argc */, char * /* argv */[]) {
	printf("open driver\n");
	short	e = SBIGUnivDrvCommand(CC_OPEN_DRIVER, NULL, NULL);
	if (e != CE_NO_ERROR) {
		perror("cannot open driver", e);
		exit(EXIT_FAILURE);
	}

	printf("open device\n");
	OpenDeviceParams	openparams;
	memset(&openparams, 0, sizeof(openparams));
	openparams.deviceType = 0x7f02;
	e = SBIGUnivDrvCommand(CC_OPEN_DEVICE, &openparams, NULL);
	if (e != CE_NO_ERROR) {
		perror("cannot open device", e);
		exit(EXIT_FAILURE);
	}

	printf("establish link\n");
	EstablishLinkParams	establishparams;
	memset(&establishparams, 0, sizeof(establishparams));
	establishparams.sbigUseOnly = 0;
	EstablishLinkResults	results;
	e = SBIGUnivDrvCommand(CC_ESTABLISH_LINK, &establishparams, &results);
	if (e != CE_NO_ERROR) {
		perror("cannot establish link", e);
		exit(EXIT_FAILURE);
	}

	printf("get driver info\n");
	GetDriverInfoParams	driverinfoparams;
	memset(&driverinfoparams, 0, sizeof(driverinfoparams));
	driverinfoparams.request = 0;
	GetDriverInfoResults0	driverinfo;
	e = SBIGUnivDrvCommand(CC_GET_DRIVER_INFO, &driverinfoparams,
		&driverinfo);
	if (e != CE_NO_ERROR) {
		perror("cannot get driver info", e);
		exit(EXIT_FAILURE);
	}
	printf("driver: %s, version %04x\n", driverinfo.name,
		driverinfo.version);

	QueryTemperatureStatusParams    tempparams;
	tempparams.request = TEMP_STATUS_STANDARD;
	QueryTemperatureStatusResults	tempresults;
	e = SBIGUnivDrvCommand(CC_QUERY_TEMPERATURE_STATUS,
		&tempparams, &tempresults);
	if (e != CE_NO_ERROR) {
		perror("cannot get temperature info", e);
		exit(EXIT_FAILURE);
	}
	printf("enabled: %s, setPoint: %hd\n", 
		(tempresults.enabled) ? "YES" : "NO", tempresults.ccdSetpoint);

	QueryTemperatureStatusResults2  tempresults2;
	tempparams.request = TEMP_STATUS_ADVANCED2;
	e = SBIGUnivDrvCommand(CC_QUERY_TEMPERATURE_STATUS,
		&tempparams, &tempresults2);
	if (e != CE_NO_ERROR) {
		perror("cannot get temperature info", e);
		exit(EXIT_FAILURE);
	}

#if 0
	CFWParams	cfwparams;
	memset(&cfwparams, 0, sizeof(cfwparams));
	cfwparams.cfwModel = CFWSEL_AUTO;
	cfwparams.cfwCommand = CFWC_QUERY;
	CFWResults	cfwresults;
	e = SBIGUnivDrvCommand(CC_CFW, &cfwparams, &cfwresults);
	if (e != CE_NO_ERROR) {
		perror("cannot query filter wheel", e);
		exit(EXIT_FAILURE);
	}
	printf("Model: %hd, Position: %hd, status: %hd, error: %hd\n",
		cfwresults.cfwModel, cfwresults.cfwPosition,
		cfwresults.cfwStatus, cfwresults.cfwError);
#endif

	return EXIT_SUCCESS;
}
