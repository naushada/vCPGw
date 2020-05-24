#ifndef __SYSMGR_CC__
#define __SYSMGR_CC__

#include "SysMgr.h"
#include "ProcMgr.h"

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

void SysMgr::populateProcessTable(void)
{
  JSON root(jsonObj().value());

  JSON::JSONValue *taskTab = root["task-table"];
  int idx = 0;
  JSON::JSONValue *val = nullptr;
  JSON taskTabArray(taskTab);

  for(val = taskTabArray[idx]; val; val = taskTabArray[++idx])
  {
    JSON paramObj(val);
    JSON::JSONValue *paramVal = nullptr;
    paramVal = paramObj["task-name"];
    if(!paramVal)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid value of task-name\n")));
      break;
    }

    SysMgr::_taskTable_t tTable;
    ACE_CString key(paramVal->m_svalue);

    if(-1 == m_taskMap.bind(key, tTable))
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l binding to Hash Map Failed for key %s\n"), key.c_str()));
      break;
    }

    tTable.taskName(key.c_str());

    paramVal = paramObj["start-level"];
    tTable.startLevel(paramVal->m_ivalue);

    paramVal = paramObj["parent-entity"];
    tTable.parentTask(paramVal->m_svalue);

    paramVal = paramObj["container"];
    paramVal = paramObj["visible"];

    paramVal = paramObj["task-instance-min"];
    tTable.minInstance(paramVal->m_ivalue);

    paramVal = paramObj["task-instance-max"];
    tTable.maxInstance(paramVal->m_ivalue);

    paramVal = paramObj["stack-size"];

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l task-name %s start-level %u parent-task %s min-instance %u max-inatance %u\n"),
               tTable.taskName(), tTable.startLevel(),
               tTable.parentTask(), tTable.minInstance(), tTable.maxInstance()));
  }
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
  CommonIF::_cmMessage_t *cMsg = (CommonIF::_cmMessage_t *)mb.wr_ptr();
  _processSpawnReq_t *spawnReq = (_processSpawnReq_t *)cMsg->m_message;

  cMsg->m_dst.m_procId = get_self_procId();
  cMsg->m_dst.m_entId = CommonIF::ENT_PROCMGR;
  cMsg->m_dst.m_instId = CommonIF::INST1;

  cMsg->m_src.m_procId = get_self_procId();
  cMsg->m_src.m_entId = CommonIF::ENT_SYSMGR;
  cMsg->m_src.m_instId = CommonIF::INST1;

  cMsg->m_messageLen = sizeof(_processSpawnReq_t);
  cMsg->m_msgType = CommonIF::MSG_SYSMGR_PROCMGR_PROCESS_SPAWN_REQ;
  /*Populating Payload of Request.*/
  spawnReq->m_taskId = CommonIF::get_task_id(, instId);

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
