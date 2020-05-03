#ifndef __CFG_MGR_MAIN_H__
#define __CFG_MGR_MAIN_H__

#include "Ipc.h"

#include <ace/Basic_Types.h>
#include <ace/Log_Msg.h>
#include <ace/INET_Addr.h>

class CfgMgrMain : public UniIPC
{
public:
  CfgMgrMain(ACE_CString ip, ACE_UINT8 ent, ACE_UINT8 inst, ACE_CString nodeTag);
  virtual ~CfgMgrMain();

  void start(void);
  void stop(void);
  ACE_UINT32 handle_ipc(ACE_UINT8 *req, ACE_UINT32 reqLen);
  ACE_HANDLE get_handle(void) const;

private:
  ACE_INET_Addr m_myAddr;
};


#endif /*__CFG_MGR_MAIN_H__*/
