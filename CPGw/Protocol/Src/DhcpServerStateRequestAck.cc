#ifndef __DHCP_SERVER_STATE_REQUEST_ACK_CC__
#define __DHCP_SERVER_STATE_REQUEST_ACK_CC__

#include "DhcpServerStateRequestAck.h"
#include "ace/Log_Msg.h"

DhcpServerStateRequestAck *DhcpServerStateRequestAck::m_instance = nullptr;

DhcpServerStateRequestAck *DhcpServerStateRequestAck::instance()
{
  ACE_TRACE("DhcpServerStateRequestAck::get_instance\n");
  if(!m_instance)
  {
    ACE_NEW_NORETURN(m_instance, DhcpServerStateRequestAck());
  }

  return(m_instance);
}

DhcpServerStateRequestAck *DhcpServerStateRequestAck::get_instance(void)
{
  return(m_instance);
}

DhcpServerStateRequestAck::~DhcpServerStateRequestAck()
{
  ACE_TRACE("DhcpServerStateRequestAck::~DhcpServerStateRequestAck\n");
  m_instance = NULL;
}

void DhcpServerStateRequestAck::do_onEntry(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRequestAck::do_onEntry\n");
}

void DhcpServerStateRequestAck::do_onExit(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRequestAck::do_onExit\n");
}

ACE_UINT32 DhcpServerStateRequestAck::do_offer(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequestAck::do_offer\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::do_discover(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequestAck::do_discover\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::do_request(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequestAck::do_request\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::do_requestAck(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequestAck::do_requestAck\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::do_nack(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequestAck::do_nack\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::do_decline(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequestAck::do_decline\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::do_release(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequestAck::do_release\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::do_leaseTO(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequestAck::do_leaseTO\n");
  return(0);
}

/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateRequestAck::do_guardTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateRequestAck::do_guardTimerExpiry\n");
  return(0);
}

/*lease Timer is started when IP address assignment is done successfully.*/
ACE_UINT32 DhcpServerStateRequestAck::do_leaseTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateRequestAck::do_leaseTimerExpiry\n");
  return(0);
}

#endif /*__DHCP_SERVER_STATE_REQUEST_ACK_CC__*/
