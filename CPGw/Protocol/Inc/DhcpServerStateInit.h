#ifndef __DHCP_SERVER_STATE_INIT_H__
#define __DHCP_SERVER_STATE_INIT_H__

#include "DhcpServerState.h"

class DhcpServerStateInit : public DhcpServerState
{
private:
  static DhcpServerStateInit *m_instance;
  DhcpServerStateInit();

public:
  virtual ~DhcpServerStateInit();
  static DhcpServerStateInit *instance();
  static DhcpServerStateInit *get_instance(void);

  void onEntry(DHCP::Server &parent);
  void onExit(DHCP::Server &parent);
  ACE_UINT32 offer(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen);
  ACE_UINT32 discover(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen);
  ACE_UINT32 request(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen);
  ACE_UINT32 requestAck(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen);
  ACE_UINT32 leaseTO(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen);
  /*Guard Timer is stared if next request is expected to complete the Flow.*/
  ACE_UINT32 guardTimerExpiry(DHCP::Server &parent, const void *act);
  /*lease Timer is started when IP address assignment is done successfully.*/
  ACE_UINT32 leaseTimerExpiry(DHCP::Server &parent, const void *act);

};

#endif /*__DHCP_SERVER_STATE_INIT_H__*/
