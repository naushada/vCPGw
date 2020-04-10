#ifndef __DHCP_SERVER_STATE_INIT_CC__
#define __DHCP_SERVER_STATE_INIT_CC__

#include "DhcpServer.h"
#include "DhcpServerStateInit.h"
#include "DhcpServerStateRequest.h"
#include "ace/Log_Msg.h"

DhcpServerStateInit *DhcpServerStateInit::m_instance = NULL;

DhcpServerStateInit *DhcpServerStateInit::instance()
{
  ACE_TRACE("DhcpServerStateInit::instance\n");

  if(!m_instance)
  {
    ACE_NEW_NORETURN(m_instance, DhcpServerStateInit());
  }

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
  delete m_instance;
  m_instance = NULL;
}

void DhcpServerStateInit::onEntry(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateInit::onEntry\n");
}

void DhcpServerStateInit::onExit(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateInit::onExit\n");
}

ACE_UINT32 DhcpServerStateInit::offer(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::offer");
  return(0);
}

ACE_UINT32 DhcpServerStateInit::discover(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::discover\n");

  /*Process DISCOVER Message.*/


  /*move to next state.*/
  parent.setState(DhcpServerStateRequest::instance());
  return(0);
}

ACE_UINT32 DhcpServerStateInit::request(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::request\n");
  return(0);
}

ACE_UINT32 DhcpServerStateInit::requestAck(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::requestAck\n");
  return(0);
}

ACE_UINT32 DhcpServerStateInit::leaseTO(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateInit::leaseTO\n");
  return(0);
}


/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateInit::guardTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateInit::guardTimerExpiry\n");
  return(0);
}

/*lease Timer is started when IP address assignment is done successfully.*/
ACE_UINT32 DhcpServerStateInit::leaseTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateInit::leaseTimerExpiry\n");
  return(0);
}

#endif /*__DHCP_SERVER_STATE_INIT_CC__*/
