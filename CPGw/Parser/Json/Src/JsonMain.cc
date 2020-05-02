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

  if(inst->start(argv[1]))
    return(1);

  inst->display(inst->value());


  JSON root(inst->value());

  JSON::JSONValue *r = root["cp-gateway"];
  JSON obr(r);
  JSON::JSONValue *rc1 = obr["instances"];
  JSON obrc1(rc1);
  JSON::JSONValue *rc11 = obrc1["dhcp-server"];
  JSON obrc11(rc11);
  //JSON::JSONValue *jValue = inst->json_value_at_key(ob[3], "label");
  JSON::JSONValue *jValue = obrc11[0];

  for(int idx = 0;  jValue; jValue = obrc11[idx])
  {

    if(jValue->m_type == JSON::JSON_VALUE_TYPE_STRING)
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l dhcp-server instances %s\n"), jValue->m_svalue));
    }

    idx++;
  }

  inst->stop();
  delete inst;
  return(0);
}


#endif /*__JSON_MAIN_CC__*/
