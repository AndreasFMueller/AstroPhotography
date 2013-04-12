%{
/*
 * parser for the command line interpreter for the astrophotography project
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdlib.h>
#include <cli.h>
#include <FlexLexer.h>

extern int	yylex();
extern int yyerror(char *s);

using namespace astro::cli;

/**
 * Because the parser is not a class, that could hold its own private state,
 * we have to use a sharedcli object that does the same for us. Before the
 * parser is called, we have to initialize it (see astrocli.cpp)
 */
sharedcli	shcli;

%}
%union {
	double	doublevalue;
#define MAXVARSIZE	120
	char	variablename[MAXVARSIZE];
}
%token ERROR
%token <doublevalue>NUMBER
%token <variablename>VARIABLE
%token END_OF_FILE
%type <doublevalue>expression term factor
%type <doublevalue>assignment
%%
commandfile:
	commandlist END_OF_FILE	{
		YYACCEPT;
	}
    ;

commandlist:
	commandlist commandline
    |	commandline
    ;

commandline:
	command '\n'
    |	error '\n'	{
		yyclearin;	/* discard lookahead */
		yyerrok;
	}
    ;

command:
	assignment
    |	expression
    ;

assignment:
	VARIABLE '=' expression	
	{
		std::string	varname($1);
		if (shcli.vars().contains(varname)) {
			// the variable already exists, assign it the new
			// value from the expression
			ValuePtr	p = shcli.vars()[varname];
			*((value<double> *)&*p)->val() = $3;
		} else {
			// the variable does not exist yet, create a new
			// ValuePtr	object and put it into the map
			ValuePtr	varptr(new value<double>(new double($3)));
			shcli.vars()[varname] = varptr;
		}
		$$ = $3;
	}
    ;

expression:
	expression '+' term	{ $$ = $1 + $3; }
    |	expression '-' term	{ $$ = $1 - $3; }
    |	term
    ;

term:
	term '*' factor
    |	term '/' factor
    |	factor
    ;

factor:
	'(' expression ')'
    |	NUMBER	{ $$ = $1; }
    |	VARIABLE {
		// find the variable and assign the value of it to
		// the expression
		std::string	varname($1);
		if (shcli.vars().contains(varname)) {
			// the variable exists, get its value
			ValuePtr	p = shcli.vars()[varname];
			double	v = *((value<double> *)&*p)->val();
			$$ = v;
		} else {
			// if the variable does not exist, just return 0
			$$ = 0.;
		}
	}
    ;

%%
