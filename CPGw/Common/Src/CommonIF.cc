#ifndef __COMMONIF_CC__
#define __COMMONIF_CC__

#include <ace/Log_Msg.h>
#include "CommonIF.h"

ACE_UINT32 CommonIF::get_hash32(const char *nodeTag)
{
  ACE_TRACE(("CommonIF::get_hash32\n"));
  std::hash<const char *>hs;
  return(hs(nodeTag));
}

ACE_UINT16 CommonIF::get_ipc_port(ACE_UINT8 facility, ACE_UINT8 instance)
{
  ACE_TRACE(("CommonIF::get_ip_port\n"));
  return((facility << 8) | instance);
}

ACE_UINT32 CommonIF::get_task_id(ACE_UINT8 facility, ACE_UINT8 instance)
{
  ACE_TRACE(("CommonIF::get_task_id\n"));
  return(OFFSET << 24 |
         CONTAINER << 16 |
         facility << 8 |
         instance);
}

ACE_UINT16 CommonIF::get_ipc_port(ACE_UINT32 taskId)
{
  ACE_TRACE(("CommonIF::get_ipc_port\n"));
  ACE_UINT16 mask = ~(0);
  return(taskId & mask);
}

ACE_UINT8 CommonIF::get_inst_id(ACE_UINT32 tId)
{
  ACE_TRACE(("CommonIF::get_inst_id\n"));
  return(tId & 0b11111111);
}

ACE_UINT8 CommonIF::get_ent_id(ACE_UINT32 tId)
{
  ACE_TRACE(("CommonIF::get_ent_id\n"));
  return((tId >> 8) & 0b11111111);
}



































#endif /*__COMMONIF_CC__*/
