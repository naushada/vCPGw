#ifndef __JSON_MAIN_CC__
#define __JSON_MAIN_CC__

#include <iostream>
#include "Json.h"
#include "JsonParser.hh"

/*
 * @brief
 * @param
 * @param
 *
 * @return
 * */
int main(int argc, char *argv[])
{

  JSON *inst = JSON::instance();

  inst->start(argv[1]);

  inst->display(inst->value());
#if 0
  JSON root(inst->value());

  JSON::JSONValue *menu = root["menu"];
  JSON objMenu(menu);
  JSON::JSONValue *items = objMenu["items"];
  JSON ob(items);
  //JSON::JSONValue *jValue = inst->json_value_at_key(ob[3], "label");
  JSON::JSONValue *jValue = objMenu["header"];

  if(jValue && jValue->m_type == JSON::JSON_VALUE_TYPE_STRING)
  {
    std::cout << "value of jValue is " << jValue->m_svalue << std::endl;
  }
#endif

  inst->stop();
  JSON::destroy();
  delete inst;
  return(0);
}


#endif /*__JSON_MAIN_CC__*/
