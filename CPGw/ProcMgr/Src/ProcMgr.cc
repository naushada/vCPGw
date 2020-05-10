#ifndef __PROCMGR_CC__
#define __PROCMGR_CC__

#include "ProcMgr.h"
#include "CommonIF.h"

ProcMgr::ProcMgr(ACE_Thread_Manager *thrMgr, ACE_CString ip, ACE_UINT8 entId, ACE_UINT8 instId, ACE_CString nodeTag) :
  UniIPC(thrMgr, ip, entId, instId, nodeTag)
{
  int idx = 0;
  for(;CommonIF::m_entTable[idx].m_entName; idx++)
  {
    m_entNameToIdMap.bind(ACE_CString(CommonIF::m_entTable[idx].m_entName),
                          CommonIF::m_entTable[idx].m_entId);
  }

  ACE_Reactor::instance()->register_handler(this,
                                            ACE_Event_Handler::READ_MASK);
}

ProcMgr::~ProcMgr()
{
}

ACE_UINT32 ProcMgr::handle_ipc(ACE_Message_Block *mb)
{
  putq(mb);
  return(0);
}

ACE_HANDLE ProcMgr::get_handle(void) const
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l CfgMgr::get_handle\n")));
  return(const_cast<ProcMgr *>(this)->handle());
}

int ProcMgr::svc(void)
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

int ProcMgr::processIPCMessage(ACE_Message_Block &mb)
{
  ACE_Byte *in = nullptr;
  ACE_UINT32 len = 0;
  int rspLen = -1;

  /*Process The Request.*/
  CommonIF::_cmMessage_t *cMsg = (CommonIF::_cmMessage_t *)mb.rd_ptr();
  len = mb.length();

  ACE_UINT32 msgType = *((ACE_UINT32 *)cMsg->m_message);

  switch(msgType)
  {
  case CommonIF::MSG_SYSMGR_PROCMGR_PROCESS_SPAWN_REQ:
    ACE_Message_Block *mbRsp = nullptr;
    ACE_NEW_NORETURN(mbRsp, ACE_Message_Block(CommonIF::SIZE_1KB));
    processSpawnReq(in, len, *mbRsp);
    send_ipc((ACE_Byte *)mbRsp->rd_ptr(), (ACE_UINT32)mbRsp->length());
    mbRsp->release();
    break;
  }

  return(0);
}

int ProcMgr::processSpawnReq(ACE_Byte *in, ACE_UINT32 len, ACE_Message_Block &mb)
{
  CommonIF::_cmMessage_t *req = (CommonIF::_cmMessage_t *)in;
  pid_t cPid;
  pid_t pPid;

  _processSpawnReq *pReq = (_processSpawnReq *)req->m_message;
  ACE_UINT8 entId = 0;
  ACE_CString key((const ACE_TCHAR *)pReq->m_entName);

  if(m_entNameToIdMap.find(key, entId) == -1)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l key %s is not found in the map\n"), key.c_str()));
    return(0);
  }

  switch((cPid = ACE_OS::fork()))
  {
  case 0:
    /*child process.*/
    //ACE_TCHAR instIdStr[8];
    //ACE_OS::itoa((int)pReq->m_instId, instIdStr, 10);
    //ACE_Byte *argv[] = {pReq->m_ip, entId, instIdStr, pReq->m_nodeTag, nullptr};
    ACE_OS::execlp((const char *)pReq->m_entName, (const char *)pReq->m_ip, entId, pReq->m_instId, (const char *)pReq->m_nodeTag);
    break;

  default:
    /*parent Process.*/
    pPid = ACE_OS::getppid();
    buildSpawnRsp(in, cPid, pPid, mb);
    break;
  }
}

int ProcMgr::buildSpawnRsp(ACE_Byte *in, pid_t cPid, pid_t pPid, ACE_Message_Block &mb)
{
  return(0);
}

#endif /*__PROCMGR_CC__*/
