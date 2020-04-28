#ifndef __JSON_MAIN_CC__
#define __JSON_MAIN_CC__

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
  return(0);
}


#endif /*__JSON_MAIN_CC__*/
