%{
#include <iostream>
#include "JsonParser.hh"
#include "Json.h"

extern void yyerror(YYLTYPE *ltype, yyscan_t scanner, JSON *pJson, const char *msg);
%}

 /*%option outfile="JsonLexer.cc"*/

%option header-file="JsonLexer.hh"

%option 8bit reentrant noyywrap

%option stack

%option warn nodefault

%option bison-bridge bison-locations

 /*line number tracking*/
%option yylineno

 /*Instance of class JSON */
%option extra-type="JSON*"

%option nounput
%option noinput


space   [ \t\r\n]
digit   [0-9]
integer -?(0|[1-9]{digit}*)
number  {integer}\.{digit}
alpha   [a-zA-Z]
 /*ASCII Character*/
char    [\x20 -\x7F]
str     [\"\']+{char}*[\"\']+
hex     [0-9a-fA-F]
double  ({integer}|{number})[eE][+-]?{integer}+

%%

"}" return '}';
"{" return '{';
"[" return '[';
"]" return ']';
"," return ',';
":" return ':';

 /* eating up all the spaceses */
{space}* ;

 /*yyextra holds the pointer to instance of JSON.*/
 /*yylval is of YYSTYPE and YYSTYPE mapped to union defined in .y file.*/
"true"     {yylval->m_jvalue = yyextra->json_new_value(JSON::TRUE); return LITERAL;}
"false"    {yylval->m_jvalue = yyextra->json_new_value(JSON::FALSE); return LITERAL;}
"null"     {yylval->m_jvalue = yyextra->json_new_value(nullptr); return LITERAL;}
{integer}  {yylval->m_jvalue = yyextra->json_new_value(atoi(yytext)); return LITERAL;}
{number}   {yylval->m_jvalue = yyextra->json_new_value(atof(yytext)); return LITERAL;}
{double}   {yylval->m_jvalue = yyextra->json_new_value(strtod(yytext, nullptr)); return LITERAL;}
{str}      {yylval->m_jvalue = yyextra->json_new_value(yytext); return lSTRING;}


%%
