#ifndef __JSON_H__
#define __JSON_H__

#include "ace/Basic_Types.h"
#include "ace/SString.h"
#include "ace/Log_Msg.h"

class JSON
{
public:
  static const ACE_UINT8 JSON_VALUE_TYPE_INTEGER = 1;
  static const ACE_UINT8 JSON_VALUE_TYPE_DOUBLE = 2;
  static const ACE_UINT8 JSON_VALUE_TYPE_STRING = 3;
  static const ACE_UINT8 JSON_VALUE_TYPE_OBJECT = 4;
  static const ACE_UINT8 JSON_VALUE_TYPE_ARRAY = 5;
  static const ACE_UINT8 JSON_VALUE_TYPE_TRUE = 6;
  static const ACE_UINT8 JSON_VALUE_TYPE_FALSE = 7;
  static const ACE_UINT8 JSON_VALUE_TYPE_NUM = 8;
  static const ACE_UINT8 JSON_VALUE_TYPE_NULL = 9;

  static const ACE_UINT8 TRUE = 1;
  static const ACE_UINT8 FALSE = 0;

  struct JSONMember ;
  struct JSONArray ;
  struct JSONElement ;
  struct JSONValue ;

  struct JSONMembers
  {
    JSONMember *m_member;
    JSONMembers *m_next;
  };

  struct JSONObject
  {
    JSONMembers *m_members;
  };

  struct JSONMember
  {
    JSONValue *m_key;
    JSONValue *m_value;
  };

  struct JSONArray
  {
    JSONElement *m_elements;
  };

  struct JSONElement
  {
    JSONValue *m_value;
    JSONElement *m_next;
  };

  struct JSONValue
  {
    JSONValue()
    {
      m_ivalue = 0;
    }

    ~JSONValue()
    {
    }

    ACE_UINT8 m_type;
    union
    {
      int m_ivalue;
      double m_dvalue;
      char *m_svalue;
      JSONObject *m_ovalue;
      JSONArray *m_avalue;
    };
  };

  ~JSON();
  JSONValue *json_new_value(void);
  JSONValue *json_new_value_int(int i);
  JSONValue *json_new_value_double(double d);
  JSONValue *json_new_value_str(char *str);
  JSONValue *json_new_value(char *str, int size);
  JSONValue *json_new_value_object(JSONObject *object);
  JSONValue *json_new_value_array(JSONArray *array);
  JSONValue *json_new_value_bool(const ACE_UINT8 tOrF);
  JSONValue *json_new_value(std::nullptr_t nullp);

  JSONElement *json_new_element(JSONValue *value);
  JSONArray *json_new_array(JSONElement *element);
  JSONMember *json_new_member(JSONValue *key, JSONValue *value);

  JSONObject *json_new_object(JSONMembers *members);
  JSONMembers *json_add_member_in_members(JSONMembers *members, JSONMember *member);
  JSONMembers *json_new_members(JSONMember *member);

  JSONObject *json_new_object(void);
  JSONArray *json_new_array(void);

  void json_free(JSONValue *value);
  void json_free(JSONElement *element);
  void json_free(JSONArray *array);
  void json_free(JSONMember *member);
  void json_free(JSONObject *object);

  JSONElement *json_value_add_element(JSONElement *element, JSONValue *value);
  JSONMember *json_value_add_member(JSONMember *member, JSONValue *key, JSONValue *value);

  JSONValue *json_value_at_index(JSONValue *value, int index);
  JSONValue *json_value_at_key(JSONValue *value, char *key);

  /*Array operator overloading.*/
  JSONValue *operator [] (int index);
  JSONValue *value(void);
  void value(JSONValue *value);

  static JSON *instance(void);
  static JSON *get_instance(void);
  int start(const ACE_TCHAR *fname);
  int stop(void);

private:
  static JSON *m_instance;
  JSONValue *m_value;
  JSON();

};

#endif /*__JSON_H__*/
