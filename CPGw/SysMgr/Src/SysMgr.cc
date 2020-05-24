#ifndef __SYSMGR_CC__
#define __SYSMGR_CC__

#include "SysMgr.h"

SysMgr::SysMgr(ACE_Thread_Manager *thrMgr, ACE_CString ip, ACE_UINT8 entId, ACE_UINT8 instId, ACE_CString nodeTag) :
  UniIPC(thrMgr, ip, entId, instId, nodeTag)
{
  ACE_Reactor::instance()->register_handler(this,
                                            ACE_Event_Handler::READ_MASK |
                                            ACE_Event_Handler::SIGNAL_MASK);
  jsonObj(JSON::instance());
  jsonObj().start();
  populateProcessTable();
  jsonObj().stop();
  delete m_jsonObj;
  m_jsonObj = nullptr;

}

SysMgr::~SysMgr()
{
}


ACE_UINT32 SysMgr::process_signal(int signum)
{
  return(0);
}


ACE_UINT32 SysMgr::handle_ipc(ACE_Message_Block *mb)
{
  putq(mb);
  return(0);
}

ACE_HANDLE SysMgr::get_handle(void) const
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l SysMgr::get_handle\n")));
  return(const_cast<SysMgr *>(this)->handle());
}

int SysMgr::svc(void)
{
  ACE_Message_Block *mb;

  for(;getq(mb) != -1;)
  {
    if(mb->msg_type() == ACE_Message_Block::MB_HANGUP)
    {
      mb->release();
      break;
    }

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l dequeue equest\n")));
    processIPCMessage(*mb);
    /*reclaim the heap memory now. allocated by the sender*/
    mb->release();
  }

  return(0);
}

int SysMgr::processIPCMessage(ACE_Message_Block &mb)
{
  ACE_Byte *in = nullptr;
  ACE_UINT32 len = 0;
  int rspLen = -1;

  /*Process The Request.*/
  CommonIF::_cmMessage_t *cMsg = (CommonIF::_cmMessage_t *)mb.rd_ptr();
  in = (ACE_Byte *)cMsg;
  len = mb.length();

  ACE_UINT32 msgType = cMsg->m_msgType;

  switch(msgType)
  {
  case CommonIF::MSG_PROCMGR_SYSMGR_PROCESS_SPAWN_RSP:
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l SPAWN RSP is received\n")));

    ACE_Message_Block *mbRsp = nullptr;
    ACE_NEW_NORETURN(mbRsp, ACE_Message_Block(CommonIF::SIZE_1KB));

    processSpawnRsp(ACE_Byte *in, ACE_UINT32 len, ACE_Message_Block &mbRsp);
    /*Reclaim the Heap Memory Now.*/
    mbRsp->release();
    break;

  case CommonIF::MSG_PROCMGR_SYSMGR_PROCESS_DIED_IND:
    break;
  }

  return(0);
}

void SysMgr::buildAndSendSpawnReq(ACE_CString &entName, ACE_UINT8 instId, ACE_Message_Block &mb)
{
}

int SysMgr::start(void)
{
  ACE_Time_Value to(1,0);

  for(;;)
  {
    ACE_Reactor::instance()->handle_events(to);
  }

  return(0);
}












#endif /*__SYSMGR_CC__*/
