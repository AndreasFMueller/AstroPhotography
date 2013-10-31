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
#include <AstroDebug.h>
%}
%option yylineno
%x QUOTED
%x ARGUMENTS
%%
exit|quit	{
		yylval.argument = new std::string(yytext);
		return EXIT;
	}

[a-zA-Z0-9]*	{
		yylval.argument = new std::string(yytext);
		yy_push_state(ARGUMENTS);
		return COMMANDNAME;
	}

<ARGUMENTS>[ \t]

<ARGUMENTS>[^ \t\n"]*	{
		yylval.argument = new std::string(yytext);
		//std::cerr << astro::stringprintf("argument string: %s",
		//	yylval.argument->c_str()) << std::endl;
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
		//std::cerr << astro::stringprintf("quoted string: %s",
		//	yylval.argument->c_str()) << std::endl;
		yy_pop_state();
		return ARGUMENT;
	}

\n	{
		return '\n';
	}

.	{
		return *yytext;
	}

<<EOF>>	{
		return END_OF_FILE;
	}
%%
