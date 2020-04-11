#ifndef __DHCP_SERVER_STATE_REQUEST_ACK_CC__
#define __DHCP_SERVER_STATE_REQUEST_ACK_CC__

#include "DhcpServerStateRequestAck.h"
#include "ace/Log_Msg.h"

DhcpServerStateRequestAck *DhcpServerStateRequestAck::m_instance = NULL;

DhcpServerStateRequestAck *DhcpServerStateRequestAck::instance()
{
  ACE_TRACE("DhcpServerStateRequestAck::get_instance\n");
  if(!m_instance)
  {
    ACE_NEW_NORETURN(m_instance, DhcpServerStateRequestAck());
  }

  return(m_instance);
}

DhcpServerStateRequestAck::~DhcpServerStateRequestAck()
{
  ACE_TRACE("DhcpServerStateRequestAck::~DhcpServerStateRequestAck\n");
  delete m_instance;
}
void DhcpServerStateRequestAck::onEntry(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRequestAck::onEntry\n");
}

void DhcpServerStateRequestAck::onExit(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRequestAck::onExit\n");
}

ACE_UINT32 DhcpServerStateRequestAck::offer(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRequestAck::offer\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::discover(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRequestAck::discover\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::request(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRequestAck::request\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::requestAck(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRequestAck::requestAck\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::leaseTO(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRequestAck::leaseTO\n");
  return(0);
}

/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateRequestAck::guardTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateRequestAck::guardTimerExpiry\n");
  return(0);
}

/*lease Timer is started when IP address assignment is done successfully.*/
ACE_UINT32 DhcpServerStateRequestAck::leaseTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateRequestAck::leaseTimerExpiry\n");
  return(0);
}

#endif /*__DHCP_SERVER_STATE_REQUEST_ACK_CC__*/
