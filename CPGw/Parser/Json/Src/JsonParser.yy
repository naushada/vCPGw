%{
#include <iostream>
#include <cstddef>
#include "Json.h"
#include "JsonParser.hh"

using namespace std;
typedef void* yyscan_t;
extern int yylex(YYSTYPE *stype, YYLTYPE *ltype, yyscan_t scanner);
void yyerror(YYLTYPE *ltype, yyscan_t scanner, JSON *pJson, const char *msg);
%}


%code requires {
#include "Json.h"
#include "JsonParser.hh"
typedef void* yyscan_t;
}


%initial-action
{
  /*This piece of code is called before parsing.*/
}

%union {
  JSON::JSONObject  *m_jobject;
  JSON::JSONMember  *m_jmember;
  JSON::JSONArray   *m_jarray;
  JSON::JSONElement *m_jelement;
  JSON::JSONValue   *m_jvalue;
  JSON::JSONMembers *m_jmembers;
}

%token <m_jvalue> lSTRING LITERAL

%type <m_jobject>  object
%type <m_jmember>  member
%type <m_jmembers> members
%type <m_jarray>   array
%type <m_jelement> element
%type <m_jvalue>   value

%locations
%define parse.error verbose
%define parse.lac full
%define api.pure full

%param {yyscan_t scanner}
%parse-param {JSON *pJson}

%start input

/* Rules Section */
%%

input
 : %empty
 | value {pJson->value($1);}
 ;

value
 : LITERAL
 | lSTRING
 | object   {$$ = pJson->json_new_value_object($1);}
 | array    {$$ = pJson->json_new_value_array($1);}
 ;

object
 : '{' '}'         {$$ = pJson->json_new_object(nullptr);}
 | '{' members '}' {$$ = pJson->json_new_object($2);}
 ;

members
 : member              {$$ = pJson->json_new_members($1);}
 | members ',' member  {$$ = pJson->json_add_member_in_members($1, $3);}
 ;

member
 : lSTRING ':' value   {$$ = pJson->json_new_member($1, $3);}
 ;

array
 : '[' ']'         {$$ = pJson->json_new_array(nullptr);}
 | '[' element ']' {$$ = pJson->json_new_array($2);}
 ;

element
 : value             {$$ = pJson->json_new_element($1);}
 | element ',' value {$$ = pJson->json_value_add_element($1, $3);}
 ;

%%

void yyerror(YYLTYPE *yylloc, yyscan_t yyscanner,
             JSON* pJson, const char *msg)
{
  std::cout << "Error - " << msg << std::endl;
}
