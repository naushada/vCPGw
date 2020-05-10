#ifndef __PROCMGR_MAIN_CC__
#define __PROCMGR_MAIN_CC__

#include "ProcMgr.h"


int main(int argc, char *argv[])
{

  ACE_CString ip(argv[1]);
  ACE_UINT8 entId = ACE_OS::atoi(argv[2]);
  ACE_UINT8 instId = ACE_OS::atoi(argv[3]);
  ACE_CString nodeTag(argv[4]);

  ProcMgr procMgr(ACE_Thread_Manager::instance(), ip, entId, instId, nodeTag);

  procMgr.start();
  return(0);
}









#endif /*__PROCMGR_MAIN_CC__*/
