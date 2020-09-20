#ifndef __DHCP_SERVER_STATE_INIT_CC__
#define __DHCP_SERVER_STATE_INIT_CC__

#include "DhcpServer.h"
#include "DhcpServerStateInit.h"
#include "DhcpServerStateRequest.h"
#include "ace/Log_Msg.h"

DhcpServerStateInit *DhcpServerStateInit::m_instance = nullptr;

DhcpServerStateInit *DhcpServerStateInit::instance()
{
  ACE_TRACE("DhcpServerStateInit::instance\n");

  if(!m_instance)
  {
    ACE_NEW_NORETURN(m_instance, DhcpServerStateInit());
  }

  return(m_instance);
}

DhcpServerStateInit *DhcpServerStateInit::get_instance(void)
{
  return(m_instance);
}

DhcpServerStateInit::DhcpServerStateInit()
{
  ACE_TRACE("DhcpServerStateInit::DhcpServerStateInit\n");
  ACE_CString desc("DhcpServerStateInit");
  m_description = desc;
}

DhcpServerStateInit::~DhcpServerStateInit()
{
  ACE_TRACE("DhcpServerStateInit::~DhcpServerStateInit\n");
  m_instance = NULL;
}

void DhcpServerStateInit::onEntryImpl(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateInit::onEntryImpl\n");
}

void DhcpServerStateInit::onExitImpl(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateInit::onExitImpl\n");
}

ACE_UINT32 DhcpServerStateInit::declineImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::declineImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateInit::nackImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::nackImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateInit::offerImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::offerImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateInit::discoverImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::discoverImpl\n");

  /*Process DISCOVER Message.*/
  /*move to next state.*/
  parent.setState(DhcpServerStateRequest::instance());
  return(0);
}

ACE_UINT32 DhcpServerStateInit::releaseImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::releaseImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateInit::requestImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::requestImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateInit::requestAckImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::requestAckImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateInit::leaseTOImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::leaseTOImpl\n");
  return(0);
}

/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateInit::guardTimerExpiryImpl(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateInit::guardTimerExpiryImpl\n");
  return(0);
}

/*lease Timer is started when IP address assignment is done successfully.*/
ACE_UINT32 DhcpServerStateInit::leaseTimerExpiryImpl(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateInit::leaseTimerExpiryImpl\n");
  return(0);
}

#endif /*__DHCP_SERVER_STATE_INIT_CC__*/
