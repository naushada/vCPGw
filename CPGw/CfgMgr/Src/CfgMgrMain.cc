#ifndef __CFG_MGR_MAIN_CC__
#define __CFG_MGR_MAIN_CC__

#include "CfgMgr.h"
#include "CfgMgrMain.h"
#include "CfgCmdHandler.h"

#include "ace/Reactor.h"
#include "ace/Message_Block.h"

void CfgMgrMain::start(void)
{
  /*! Time Out Value of 1sec.*/
  ACE_Time_Value to(1,0);

  while(1)
  {
    if(-1 == ACE_Reactor::instance()->handle_events(to))
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l handle_events failed :%m\n")));
      break;
    }
  }
}

void CfgMgrMain::stop(void)
{
}

int main(int argc, char *argv[])
{
  /*argv[0] - Process Name,
   *argv[1] - IP Address - 127.0.0.1
   *argv[2] - ent,
   *argv[3] - inst,
   *argv[4] - nodeTag,
   *argv[5] - schema,
   **/

  if(argc > 4)
  {
    ACE_CString ip(argv[1]);
    ACE_UINT8 ent = ACE_OS::atoi(argv[2]);
    ACE_UINT8 inst = ACE_OS::atoi(argv[3]);
    ACE_CString proc(argv[4]);
    ACE_CString schema(argv[5]);

    CfgMgr cfgMgr(ACE_Thread_Manager::instance(), ip, ent, inst, proc);
    cfgMgr.start();
    cfgMgr.display();

    CfgCmdHandler cmdHandler(ACE_Thread_Manager::instance());
    cmdHandler.open();

    CfgMgrMain cfgMgrMain;

    /*Start the Reactor's main loop now.*/
    cfgMgrMain.start();
  }
  return(0);
}


#endif /*__CFG_MGR_MAIN_CC__*/
