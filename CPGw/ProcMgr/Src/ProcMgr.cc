#ifndef __PROCMGR_CC__
#define __PROCMGR_CC__

#include "ProcMgr.h"
#include "CommonIF.h"

ProcMgr::ProcMgr(ACE_Thread_Manager *thrMgr, ACE_CString ip, ACE_UINT8 entId, ACE_UINT8 instId, ACE_CString nodeTag) :
  UniIPC(thrMgr, ip, entId, instId, nodeTag)
{
  ACE_Reactor::instance()->register_handler(this,
                                            ACE_Event_Handler::READ_MASK);

  ACE_Reactor::instance()->register_handler(SIGCHLD,
                                            this);

  /*Build and send processSpawnReq to itself to spawn SYSMGR.*/

  ACE_Message_Block *mbReq = nullptr;
  ACE_NEW_NORETURN(mbReq, ACE_Message_Block(CommonIF::SIZE_1KB));

  buildAndSendSysMgrSpawnReq(*mbReq);
  /*Reclaim the memory Now.*/
  mbReq->release();
}

ProcMgr::~ProcMgr()
{
}

void ProcMgr::buildAndSendSysMgrChildDiedInd(pid_t cPid, ACE_UINT32 reason)
{
  ACE_Message_Block *mb = nullptr;
  ACE_NEW_NORETURN(mb, ACE_Message_Block(CommonIF::SIZE_1KB));

  CommonIF::_cmMessage_t *cMsg = (CommonIF::_cmMessage_t *)mb->wr_ptr();
  _processDiedInd_t *diedInd = (_processDiedInd_t *)cMsg->m_message;

  cMsg->m_dst.m_procId = get_self_procId();
  cMsg->m_dst.m_entId = CommonIF::ENT_SYSMGR;
  cMsg->m_dst.m_instId = CommonIF::INST1;

  cMsg->m_src.m_procId = get_self_procId();
  cMsg->m_src.m_entId = facility();
  cMsg->m_src.m_instId = instance();

  cMsg->m_msgType = CommonIF::MSG_PROCMGR_SYSMGR_PROCESS_DIED_IND;
  cMsg->m_messageLen = sizeof(_processDiedInd_t);

  /*Populating Payload of Request.*/
  diedInd->m_cPid = cPid;

  /*Update the request length now.*/
  mb->wr_ptr(sizeof(CommonIF::_cmMessage_t) + sizeof(_processDiedInd_t));

  send_ipc((ACE_Byte *)mb->rd_ptr(), (ACE_UINT32)mb->length());
  /*Reclaim the memory.*/
  mb->release();
}


void ProcMgr::buildAndSendSysMgrSpawnReq(ACE_Message_Block &mb)
{
  CommonIF::_cmMessage_t *cMsg = (CommonIF::_cmMessage_t *)mb.wr_ptr();
  _processSpawnReq_t *spawnReq = (_processSpawnReq_t *)cMsg->m_message;

  cMsg->m_dst.m_procId = get_self_procId();
  cMsg->m_dst.m_entId = CommonIF::ENT_PROCMGR;
  cMsg->m_dst.m_instId = CommonIF::INST1;

  cMsg->m_src = cMsg->m_dst;

  cMsg->m_messageLen = sizeof(_processSpawnReq_t);
  cMsg->m_msgType = CommonIF::MSG_SYSMGR_PROCMGR_PROCESS_SPAWN_REQ;
  /*Populating Payload of Request.*/
  spawnReq->m_taskId = CommonIF::get_task_id(CommonIF::ENT_SYSMGR, CommonIF::INST1);

  const ACE_TCHAR *entName = "SYSMGR";
  ACE_OS::memset((void *)spawnReq->m_entName, 0, sizeof(spawnReq->m_entName));
  ACE_OS::strncpy((ACE_TCHAR *)spawnReq->m_entName, entName, ACE_OS::strlen(entName) + 1);

  spawnReq->m_restartCnt = 0;
  ACE_OS::strncpy((ACE_TCHAR *)spawnReq->m_nodeTag, nodeTag().c_str(), nodeTag().length());

  const ACE_TCHAR *ip = "127.0.0.1";
  ACE_OS::memset((void *)spawnReq->m_ip, 0, sizeof(spawnReq->m_ip));
  ACE_OS::strncpy((ACE_TCHAR *)spawnReq->m_ip, ip, ACE_OS::strlen(ip) + 1);

  /*Update the request length now.*/
  mb.wr_ptr(sizeof(CommonIF::_cmMessage_t) + sizeof(_processSpawnReq_t));

  send_ipc((ACE_Byte *)mb.rd_ptr(), (ACE_UINT32)mb.length());
}

