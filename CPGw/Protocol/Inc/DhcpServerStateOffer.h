#ifndef __DHCP_SERVER_STATE_OFFER_H__
#define __DHCP_SERVER_STATE_OFFER_H__

#include "DhcpServerState.h"

class DhcpServerStateOffer : public DhcpServerState
{
private:
  static DhcpServerStateOffer *m_instance;
  DhcpServerStateOffer();
public:
  virtual ~DhcpServerStateOffer();
  static DhcpServerStateOffer *instance();
  void onEntry(DHCP::Server &parent);
  void onExit(DHCP::Server &parent);
  ACE_UINT32 offer(DHCP::Server &parent);
  ACE_UINT32 discover(DHCP::Server &parent);
  ACE_UINT32 request(DHCP::Server &parent);
  ACE_UINT32 requestAck(DHCP::Server &parent);
  ACE_UINT32 leaseTO(DHCP::Server &parent);
  /*Guard Timer is stared if next request is expected to complete the Flow.*/
  ACE_UINT32 guardTimerExpiry(DHCP::Server &parent, const void *act);
  /*lease Timer is started when IP address assignment is done successfully.*/
  ACE_UINT32 leaseTimerExpiry(DHCP::Server &parent, const void *act);

};


#endif /*__DHCP_SERVER_STATE_OFFER_H__*/
