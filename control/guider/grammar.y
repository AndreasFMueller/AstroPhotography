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
%token <argument>LIST
%token <argument>LOCATOR
%token <argument>MODULE
%token <argument>ARGUMENT
%token END_OF_FILE
%type <argument>command_with_arguments
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

command_with_arguments:
   |	LIST	{ $$ = $1; }
   |	LOCATOR	{ $$ = $1; }
   |	MODULE	{ $$ = $1; }
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
   |	command_with_arguments arguments '\n' {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s command", $1->c_str());
		clicommandptr	cmd = shcli->factory().get(*$1, *$2);
		(*cmd)(*$1, *$2);
		delete $2;
		$2 = NULL;
	}
   ;

%%
