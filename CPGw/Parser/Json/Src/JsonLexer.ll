%{
#include <iostream>
#include "JsonParser.hh"
#include "Json.h"

extern void yyerror(YYLTYPE *ltype, yyscan_t scanner, JSON *pJson, const char *msg);
%}

%option header-file="JsonLexer.hh"
%option 8bit reentrant noyywrap
%option stack
%option warn
%option default
%option bison-bridge bison-locations
 /*line number tracking*/
%option yylineno
 /*Instance of class JSON */
%option extra-type="JSON*"
 /*This is required to remove warnings.*/
%option nounput
%option noinput

 /*Exclusive State*/
%x STRING_ST

space           [ \t\r\n]
digit           [0-9]
integer         -?(0|[1-9]{digit}*)
number          {integer}\.{digit}
alpha           [a-zA-Z]

 /*ASCII Printable Character*/
char            [\x20 -\x7F]
 /*Printable character may be json delimiter \", :, , so excluding \"*/
except_quote    [^\x22]

double_quote    (\")
hex             [0-9a-fA-F]
double          ({integer}|{number})[eE][+-]?{integer}+

%%

"}" return '}';
"{" return '{';
"[" return '[';
"]" return ']';
"," return ',';
":" return ':';

 /*eating up all the spaceses */
{space}*        {;}

 /*yyextra holds the pointer to instance of JSON.*/
 /*yylval is of YYSTYPE and YYSTYPE mapped to union defined in .yy file.*/

"true"          {yylval->m_jvalue = yyextra->json_new_value_bool(JSON::TRUE); return LITERAL;}

"false"         {yylval->m_jvalue = yyextra->json_new_value_bool(JSON::FALSE); return LITERAL;}

"null"          {yylval->m_jvalue = yyextra->json_new_value(nullptr); return LITERAL;}

{integer}       {yylval->m_jvalue = yyextra->json_new_value_int(atoi(yytext)); return LITERAL;}

{double}        {yylval->m_jvalue = yyextra->json_new_value_double(strtod(yytext, nullptr)); return LITERAL;}

{double_quote}  {BEGIN(STRING_ST);}

 /*The default state of flex is INITIAL the STRING_ST is required because string in double quote
  *can have json delimiters. the delimiters of json is : or ,
  */

<STRING_ST>{except_quote}* {yylval->m_jvalue = yyextra->json_new_value_str(yytext); return(lSTRING);}

<STRING_ST>{double_quote}  {BEGIN(INITIAL);}

 /*This is the default rule if none of above matches. withour this flex reports \"flex is jammed error.\"*/
.               {std::cout << "unsupported input "<< yytext << "line no:"<<yylineno << std::endl;}

%%
