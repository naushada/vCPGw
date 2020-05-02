#ifndef __CFG_MGR_MAIN_CC__
#define __CFG_MGR_MAIN_CC__

#include "CfgCpGw.h"

int main(int argc, char *argv[])
{
  if(argc > 1 && argv[1])
  {
    ACE_CString schema(argv[1]);
    CfgMgr cfgInst(schema);
    cfgInst.start();
    cfgInst.display();
  }

  return(0);
}


#endif /*__CFG_MGR_MAIN_CC__*/
