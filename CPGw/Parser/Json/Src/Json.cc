#ifndef __JSON_CC__
#define __JSON_CC__

#include "Json.h"
//#include "JsonParser.hh"

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
  m_value = nullptr;
}

JSON::~JSON()
{
  m_instance = nullptr;
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
#if 0
  FILE *in = nullptr;

  yyscan_t scanner;

  if(fname)
    fopen_s(&in, fname, "r");
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
#endif
  return(ret);
}

JSON::JSONValue *JSON::json_new_value(void)
{
  JSONValue *value = nullptr;

  ACE_NEW_RETURN(value, JSONValue(), 0);
  return(value);
}

JSON::JSONValue *JSON::json_new_value(int i)
{
  JSONValue *value = json_new_value();

  if(value)
  {
    value->m_type = JSON::JSON_VALUE_TYPE_INTEGER;
    value->m_ivalue = i;
  }

  return(value);
}

JSON::JSONValue *JSON::json_new_value(double d)
{
  JSONValue *value = json_new_value();

  if(value)
  {
    value->m_type = JSON::JSON_VALUE_TYPE_DOUBLE;
    value->m_ivalue = d;
  }

  return(value);
}

JSON::JSONValue *JSON::json_new_value(char *str)
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

JSON::JSONValue *JSON::json_new_value(JSONObject *object)
{
  if(nullptr == object)
    return(nullptr);

  JSONValue *value = json_new_value();
  value->m_type = JSON::JSON_VALUE_TYPE_OBJECT;
  value->m_ovalue = object;

  return(value);
}

JSON::JSONValue *JSON::json_new_value(JSONArray *array)
{
  if(nullptr == array)
    return(nullptr);

  JSONValue *value = json_new_value();
  value->m_type = JSON::JSON_VALUE_TYPE_ARRAY;
  value->m_avalue = array;

  return(value);
}

JSON::JSONValue *JSON::json_new_value(const ACE_UINT8 tOF)
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

JSON::JSONElement *JSON::json_new(JSONValue *value)
{
  JSONElement *element = nullptr;

  ACE_NEW_RETURN(element, JSONElement(), 0);

  element->m_value = value;
  element->m_next = nullptr;

  return(element);
}

JSON::JSONArray *JSON::json_new(JSONElement *element)
{
  JSONArray *array = nullptr;

  ACE_NEW_RETURN(array, JSONArray(), 0);

  array->m_elements = element;

  return(array);
}

JSON::JSONMember *JSON::json_new(JSONValue *key, JSONValue *value)
{
  JSONMember *member = nullptr;

  ACE_NEW_RETURN(member, JSONMember(), 0);

  member->m_key = key;
  member->m_value = value;
  member->m_next = nullptr;

  return(member);
}

JSON::JSONObject *JSON::json_new(JSONMember *member)
{
  JSONObject *object = nullptr;

  ACE_NEW_RETURN(object, JSONObject(), 0);

  object->m_members = member;

  return(object);
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

  json_free(member->m_next);
  json_free(member->m_key);
  json_free(member->m_value);

  delete member;
}

void JSON::json_free(JSONObject *object)
{
  if(nullptr == object)
    return;

  json_free(object->m_members);
  delete object;
}

JSON::JSONElement *JSON::json_value_add_element(JSONElement *element, JSONValue *value)
{
  if(nullptr == element)
    return(json_new(value));

  JSONElement *e = nullptr;
  for(e = element; e->m_next != nullptr; e = e->m_next)
    ;

  e->m_next = json_new(value);

  return(element);
}

JSON::JSONMember *JSON::json_value_add_member(JSONMember *member, JSONValue *key, JSONValue *value)
{
  return(json_member_add_member(member, json_new(key, value)));
}

JSON::JSONMember *JSON::json_member_add_member(JSONMember *member, JSONMember *value)
{
  if(nullptr == member)
    return(member);

  JSONMember *m = nullptr;

  for(m = member; m->m_next != nullptr; m = m->m_next)
    ;
  m->m_next = value;

  return(member);
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
  JSONMember *m = nullptr;

  if((nullptr == value) ||
     (value->m_type != JSON::JSON_VALUE_TYPE_OBJECT) ||
     (nullptr == key))
    return(nullptr);

  for(m = value->m_ovalue->m_members; m != nullptr; m = m->m_next)
  {
    if((m->m_key->m_type == JSON::JSON_VALUE_TYPE_STRING) &&
       (!strcmp(m->m_key->m_svalue, key)))
    {
      return(m->m_value);
    }
  }

  return(nullptr);
}

#endif /*__JSON_CC__*/
