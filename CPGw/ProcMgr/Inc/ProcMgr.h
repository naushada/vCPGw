#ifndef __PROCMGR_H__
#define __PROCMGR_H__

#include "UniIPC.h"
#include "CommonIF.h"

#include "ace/Basic_Types.h"
#include "ace/Message_Block.h"
#include "ace/SString.h"
#include "ace/Hash_Map_Manager.h"
#include "ace/Null_Mutex.h"
#include "ace/Task.h"
#include "ace/Log_Msg.h"
#include "ace/Reactor.h"
#include "ace/Process.h"

typedef struct _processSpawnReq
{
  ACE_UINT32 m_taskId;
  ACE_Byte m_entName[255];
  ACE_UINT8 m_restartCnt;
  ACE_Byte m_nodeTag[256];
  ACE_Byte m_ip[32];

}__attribute__((packed))_processSpawnReq_t;

typedef struct _processSpawnRsp
{
  pid_t m_cPid;
  pid_t m_pPid;
  ACE_UINT32 m_taskId;

}__attribute__((packed))_processSpawnRsp_t;

typedef struct _processDiedInd
{
  /*Child PID.*/
  pid_t m_cPid;
  /*Parent PID.*/
  pid_t m_pPid;
  /*A TaskId composed of <magic> + ent + instId.*/
  ACE_UINT32 m_tId;

  void cPid(pid_t c)
  {
    m_cPid = c;
  }

  pid_t cPid(void)
  {
    return(m_cPid);
  }

  void pPid(pid_t p)
  {
    m_pPid = p;
  }

  pid_t pPid(void)
  {
    return(m_pPid);
  }

  void tId(ACE_UINT32 t)
  {
    m_tId = t;
  }

  ACE_UINT32 tId(void)
  {
    return(m_tId);
  }

  _processDiedInd()
  {
    m_cPid = 0;
    m_pPid = 0;
    m_tId = 0;
  }

  ~_processDiedInd()
  {
  }
}__attribute__((packed))_processDiedInd_t;

typedef ACE_Hash_Map_Manager<ACE_CString, ACE_UINT8, ACE_Null_Mutex>EntNameToEntIdMap_t;
typedef ACE_Hash_Map_Manager<ACE_CString, ACE_UINT8, ACE_Null_Mutex>::iterator EntNameToEntIdMap_Iter_t;

typedef ACE_Hash_Map_Manager<pid_t, _processDiedInd_t, ACE_Null_Mutex> ChildPidMap_t;
typedef ACE_Hash_Map_Manager<pid_t, _processDiedInd_t, ACE_Null_Mutex>::iterator ChildPidMapIter_t;

class ProcessHandler : public ACE_Process
{
public:
  ProcessHandler() = default;
  virtual ~ProcessHandler() = default;
  ACE_Process_Options m_processOpt;

private:

};

class ProcMgr : public UniIPC
{
private:
  ChildPidMap_t m_childPidMap;

public:

  ProcMgr(ACE_Thread_Manager *thrMgr, ACE_CString ip, ACE_UINT8 entId, ACE_UINT8 instId,
          ACE_CString nodeTag);

  ProcMgr() = default;
  virtual ~ProcMgr();

  int start(void);
  int svc(void);
  ACE_HANDLE get_handle(void) const;
  ACE_UINT32 handle_ipc(ACE_Message_Block *mb);
  ACE_UINT32 process_signal(int signum);
  int processIPCMessage(ACE_Message_Block &mb);
  int processSpawnReq(ACE_Byte *in, ACE_UINT32 len, ACE_Message_Block &mb);
  int buildSpawnRsp(ACE_Byte *in, pid_t cPid, pid_t pPid, ACE_Message_Block &mb);
  void buildAndSendSysMgrSpawnReq(ACE_Message_Block &mb);
  void buildAndSendSysMgrChildDiedInd(pid_t cPid, ACE_UINT32 reason);
};


#endif /*__PROCMGR_H__*/
