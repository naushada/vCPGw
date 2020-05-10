#ifndef __PROCMGR_H__
#define __PROCMGR_H__

#include "UniIPC.h"

#include "ace/Basic_Types.h"
#include "ace/Message_Block.h"
#include "ace/SString.h"
#include "ace/Hash_Map_Manager.h"
#include "ace/Null_Mutex.h"
#include "ace/Task.h"
#include "ace/Log_Msg.h"
#include "ace/Reactor.h"

typedef struct _processSpawnReq
{
  ACE_Byte m_entName[256];
  ACE_UINT8 m_instId;
  ACE_UINT8 m_restartCnt;
  ACE_Byte m_nodeTag[256];
  ACE_Byte m_ip[32];

}_processSpawnReq_t;

typedef struct _ProcessSpawnRsp
{
  ACE_UINT8 m_cPid;
  ACE_UINT8 m_pPid;

}_ProcessSpawnRsp_t;

typedef ACE_Hash_Map_Manager<ACE_CString, ACE_UINT8, ACE_Null_Mutex>EntNameToEntIdMap_t;
typedef ACE_Hash_Map_Manager<ACE_CString, ACE_UINT8, ACE_Null_Mutex>::iterator EntNameToEntIdMap_Iter_t;

class ProcMgr : public UniIPC
{
public:

  ProcMgr(ACE_Thread_Manager *thrMgr, ACE_CString ip, ACE_UINT8 entId, ACE_UINT8 instId, ACE_CString nodeTag);
  ProcMgr() = default;
  virtual ~ProcMgr();

  int svc(void);
  ACE_HANDLE get_handle(void) const;
  ACE_UINT32 handle_ipc(ACE_Message_Block *mb);
  int processIPCMessage(ACE_Message_Block &mb);
  int processSpawnReq(ACE_Byte *in, ACE_UINT32 len, ACE_Message_Block &mb);
  int buildSpawnRsp(ACE_Byte *in, pid_t cPid, pid_t pPid, ACE_Message_Block &mb);
private:
  EntNameToEntIdMap_t m_entNameToIdMap;
};

#endif /*__PROCMGR_H__*/
