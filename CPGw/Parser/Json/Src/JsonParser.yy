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
/*#include "JsonLexer.hh"*/
typedef void* yyscan_t;
/*extern int yylex(YYSTYPE *stype, YYLTYPE *ltype, yyscan_t scanner);*/
/*void yyerror(YYLTYPE*, yyscan_t scanner, JSON *pJson, const char *);*/
}


%initial-action
{
  /*This piece of code is called before parsing.*/
}

%union {
  JSON::JSONObject *m_jobject;
  JSON::JSONMember *m_jmember;
  JSON::JSONArray *m_jarray;
  JSON::JSONElement *m_jelement;
  JSON::JSONValue *m_jvalue;
}

%token <m_jvalue> lSTRING LITERAL

%type <m_jobject> object
%type <m_jmember> member
%type <m_jarray> array
%type <m_jelement> element
%type <m_jvalue> value

 /*%define "Inc/JsonParser.h"*/
 /*%output "Src/JsonParser.cc"*/

%locations
%verbose
%define parse.trace
%define api.pure full

%param {yyscan_t scanner}
%parse-param {JSON *pJson}


%start input

/* Rules Section */
%%

input
 : /*empty*/
 | value {pJson->value($1);}
 ;

value
 : LITERAL
 | lSTRING
 | object   {$$ = pJson->json_new_value($1);}
 | array    {$$ = pJson->json_new_value($1);}
 ;

object
 : '{' '}'        {$$ = pJson->json_new_object();}
 | '{' member '}' {$$ = pJson->json_new($2);}
 ;

member
 : lSTRING ':' value            {pJson->json_new($1, $3);}
 | member ',' lSTRING ':' value {$$ = pJson->json_value_add_member($1, $3, $5);}
 ;

array
 : '[' ']'         {$$ = pJson->json_new_array();}
 | '[' element ']' {$$ = pJson->json_new($2);}
 ;

element
 : value             {$$ = pJson->json_new($1);}
 | element ',' value {$$ = pJson->json_value_add_element($1, $3);}
 ;

%%

void yyerror(YYLTYPE *yylloc, yyscan_t yyscanner,
             JSON* pJson, const char *msg)
{
  std::cout << "Error - " << msg << std::endl;
}
