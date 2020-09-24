#ifndef __DHCP_SERVER_STATE_OFFER_CC__
#define __DHCP_SERVER_STATE_OFFER_CC__

#include "DhcpServerStateOffer.h"
#include "ace/Log_Msg.h"

DhcpServerStateOffer *DhcpServerStateOffer::m_instance = nullptr;

DhcpServerStateOffer *DhcpServerStateOffer::instance()
{
  ACE_TRACE("DhcpServerStateOffer::instance\n");
  if(!m_instance)
  {
    ACE_NEW_NORETURN(m_instance, DhcpServerStateOffer());
  }

  return (m_instance);
}

DhcpServerStateOffer *DhcpServerStateOffer::get_instance(void)
{
  return(m_instance);
}

DhcpServerStateOffer::~DhcpServerStateOffer()
{
  ACE_TRACE("DhcpServerStateOffer::~DhcpServerStateOffer\n");
  m_instance = NULL;
}

void DhcpServerStateOffer::do_onEntry(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateOffer::onEntryImpl\n");
}

void DhcpServerStateOffer::do_onExit(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateOffer::do_onExit\n");
}

ACE_UINT32 DhcpServerStateOffer::do_offer(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateOffer::do_offer\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::do_discover(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateOffer::do_discover\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::do_request(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateOffer::do_request\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::do_requestAck(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateOffer::do_requestAck\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::do_nack(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateOffer::do_nack\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::do_release(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateOffer::do_release\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::do_decline(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateOffer::do_decline\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::do_leaseTO(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateOffer::do_leaseTO\n");
  return(0);
}

/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateOffer::do_guardTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateOffer::do_guardTimerExpiry\n");
  return(0);
}

/*lease Timer is started when IP address assignment is done successfully.*/
ACE_UINT32 DhcpServerStateOffer::do_leaseTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateOffer::do_leaseTimerExpiry\n");
  return(0);
}

#endif /*__DHCP_SERVER_STATE_OFFER_CC__*/
