#ifndef __DHCP_SERVER_STATE_RELEASE_CC__
#define __DHCP_SERVER_STATE_RELEASE_CC__

#include "ace/Basic_Types.h"
#include "ace/SString.h"

#include "DhcpServerStateRelease.h"
#include "DhcpServer.h"
#include "DhcpServerUser.h"

DhcpServerStateRelease *DhcpServerStateRelease::m_instance = NULL;

DhcpServerStateRelease *DhcpServerStateRelease::instance()
{
  ACE_TRACE("DhcpServerStateRelease::instance\n");
  if(!m_instance)
  {
    ACE_NEW_NORETURN(m_instance, DhcpServerStateRelease());
  }

  return(m_instance);
}

DhcpServerStateRelease::DhcpServerStateRelease()
{
  ACE_TRACE("DhcpServerStateRelease::DhcpServerStateRelease\n");
}

DhcpServerStateRelease::~DhcpServerStateRelease()
{
  ACE_TRACE("DhcpServerStateRelease::~DhcpServerStateRelease\n");
  m_instance = NULL;
}

void DhcpServerStateRelease::onEntry(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRelease::onEntry\n");
  /*1 second of grace period before removing the subscriber.*/
  ACE_UINT32 to = 1;

  TIMER_ID *act = new TIMER_ID();
  act->timerType(DHCP::PURGE_TIMER_ID);
  act->chaddrLen(parent.ctx().chaddrLen());
  ACE_OS::memcpy((void *)act->chaddr(), (const void *)parent.ctx().chaddr(),
                 parent.ctx().chaddrLen());

  /*Start a timer of 1seconds*/
  parent.getDhcpServerUser().guardTid(parent.getDhcpServerUser().start_timer(to, (const void *)act));
}

void DhcpServerStateRelease::onExit(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRelease::onExit\n");
}

ACE_UINT32 DhcpServerStateRelease::offer(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::offer\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::discover(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::discover\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::request(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::request\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::requestAck(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::requestAck\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::leaseTO(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::leaseTO\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::release(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::release\n");
  return(0);
}

/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateRelease::guardTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateRelease::guardTimerExpiry\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::leaseTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateRelease::leaseTimerExpiry\n");
  return(0);
}

#endif /*__DHCP_SERVER_STATE_RELEASE_CC__*/
