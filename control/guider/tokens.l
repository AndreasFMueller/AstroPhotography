%{
/*
 * lexer for the guider command line program
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cli.h>
#include <vector>
#include <string>
#include <grammar.hpp>
#include <AstroFormat.h>
%}
%option yylineno
%x QUOTED
%x ARGUMENTS
%%
list	{
		yy_push_state(ARGUMENTS);
		return LIST;
	}
module	{
		yy_push_state(ARGUMENTS);
		return MODULE;
	}
locator	{
		yy_push_state(ARGUMENTS);
		return LOCATOR;
	}

exit	{
		return EXIT;
	}

<ARGUMENTS>[^ \t\n"]*	{
		yylval.argument = new std::string(yytext);
		std::cerr << astro::stringprintf("argument string: %s",
			yylval.argument->c_str()) << std::endl;
		return ARGUMENT;
	}

<ARGUMENTS>\n	{
		yy_pop_state();
		return '\n';
	}

<ARGUMENTS>\"	{
		yy_push_state(QUOTED);
	}

<QUOTED>[^"\n]*	{
		yylval.argument = new std::string(yytext);
		std::cerr << astro::stringprintf("quoted string: %s",
			yylval.argument->c_str()) << std::endl;
		return ARGUMENT;
	}

\n	{
		return '\n';
	}

<<EOF>>	{ return END_OF_FILE; }
%%
