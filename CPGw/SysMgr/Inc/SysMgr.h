#ifndef __SYSMGR_H__
#define __SYSMGR_H__

#include "UniIPC.h"
#include "CommonIF.h"
#include "Json.h"

#include "ace/Basic_Types.h"
#include "ace/Message_Block.h"
#include "ace/SString.h"
#include "ace/Hash_Map_Manager.h"
#include "ace/Null_Mutex.h"
#include "ace/Task.h"
#include "ace/Log_Msg.h"
#include "ace/Reactor.h"
#include "ace/Process.h"
#include "ace/Event_Handler.h"

namespace CPTaskTable
{
  typedef struct _taskTable
  {
    ACE_TCHAR m_taskName[64];
    ACE_UINT8 m_startLevel;
    ACE_TCHAR m_parentTask[64];
    ACE_UINT8 m_container;
    ACE_UINT8 m_visible;
    ACE_UINT8 m_minInstance;
    ACE_UINT8 m_maxInstance;
    ACE_UINT32 m_taskId;

    _taskTable()
    {
      ACE_OS::memset((void *)m_taskName, 0, sizeof(m_taskName));
      ACE_OS::memset((void *)m_parentTask, 0, sizeof(m_parentTask));
      m_startLevel = 0;
      m_container = 0;
      m_visible = 0;
      m_minInstance = 0;
      m_maxInstance = 0;
    }

    ~_taskTable()
    {
    }

    ACE_TCHAR *taskName(void)
    {
      return((ACE_TCHAR *)m_taskName);
    }

    void taskName(ACE_TCHAR *tName)
    {
      /*+1 for null(\0) character*/
      ACE_OS::strncpy(m_taskName, tName, (sizeof(m_taskName) - 1));
    }

    ACE_UINT8 startLevel(void)
    {
      return(m_startLevel);
    }

    void startLevel(ACE_UINT8 sLevel)
    {
      m_startLevel = sLevel;
    }

    ACE_TCHAR *parentTask(void)
    {
      return((ACE_TCHAR *)m_parentTask);
    }

    void parentTask(ACE_TCHAR *pTask)
    {
      ACE_OS::strncpy(m_parentTask, pTask, (sizeof(m_parentTask) - 1));
    }

    ACE_UINT8 minInstance(void)
    {
      return(m_minInstance);
    }

    void minInstance(ACE_UINT8 mInstance)
    {
      m_minInstance = mInstance;
    }

    ACE_UINT8 maxInstance(void)
    {
      return(m_maxInstance);
    }

    void maxInstance(ACE_UINT8 mInstance)
    {
      m_maxInstance = mInstance;
    }

    ACE_UINT32 taskId(void)
    {
      return(m_taskId);
    }

    void taskId(ACE_UINT32 tId)
    {
      m_taskId = tId;
    }

  }_taskTable_t;
};

/*Task Id is the Key.*/
typedef ACE_Hash_Map_Manager<ACE_CString, CPTaskTable::_taskTable_t, ACE_Null_Mutex> taskTableMap_t;
typedef ACE_Hash_Map_Manager<ACE_CString, CPTaskTable::_taskTable_t, ACE_Null_Mutex>::iterator taskTableMapIter_t;

class SysMgr : public UniIPC
{
public:
  SysMgr() = default;
  SysMgr(ACE_Thread_Manager *thr, ACE_CString ip, ACE_UINT8 entId, ACE_UINT8 instId, ACE_CString nodeTag);
  virtual ~SysMgr();

  int start(void);
  int svc(void);

  ACE_HANDLE get_handle(void) const;
  ACE_UINT32 handle_ipc(ACE_Message_Block *mb);
  ACE_UINT32 process_signal(int signum);
  int processIPCMessage(ACE_Message_Block &mb);
  int processSpawnRsp(ACE_Byte *in, ACE_UINT32 len, ACE_Message_Block &mb);
  int processChildDiedInd(ACE_Message_Block &mb);
  void buildAndSendSpawnReq(ACE_CString &entName, ACE_UINT8 instId, ACE_Message_Block &mb);

  ACE_UINT8 get_entId(ACE_CString &entName);
  void populateProcessTable(void);
  JSON &jsonObj(void);
  void jsonObj(JSON *obj);

private:
  JSON *m_jsonObj;

  taskTableMap_t m_taskMap;
  taskTableMapIter_t *m_taskIter;

  ACE_CString m_schema;

};




















#endif /*__SYSMGR_H__*/
