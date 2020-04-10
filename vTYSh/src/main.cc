#ifndef __MAIN_CC__
#define __MAIN_CC__

#include "readlineIF.h"
#include "vtysh.h"

int main()
{
  VtyshTask *vtyshTask = new VtyshTask(ACE_Thread_Manager::instance());
  ReadlineIF *rdIF = new ReadlineIF(vtyshTask);

  /*!Initialize the readline Interface.*/
  rdIF->init();

  /*! Make vtysh object as Active Object*/
  vtyshTask->open(0);

  /*! Start Accepting Command/Request */
  rdIF->start();

  delete vtyshTask;
  delete rdIF;

  return(0);
}


#endif /*__MAIN_CC__*/
