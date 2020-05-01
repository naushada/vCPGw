#ifndef __JSON_CC__
#define __JSON_CC__

#include <iostream>
#include "Json.h"
#include "JsonParser.hh"
#include "JsonLexer.hh"

#include "ace/Log_Msg.h"

JSON *JSON::m_instance = nullptr;

JSON *JSON::instance(void)
{
  if(nullptr == m_instance)
  {
    ACE_NEW_NORETURN(m_instance, JSON());
  }

  return(m_instance);
}

void JSON::destroy(void)
{
  delete m_instance;
  m_instance = nullptr;
}

JSON *JSON::get_instance(void)
{
  return(m_instance);
}

JSON::JSON(JSONValue *value)
{
  m_value = value;
}

JSON::JSON()
{
  ACE_NEW_NORETURN(m_value, JSONValue());
}

JSON::~JSON()
{
  /*reclaim the heap memory now.*/
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

int JSON::stop(void)
{
  json_free(m_value);
  /*don't free m_value itself.*/
  return(0);
}

int JSON::start(const ACE_TCHAR *fname)
{
  int ret = -1;

  FILE *in = nullptr;

  yyscan_t scanner;

  if(fname)
  {
    in = ACE_OS::fopen(fname, "r");
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
    ACE_OS::fclose(in);
    in = nullptr;
  }

  //JSONValue *jValue = json_value_at_key(json_value_at_index(json_value_at_key(json_value_at_key(this->value(), "menu"), "items"), 1), "id");
  //JSONValue *arr = json_value_at_key(json_value_at_key(this->value(), "menu"), "items");
#if 0
  JSONValue *menu = (*this)["menu"];
  JSON objMenu(menu);
  JSONValue *items = objMenu["items"];
  //JSONValue *arr = json_value_at_key(json_value_at_key(this->value(), "menu"), "items");
  JSON ob(items);
  JSONValue *jValue = json_value_at_key(ob[3], "label");

  if(jValue && jValue->m_type == JSON_VALUE_TYPE_STRING)
  {
    std::cout << "value of id is " << jValue->m_svalue << std::endl;
  }
#endif
  return(ret);
}

JSON::JSONValue *JSON::json_new_value(void)
{
  JSONValue *value = nullptr;

  ACE_NEW_RETURN(value, JSONValue(), nullptr);
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
  if(nullptr == value)
    return(nullptr);

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

  if(nullptr == value)
    return(nullptr);

  value->m_type = JSON::JSON_VALUE_TYPE_NULL;
  value->m_nvalue = ACE_OS::strdup("null");

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
    /*use free and not delete because memory was allocated by
     * strdup which usages malloc.*/
    ACE_OS::free((void *)value->m_svalue);
    break;

  case JSON::JSON_VALUE_TYPE_NULL:
    /*use free and not delete because memory was allocated
     *by strdup which usages malloc.*/
    ACE_OS::free((void *)value->m_nvalue);
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
  json_free(element->m_value);
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

void JSON::json_free(JSONMembers *members)
{
  if(nullptr == members)
    return;

  json_free(members->m_next);
  json_free(members->m_member);
  delete members;

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
    return(json_new_element(value));

  JSONElement *e = nullptr;
  for(e = element; e->m_next != nullptr; e = e->m_next)
    ;

  e->m_next = json_new_element(value);

  return(element);
}

JSON::JSONValue *JSON::operator[](int index)
{
  return(json_value_at_index(value(), index));
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

JSON::JSONValue *JSON::operator[](const char *key)
{
  return(json_value_at_key(m_value, key));
}

JSON::JSONValue *JSON::json_value_at_key(JSONValue *value, const char *key)
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

void JSON::display(JSONValue *value)
{
  if(nullptr == value)
    return;

  switch(value->m_type)
  {
  case JSON_VALUE_TYPE_STRING:
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l %s \n"), value->m_svalue));
    break;
  case JSON_VALUE_TYPE_OBJECT:
    display(value->m_ovalue);
    break;
  case JSON_VALUE_TYPE_ARRAY:
    display(value->m_avalue);
    break;
  default:
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Type not supported\n")));

  }
}

void JSON::display(JSONMembers *members)
{
  if(nullptr == members)
    return;

  if(members->m_member)
  {
    display(members->m_member->m_key);
    display(members->m_member->m_value);
    display(members->m_next);
  }

}

void JSON::display(JSONObject *object)
{
  if(nullptr == object)
    return;

  display(object->m_members);
}

void JSON::display(JSONElement *element)
{
  if(nullptr == element)
    return;

  display(element->m_value);
  display(element->m_next);
}

void JSON::display(JSONArray *array)
{
  if(nullptr == array)
    return;

  display(array->m_elements);
}

#endif /*__JSON_CC__*/
