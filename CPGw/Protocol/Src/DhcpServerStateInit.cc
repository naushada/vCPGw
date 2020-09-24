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

void DhcpServerStateInit::do_onEntry(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateInit::do_onEntry\n");
}

void DhcpServerStateInit::do_onExit(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateInit::do_onExit\n");
}

ACE_UINT32 DhcpServerStateInit::do_decline(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::do_decline\n");
  return(0);
}

ACE_UINT32 DhcpServerStateInit::do_nack(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::do_nack\n");
  return(0);
}

ACE_UINT32 DhcpServerStateInit::do_offer(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::do_offer\n");
  return(0);
}

ACE_UINT32 DhcpServerStateInit::do_discover(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::do_discover\n");

  /*Process DISCOVER Message.*/
  /*move to next state.*/
  parent.setState(DhcpServerStateRequest::instance());
  return(0);
}

ACE_UINT32 DhcpServerStateInit::do_release(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::do_release\n");
  return(0);
}

ACE_UINT32 DhcpServerStateInit::do_request(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::do_request\n");
  return(0);
}

ACE_UINT32 DhcpServerStateInit::do_requestAck(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::do_requestAck\n");
  return(0);
}

ACE_UINT32 DhcpServerStateInit::do_leaseTO(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::do_leaseTO\n");
  return(0);
}

/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateInit::do_guardTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateInit::do_guardTimerExpiry\n");
  return(0);
}

/*lease Timer is started when IP address assignment is done successfully.*/
ACE_UINT32 DhcpServerStateInit::do_leaseTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateInit::do_leaseTimerExpiry\n");
  return(0);
}

#endif /*__DHCP_SERVER_STATE_INIT_CC__*/
