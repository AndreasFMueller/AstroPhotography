/*
 * astrocli.c -- astrophotography command language, mainly for testing
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <cli.h>
#include <getopt.h>
#include <iostream>
#include <fstream>

using namespace astro::cli;

int	debug = 0;
extern int	yydebug;

int	main(int argc, char *argv[]) {
	/* parse the command line */
	int	c;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debug++;
			break;
		}

	/* Remaining argument (if any) must be a command file to read */
	char	*filename = NULL;
	if (argc > optind + 1) {
		filename = argv[optind++];
	}

	/* we need a cli object, which we also register with the sharedcli
	   class, as the parser needs to access it */
	cli	cli;
	sharedcli	s(&cli);

	/* start parsing the input */
	cli.parse(filename);
	std::cout << cli;

	/* if we get here, then the input was completely accepted */
	exit(EXIT_SUCCESS);
}
