#ifndef __JSON_CC__
#define __JSON_CC__

#include <iostream>
#include "ace/Log_Msg.h"
#include "Json.h"
#include "JsonParser.hh"
#include "JsonLexer.hh"

JSON *JSON::m_instance = nullptr;

JSON *JSON::instance(void)
{
  if(nullptr == m_instance)
  {
    ACE_NEW_NORETURN(m_instance, JSON());
  }

  return(m_instance);
}

JSON *JSON::get_instance(void)
{
  return(m_instance);
}

JSON::JSON()
{
  ACE_NEW_NORETURN(m_value, JSONValue());
}

JSON::~JSON()
{
  m_instance = nullptr;
  delete m_value;

  m_value = nullptr;
}

JSON::JSONValue *JSON::value(void)
{
  return(m_value);
}

void JSON::value(JSONValue *value)
{
  m_value = value;
}

int JSON::start(const ACE_TCHAR *fname)
{
  int ret = -1;

  FILE *in = nullptr;

  yyscan_t scanner;

  if(fname)
  {
    in = fopen(fname, "r");
    if(!in)
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l opening of file %s filed\n"), fname));
      return(0);
    }

  }
  else
    in = stdin;

  yylex_init_extra(this, &scanner);
  yyset_in(in, scanner);

  ret = yyparse(scanner, this);
  yylex_destroy(scanner);

  if(in != nullptr)
  {
    fclose(in);
    in = nullptr;
  }

  return(ret);
}

JSON::JSONValue *JSON::json_new_value(void)
{
  JSONValue *value = nullptr;

  ACE_NEW_RETURN(value, JSONValue(), 0);
  return(value);
}

JSON::JSONValue *JSON::json_new_value_int(int i)
{
  JSONValue *value = json_new_value();

  if(value)
  {
    value->m_type = JSON::JSON_VALUE_TYPE_INTEGER;
    value->m_ivalue = i;
  }

  return(value);
}

JSON::JSONValue *JSON::json_new_value_double(double d)
{
  JSONValue *value = json_new_value();

  if(value)
  {
    value->m_type = JSON::JSON_VALUE_TYPE_DOUBLE;
    value->m_ivalue = d;
  }

  return(value);
}

JSON::JSONValue *JSON::json_new_value_str(char *str)
{
  JSONValue *value = json_new_value();

  if(value)
  {
    value->m_type = JSON::JSON_VALUE_TYPE_STRING;
    value->m_svalue = ACE_OS::strdup(str);
  }

  return(value);
}

JSON::JSONValue *JSON::json_new_value(char *str, int size)
{
  return(nullptr);
}

JSON::JSONValue *JSON::json_new_value_object(JSONObject *object)
{
  if(nullptr == object)
    return(nullptr);

  JSONValue *value = json_new_value();
  value->m_type = JSON::JSON_VALUE_TYPE_OBJECT;
  value->m_ovalue = object;

  return(value);
}

JSON::JSONValue *JSON::json_new_value_array(JSONArray *array)
{
  if(nullptr == array)
    return(nullptr);

  JSONValue *value = json_new_value();
  value->m_type = JSON::JSON_VALUE_TYPE_ARRAY;
  value->m_avalue = array;

  return(value);
}

JSON::JSONValue *JSON::json_new_value_bool(const ACE_UINT8 tOF)
{
  JSONValue *value = json_new_value();

  if(JSON::TRUE == tOF)
  {
    value->m_type = JSON::JSON_VALUE_TYPE_TRUE;
  }
  else
  {
    value->m_type = JSON::JSON_VALUE_TYPE_FALSE;
  }

  return(value);
}

JSON::JSONValue *JSON::json_new_value(std::nullptr_t nullp)
{
  JSONValue *value = json_new_value();

  if(value)
    value->m_type = JSON::JSON_VALUE_TYPE_NULL;

  return(value);
}

JSON::JSONElement *JSON::json_new_element(JSONValue *value)
{
  JSONElement *element = nullptr;

  ACE_NEW_RETURN(element, JSONElement(), 0);

  element->m_value = value;
  element->m_next = nullptr;

  return(element);
}

JSON::JSONArray *JSON::json_new_array(JSONElement *element)
{
  JSONArray *array = nullptr;

  ACE_NEW_RETURN(array, JSONArray(), 0);

  array->m_elements = element;

  return(array);
}

JSON::JSONMember *JSON::json_new_member(JSONValue *key, JSONValue *value)
{
  JSONMember *member = nullptr;

  ACE_NEW_RETURN(member, JSONMember(), 0);

  member->m_key = key;
  member->m_value = value;
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l Value of Key is %s\n"), key->m_svalue));
  return(member);
}

