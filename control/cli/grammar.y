%{
/*
 * grammar.y -- grammar for the guider command line
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cstdlib>
#include <cli.h>
#include <guidecli.h>
#include <FlexLexer.h>
#include <AstroDebug.h>
#include <NameService.h>
#include <listcommand.h>
#include <locatorcommand.h>
#include <modulecommand.h>
#include <string>
#include <vector>

extern int	yylex();
extern int	yyerror(char *s);

using namespace astro::cli;

/**
 * Because the parser is not a class, that could hold its own private state,
 * we have to use a sharedcli object that does the same for us. Before the
 * parser is called, we have to initialize it (see astrocli.cpp)
 */
sharedcli	shcli;

%}
%token <argument>EXIT
%token <argument>ARGUMENT
%token <argument>COMMANDNAME
%token END_OF_FILE
%type <argument>commandname
%type <arguments>arguments
%union {
	std::string	*argument;
	std::vector<std::string>	*arguments;
}
%%
commandfile:
	commandlist END_OF_FILE {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "end of file");
		YYACCEPT;
	}
   ;

commandlist:
	commandline 		{ std::cout << shcli->prompt(); }
   |	commandlist commandline	{ std::cout << shcli->prompt(); }
   ;

arguments:
	ARGUMENT	{
		$$ = new std::vector<std::string>();
		$$->push_back(*$1);
		delete $1;
		$1 = NULL;
	}
   |	arguments ARGUMENT	{
		$$ = $1;
		$$->push_back(*$2);
		delete $2;
		$2 = NULL;
	}
   ;

commandname:
	COMMANDNAME	{ $$ = $1; }
   ;

commandline:
	'\n'	{
		debug(LOG_DEBUG, DEBUG_LOG, 0, "empty line is ok");
	}
   |	error '\n' {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "parse error: skip to EOL");
		yyclearin;
		yyerrok;
	}
   |	EXIT '\n'  {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "exit command");
		YYACCEPT;
	}
   |	commandname '\n' {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s command", $1->c_str());
		std::vector<std::string>	noarguments;
		clicommandptr	cmd = shcli->factory().get(*$1, noarguments);
		if (cmd) {
			try {
				(*cmd)(*$1, noarguments);
			} catch (std::exception& x) {
				std::cerr << "error in '" << *$1 << "' command: ";
				std::cerr << x.what() << std::endl;
			}
		} else {
			std::cerr << "command '" << *$1 << "' not known";
			std::cerr << std::endl;
		}
	}
   |	commandname arguments '\n' {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s command", $1->c_str());
		clicommandptr	cmd = shcli->factory().get(*$1, *$2);
		if (cmd) {
			try {
				(*cmd)(*$1, *$2);
			} catch (std::exception& x) {
				std::cerr << "error in " << *$1 << " command: ";
				std::cerr << x.what() << std::endl;
			}
		} else {
			std::cerr << "command '" << *$1 << "' not known";
			std::cerr << std::endl;
		}
		delete $2;
		$2 = NULL;
	}
   ;

%%
