#ifndef __CFG_MGR_MAIN_CC__
#define __CFG_MGR_MAIN_CC__

#include "CfgCpGw.h"
#include "CfgMgrMain.h"
#include "CfgCmdHandler.h"

#include "ace/Reactor.h"

CfgMgrMain::CfgMgrMain(ACE_CString ipAddr, ACE_UINT8 ent, ACE_UINT8 inst, ACE_CString nodeTag) :
  UniIPC(ipAddr, ent, inst, nodeTag)
{
  ACE_Reactor::instance()->register_handler(this,
                                            ACE_Event_Handler::READ_MASK);
}

CfgMgrMain::~CfgMgrMain()
{
}

ACE_HANDLE CfgMgrMain::get_handle(void) const
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l CfgMgrMain::get_handle\n")));
  return(const_cast<CfgMgrMain *>(this)->handle());
}

ACE_UINT32 CfgMgrMain::handle_ipc(ACE_UINT8 *req, ACE_UINT32 reqLen)
{
  return(0);
}

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
    ACE_CString schema(argv[5]);
    CfgMgr cfgInst(schema);
    cfgInst.start();
    cfgInst.display();
    cfgInst.stop();

    CfgCmdHandler cmdHandler(ACE_Thread_Manager::instance());
    cmdHandler.open();

    ACE_CString ip(argv[1]);
    ACE_UINT8 ent = ACE_OS::atoi(argv[2]);
    ACE_UINT8 inst = ACE_OS::atoi(argv[3]);
    ACE_CString proc(argv[4]);

    CfgMgrMain cfgMgrMain(ip, ent, inst, proc);

    cfgMgrMain.start();
  }
  return(0);
}


#endif /*__CFG_MGR_MAIN_CC__*/