ACE_UINT32 ProcMgr::process_signal(int signum)
{
  pid_t childPid;
  int childStatus;

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l The signum is %u\n"), signum));

  for(;;)
  {
    childStatus = 0;

    childPid = ACE_OS::waitpid(-1, &childStatus, WNOHANG);

    if(childPid <= 0)
    {
      /* Control comes here when died child list is exhausted, return the
       * control to Reactor main loop.*/
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l childPid is less than zero\n")));
      break;
    }

    if(WIFEXITED(childStatus))
    {
      /*Child is terminated.*/
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l SIGCHLD exited with childPid %u\n"), childPid));
      /*build and send notify to SYSMGR*/
      buildAndSendSysMgrChildDiedInd(childPid, 1);
    }
    else if(WIFSIGNALED(childStatus))
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l SIGCHLD Terminated childPid %u\n"), childPid));
      buildAndSendSysMgrChildDiedInd(childPid, 1);

    }
    else if(WIFSTOPPED(childStatus))
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l SIGCHLD Stopped childPid %u\n"), childPid));
      buildAndSendSysMgrChildDiedInd(childPid, 1);
    }
    else if(WIFCONTINUED(childStatus))
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l SIGCHLD Continued childPid %u\n"), childPid));
    }
    else
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l SIGCHLD Unknown childPid %u\n"), childPid));
    }
  }

  return(0);
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
  in = (ACE_Byte *)cMsg;
  len = mb.length();

  ACE_UINT32 msgType = cMsg->m_msgType;

  switch(msgType)
  {
  case CommonIF::MSG_SYSMGR_PROCMGR_PROCESS_SPAWN_REQ:
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l SPAWN REQ is received\n")));

    ACE_Message_Block *mbRsp = nullptr;
    ACE_NEW_NORETURN(mbRsp, ACE_Message_Block(CommonIF::SIZE_1KB));

    processSpawnReq(in, len, *mbRsp);
    mbRsp->release();
    break;
  }

  return(0);
}

int ProcMgr::processSpawnReq(ACE_Byte *in, ACE_UINT32 len, ACE_Message_Block &mb)
{
  CommonIF::_cmMessage_t *req = (CommonIF::_cmMessage_t *)in;
  pid_t cPid = 0;
  pid_t pPid = 0;
  ACE_UINT32 taskId = 0;
  ACE_UINT8 instId = 0;
  ACE_UINT8 entId = 0;
  int ret = 0;

  _processSpawnReq *pReq = (_processSpawnReq *)req->m_message;

  taskId = pReq->m_taskId;
  instId = CommonIF::get_inst_id(taskId);
  entId = CommonIF::get_ent_id(taskId);

  switch((cPid = ACE_OS::fork()))
  {
  case 0:
    /*child process.*/
    ACE_TCHAR instStr[16];
    ACE_OS::itoa((int)instId, instStr, 10);

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l Child Process entName %s instStr %s\n"), pReq->m_entName, instStr));

    ret = execlp((const ACE_TCHAR *)pReq->m_entName, (const ACE_TCHAR *)pReq->m_entName, (const ACE_TCHAR *)pReq->m_ip,
                 (const ACE_TCHAR *)instStr, (const ACE_TCHAR *)pReq->m_nodeTag, (ACE_TCHAR *)0);
    if(ret < 0)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l execlp failed for %s %m\n"), pReq->m_entName));
      /*overriding of address space is failed with new process.*/
      ACE_OS::exit(1);
    }
    break;

  case -1:
    /*Error case.*/
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l fork system call failed\n")));
    break;

  default:
    /*parent Process.*/
    pPid = ACE_OS::getppid();
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l Parent Process cPid %u pPid %u\n"), cPid, pPid));
    buildSpawnRsp(in, cPid, pPid, mb);
    send_ipc((ACE_Byte *)mb.rd_ptr(), (ACE_UINT32)mb.length());
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
  _processSpawnReq_t *spawnReq = (_processSpawnReq_t *)req->m_message;

  CommonIF::_cmMessage_t *rsp = (CommonIF::_cmMessage_t *)mb.wr_ptr();

  ACE_OS::memcpy((void *)&rsp->m_src, (const ACE_TCHAR *)&req->m_dst,
                 sizeof(CommonIF::_cmHeader_t));

  rsp->m_src.m_procId = get_self_procId();
  rsp->m_src.m_entId = CommonIF::ENT_PROCMGR;
  rsp->m_src.m_instId = CommonIF::INST1;

  rsp->m_dst.m_procId = get_self_procId();
  rsp->m_dst.m_entId = CommonIF::ENT_SYSMGR;
  rsp->m_dst.m_instId = CommonIF::INST1;

  rsp->m_msgType = CommonIF::MSG_PROCMGR_SYSMGR_PROCESS_SPAWN_RSP;
  rsp->m_messageLen = sizeof(_processSpawnRsp_t);
  _processSpawnRsp_t *spawnRsp = (_processSpawnRsp_t *)rsp->m_message;
  spawnRsp->m_cPid = cPid;
  spawnRsp->m_pPid = pPid;
  spawnRsp->m_taskId = spawnReq->m_taskId;

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