JSON::JSONObject *JSON::json_new_object(JSONMembers *members)
{
  JSONObject *object = nullptr;

  if(nullptr == members)
    return(nullptr);

  ACE_NEW_NORETURN(object, JSONObject());

  object->m_members = members;

  return(object);
}

JSON::JSONMembers *JSON::json_add_member_in_members(JSONMembers *members, JSONMember *member)
{
  if(nullptr == members)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l members is nullptr\n")));
    return(json_new_members(member));
  }

  JSONMembers *m = nullptr;

  for(m = members; m->m_next != nullptr; m = m->m_next)
    ;

  m->m_next = json_new_members(member);

  return(members);
}

JSON::JSONMembers *JSON::json_new_members(JSONMember *member)
{
  JSONMembers *members = nullptr;

  ACE_NEW_RETURN(members, JSONMembers(), 0);

  members->m_member = member;
  members->m_next = nullptr;

  return(members);
}

JSON::JSONObject *JSON::json_new_object(void)
{
  JSONObject *object = nullptr;

  ACE_NEW_RETURN(object, JSONObject(), 0);
  object->m_members = nullptr;

  return(object);
}

JSON::JSONArray *JSON::json_new_array(void)
{
  JSONArray *array = nullptr;

  ACE_NEW_RETURN(array, JSONArray(), 0);

  array->m_elements = nullptr;

  return(array);
}

void JSON::json_free(JSONValue *value)
{
  if(nullptr == value)
    return;

  switch(value->m_type)
  {
  case JSON::JSON_VALUE_TYPE_STRING:
    delete value->m_svalue;
    break;

  case JSON::JSON_VALUE_TYPE_OBJECT:
    json_free(value->m_ovalue);
    break;

  case JSON::JSON_VALUE_TYPE_ARRAY:
    json_free(value->m_avalue);
    break;

  default:
    break;
  }

  delete value;
}

void JSON::json_free(JSONElement *element)
{
  if(nullptr == element)
    return;

  json_free(element->m_next);
  delete element;
}

void JSON::json_free(JSONArray *array)
{
  if(nullptr == array)
    return;

  json_free(array->m_elements);
  delete array;
}

void JSON::json_free(JSONMember *member)
{
  if(nullptr == member)
    return;

  json_free(member->m_key);
  json_free(member->m_value);

  delete member;
}

void JSON::json_free(JSONObject *object)
{
  if(nullptr == object)
    return;

  //json_free(object->m_members);
  delete object;
}

JSON::JSONElement *JSON::json_value_add_element(JSONElement *element, JSONValue *value)
{
  if(nullptr == element)
    return(json_new_element(value));

  JSONElement *e = nullptr;
  for(e = element; e->m_next != nullptr; e = e->m_next)
    ;

  e->m_next = json_new_element(value);

  return(element);
}

JSON::JSONValue *JSON::operator[](int index)
{
  JSONElement *e = nullptr;

  if(m_value->m_type != JSON::JSON_VALUE_TYPE_ARRAY)
    return(nullptr);

  for(e = m_value->m_avalue->m_elements; index && (e != nullptr); index--, e = e->m_next)
    ;

  if(e != nullptr)
    return(e->m_value);

  return(nullptr);

}
/*
 * @brief
 * @param
 * @param
 *
 * @return either nullptr or value at given index.
 * */
JSON::JSONValue *JSON::json_value_at_index(JSONValue *value, int index)
{
  JSONElement *e = nullptr;

  if(value->m_type != JSON::JSON_VALUE_TYPE_ARRAY)
    return(nullptr);

  for(e = value->m_avalue->m_elements; index && (e != nullptr); index--, e = e->m_next)
    ;

  if(e != nullptr)
    return(e->m_value);

  return(nullptr);
}

JSON::JSONValue *JSON::json_value_at_key(JSONValue *value, char *key)
{
  JSONMembers *m = nullptr;

  if((nullptr == value) ||
     (value->m_type != JSON::JSON_VALUE_TYPE_OBJECT) ||
     (nullptr == key))
    return(nullptr);

  for(m = value->m_ovalue->m_members; m != nullptr; m = m->m_next)
  {
    if((m->m_member->m_key->m_type == JSON::JSON_VALUE_TYPE_STRING) &&
       (!strcmp(m->m_member->m_key->m_svalue, key)))
    {
      return(m->m_member->m_value);
    }
  }

  return(nullptr);
}

#endif /*__JSON_CC__*/
