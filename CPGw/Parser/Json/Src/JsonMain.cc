#ifndef __JSON_MAIN_CC__
#define __JSON_MAIN_CC__

#include "Json.h"
#include "JsonParser.hh"



int main(int argc, char *argv[])
{

  JSON *inst = JSON::instance();

  inst->start(nullptr);
  return(0);
}


#endif /*__JSON_MAIN_CC__*/
