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

void DhcpServerStateRequestAck::onEntryImpl(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRequestAck::onEntryImpl\n");
}

void DhcpServerStateRequestAck::onExitImpl(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRequestAck::onExitImpl\n");
}

ACE_UINT32 DhcpServerStateRequestAck::offerImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequestAck::offerImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::discoverImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequestAck::discoverImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::requestImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequestAck::requestImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::requestAckImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequestAck::requestAckImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::nackImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequestAck::nackImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::declineImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequestAck::declineImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::releaseImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequestAck::releaseImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequestAck::leaseTOImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequestAck::leaseTOImpl\n");
  return(0);
}

/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateRequestAck::guardTimerExpiryImpl(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateRequestAck::guardTimerExpiryImpl\n");
  return(0);
}

/*lease Timer is started when IP address assignment is done successfully.*/
ACE_UINT32 DhcpServerStateRequestAck::leaseTimerExpiryImpl(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateRequestAck::leaseTimerExpiryImpl\n");
  return(0);
}

#endif /*__DHCP_SERVER_STATE_REQUEST_ACK_CC__*/
