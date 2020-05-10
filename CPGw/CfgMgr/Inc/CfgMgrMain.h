#ifndef __CFG_MGR_MAIN_H__
#define __CFG_MGR_MAIN_H__

#include <ace/Basic_Types.h>
#include <ace/Log_Msg.h>
#include <ace/INET_Addr.h>

class CfgMgrMain
{
public:
  CfgMgrMain() = default;
  ~CfgMgrMain() = default;

  void start(void);
  void stop(void);

private:
  ACE_INET_Addr m_myAddr;
};


#endif /*__CFG_MGR_MAIN_H__*/
