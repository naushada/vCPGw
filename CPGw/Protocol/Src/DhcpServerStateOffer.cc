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

void DhcpServerStateOffer::onEntryImpl(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateOffer::onEntryImpl\n");
}

void DhcpServerStateOffer::onExitImpl(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateOffer::onExitImpl\n");
}

ACE_UINT32 DhcpServerStateOffer::offerImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateOffer::offerImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::discoverImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateOffer::discoverImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::requestImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateOffer::requestImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::requestAckImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateOffer::requestAckImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::nackImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateOffer::nackImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::releaseImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateOffer::releaseImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::declineImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateOffer::declineImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateOffer::leaseTOImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateOffer::leaseTOImpl\n");
  return(0);
}

/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateOffer::guardTimerExpiryImpl(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateOffer::guardTimerExpiryImpl\n");
  return(0);
}

/*lease Timer is started when IP address assignment is done successfully.*/
ACE_UINT32 DhcpServerStateOffer::leaseTimerExpiryImpl(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateOffer::leaseTimerExpiryImpl\n");
  return(0);
}

#endif /*__DHCP_SERVER_STATE_OFFER_CC__*/
