#ifndef __DHCP_SERVER_STATE_OFFER_CC__
#define __DHCP_SERVER_STATE_OFFER_CC__

#include "DhcpServerStateOffer.h"
#include "ace/Log_Msg.h"

DhcpServerStateOffer *DhcpServerStateOffer::m_instance = NULL;

DhcpServerStateOffer *DhcpServerStateOffer::instance()
{
  ACE_TRACE("DhcpServerStateOffer::instance\n");
  if(!m_instance)
  {
    ACE_NEW_NORETURN(m_instance, DhcpServerStateOffer());
  }

  return (m_instance);
}

DhcpServerStateOffer::~DhcpServerStateOffer()
{
  ACE_TRACE("DhcpServerStateOffer::~DhcpServerStateOffer\n");
  delete m_instance;
}

void DhcpServerStateOffer::onEntry(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateOffer::onEntry\n");
}

void DhcpServerStateOffer::onExit(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateOffer::onExit\n");
}

ACE_UINT32 DhcpServerStateOffer::offer(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateOffer::offer\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::discover(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateOffer::discover\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::request(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateOffer::request\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::requestAck(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateOffer::requestAck\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::leaseTO(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateOffer::leaseTO\n");
  return(0);
}

/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateOffer::guardTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateOffer::guardTimerExpiry\n");
  return(0);
}

/*lease Timer is started when IP address assignment is done successfully.*/
ACE_UINT32 DhcpServerStateOffer::leaseTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateOffer::leaseTimerExpiry\n");
  return(0);
}

#endif /*__DHCP_SERVER_STATE_OFFER_CC__*/
