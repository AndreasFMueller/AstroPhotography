%{
/*
 * lexer for the command line interpreter for the astrophotography project
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cli.h>
#include <grammar.hpp>

%}
%option yylineno
%%
\$[a-zA-Z0-9\.]*	{
		// generate an error if a variable is too long
		if (strlen(yytext) > (MAXVARSIZE - 2)) {
			fprintf(stderr, "variable %s too long (max %d)\n",
				yytext, MAXVARSIZE);
			return ERROR;
		}
		strncpy(yylval.variablename, yytext + 1, MAXVARSIZE);
		return VARIABLE;
	}

[+\-]?[0-9]+(\.[0-9]*)?([eE][+]-]?[0-9]+)?	{
		yylval.doublevalue = atof(yytext);
		return NUMBER;
	}

[=;\-+*/()]	{
		return yytext[0];
	}

" "	{ }
^#.*\n	{ printf("comment: %s", yytext); }
\n	{ return '\n'; }
<<EOF>>	{ return END_OF_FILE; }

%%
