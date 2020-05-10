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

  ACE_NEW_NORETURN(m_childHandler, ChildHandler(thrMgr, this));

}

ProcMgr::~ProcMgr()
{
  delete m_childHandler;
  m_childHandler = nullptr;
}

ACE_UINT32 ProcMgr::handle_ipc(ACE_Message_Block *mb)
{
  putq(mb);
  return(0);
}

ACE_HANDLE ProcMgr::get_handle(void) const
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l ProcMgr::get_handle\n")));
  return(const_cast<ProcMgr *>(this)->handle());
}

ACE_UINT32 ProcMgr::process_signal(int signum)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l ProcMgr::process_signal\n")));
  return(0);
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
    ACE_OS::exit(1);

  case -1:
    /*Error case.*/
    break;

  default:
    /*parent Process.*/
    pPid = ACE_OS::getppid();
    buildSpawnRsp(in, cPid, pPid, mb);
    break;
  }

  return(0);
}

int ProcMgr::buildSpawnRsp(ACE_Byte *in, pid_t cPid, pid_t pPid, ACE_Message_Block &mb)
{
  if(!in)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l pointer to in is nullptr\n")));
    return(-1);
  }

  CommonIF::_cmMessage_t *req = (CommonIF::_cmMessage_t *)in;
  _processSpawnReq *spawnReq = (_processSpawnReq *)req->m_message;

  CommonIF::_cmMessage_t *rsp = (CommonIF::_cmMessage_t *)mb.wr_ptr();

  /*Flip the Header now.*/
  ACE_OS::memcpy((void *)&rsp->m_dst, (const ACE_TCHAR *)&req->m_src,
                 sizeof(CommonIF::_cmHeader_t));
  ACE_OS::memcpy((void *)&rsp->m_src, (const ACE_TCHAR *)&req->m_dst,
                 sizeof(CommonIF::_cmHeader_t));

  rsp->m_msgType = CommonIF::MSG_PROCMGR_SYSMGR_PROCESS_SPAWN_RSP;
  rsp->m_messageLen = 2;
  _processSpawnRsp_t *spawnRsp = (_processSpawnRsp_t *)rsp->m_message;
  spawnRsp->m_cPid = cPid;
  spawnRsp->m_pPid = pPid;

  /*Update the length now.*/
  mb.wr_ptr(sizeof(CommonIF::_cmMessage_t) + sizeof(_processSpawnRsp_t));
  return(0);
}

int ProcMgr::start(void)
{
  ACE_Time_Value to(1,0);

  for(;;)
  {
    ACE_Reactor::instance()->handle_events(to);
  }
}
#endif /*__PROCMGR_CC__*/
