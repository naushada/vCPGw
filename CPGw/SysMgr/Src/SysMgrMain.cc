#ifndef __SYSMGR_MAIN_CC__
#define __SYSMGR_MAIN_CC__

#include "SysMgr.h"

int main(int agrc, char *argv[])
{

  ACE_CString ip(argv[1]);
  ACE_UINT8 entId = CommonIF::ENT_SYSMGR;
  ACE_UINT8 instId = ACE_OS::atoi(argv[2]);
  ACE_CString nodeTag(argv[3]);

  SysMgr sysMgr(ACE_Thread_Manager::instance(), ip, entId, instId, nodeTag);
  sysMgr.start();

  return(0);
}

#endif /*__SYSMGR_MAIN_CC__*/
