#ifndef __COMMONIF_CC__
#define __COMMONIF_CC__

#include <ace/Log_Msg.h>
#include "CommonIF.h"

#if 0
CommonIF::_entNameIdTab_t m_entTable[] = {

  {CommonIF::ENT_CPGW,    "CPGateway"},
  {CommonIF::ENT_AAA,     "AAA"},
  {CommonIF::ENT_CFGMGR,  "CFGMGR"},
  {CommonIF::ENT_SYSMGR,  "SYSMGR"},
  {CommonIF::ENT_PROCMGR, "PROCMGR"},

  /*This shall be last entry*/
  {0, nullptr}
};
#endif

ACE_UINT32 CommonIF::get_hash32(const ACE_UINT8 *nodeTag)
{
  ACE_TRACE(("CommonIF::get_hash32"));
  return(std::hash<const ACE_UINT8 *>()(nodeTag));
}

ACE_UINT16 CommonIF::get_ipc_port(ACE_UINT8 facility, ACE_UINT8 instance)
{
  ACE_TRACE(("CommonIF::get_ip_port"));
  return((facility << 8) | instance);
}

ACE_UINT32 CommonIF::get_task_id(ACE_UINT8 facility, ACE_UINT8 instance)
{
  ACE_TRACE(("CommonIF::get_task_id"));
  return(OFFSET << 24 |
         CONTAINER << 16 |
         facility << 8 |
         instance);
}

ACE_UINT16 CommonIF::get_ipc_port(ACE_UINT32 taskId)
{
  ACE_TRACE(("CommonIF::get_ipc_port"));
  ACE_UINT16 mask = ~(0);
  return(taskId & mask);
}



































#endif /*__COMMONIF_CC__*/
