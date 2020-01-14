%{

/* Declarations section */
#include <stdio.h>
#include "scopes.h"
#include "hw3_output.hpp"
#include "parser.tab.hpp"

using namespace output;
using namespace scopeTables;
using namespace std; //todo - removes
extern stack<Scope*> scopes;  //todo --remove 
extern void printScopes(stack<Scope*>* scopes); //todo -remove

%}

%option yylineno
%option noyywrap

digit   		([0-9])
positive		([1-9])
letter  		([a-zA-Z])
whitespace		([\t\r\n ])
comment 		(\/\/[^\r\n]*[\r|\n|\r\n]?)
relational 		(<|>|(<=)|(>=))
equality		((==)|(!=))
		



%%

STA#								{ printScopes(&scopes); } //todo - remove
P#									{ cout << "@#$ DETECTOR @#$" << endl; } //todo - remove
void								return VOID;
int 								return INT;
byte 								return BYTE;
b 	 								return B;
bool 								return BOOL;
enum								return ENUM;
and 								return AND;
or	 								return OR;
not	 								return NOT;
true 								return TRUE;
false 								return FALSE;
return 								return RETURN;
if	 								return IF;
else 								return ELSE;
while								return WHILE;
break								return BREAK;
continue 							return CONTINUE;
;									return SC;
,									return COMMA;
\(									return LPAREN;
\)									return RPAREN;
\{									return LBRACE;
\}									return RBRACE;
=									return ASSIGN;
{relational}						{
										yylval = new operationEntry(yytext); 
										return RELATION;	
									}		
{equality}							{
										yylval = new operationEntry(yytext); 
										return EQUALITY;
									}	
\*									{
										yylval = new operationEntry(yytext); 
										return MUL;
									}
\/									{
										yylval = new operationEntry(yytext); 
										return DIV;
									}
\-									{
										yylval = new operationEntry(yytext); 
										return SUB;
									}
\+									{
										yylval = new operationEntry(yytext); 
										return ADD;
									}
{letter}({letter}|{digit})*			{
										yylval = new variableEntry(yytext);
										return ID;
									};
0|{positive}+{digit}*				{
										yylval = new variableEntry("", "", 0, atoi(yytext));
										return NUM;
									};
\"([^\n\r\"\\]|\\[rnt"\\])+\"		return STRING;
{comment}							;
{whitespace}						;
.									{ 	
										errorLex(yylineno);
										exit(0);
									};

%%






