%{
/*
 * grammar.y -- grammar for the guider command line
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdlib.h>
#include <cli.h>
#include <guidecli.h>
#include <FlexLexer.h>
#include <AstroDebug.h>
#include <NameService.h>
#include <listcommand.h>
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
%token EXIT
%token LIST
%token <argument>ARGUMENT
%token END_OF_FILE
%type <arguments>arguments
%union {
	std::string	*argument;
	std::vector<std::string>	*arguments;
}
%%
commandfile:
	commandlist END_OF_FILE {
		YYACCEPT;
	}
   ;

commandlist:
	commandline
   |	commandlist commandline
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

commandline:
	error '\n' {
		yyclearin;
		yyerrok;
	}
   |	EXIT '\n'  {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "exit command");
		YYACCEPT;
	}
   |	LIST arguments '\n' {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "list command");
		listcommand	c;
		c(*$2);
		delete $2;
		$2 = NULL;
	}
   ;

%%
