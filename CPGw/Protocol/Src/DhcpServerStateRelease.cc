#ifndef __DHCP_SERVER_STATE_RELEASE_CC__
#define __DHCP_SERVER_STATE_RELEASE_CC__

#include "ace/Basic_Types.h"
#include "ace/SString.h"

#include "DhcpServerStateRelease.h"
#include "DhcpServer.h"
#include "DhcpServerUser.h"

DhcpServerStateRelease *DhcpServerStateRelease::m_instance = nullptr;

DhcpServerStateRelease *DhcpServerStateRelease::instance()
{
  ACE_TRACE("DhcpServerStateRelease::instance\n");
  if(!m_instance)
  {
    ACE_NEW_NORETURN(m_instance, DhcpServerStateRelease());
  }

  return(m_instance);
}

DhcpServerStateRelease *DhcpServerStateRelease::get_instance(void)
{
  return(m_instance);
}

DhcpServerStateRelease::DhcpServerStateRelease()
{
  ACE_TRACE("DhcpServerStateRelease::DhcpServerStateRelease\n");
  ACE_CString desc("DhcpServerStateRelease");
  m_description = desc;
}

DhcpServerStateRelease::~DhcpServerStateRelease()
{
  ACE_TRACE("DhcpServerStateRelease::~DhcpServerStateRelease\n");
  m_instance = NULL;
}

void DhcpServerStateRelease::do_onEntry(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRelease::do_onEntry\n");
  /*1 second of grace period before removing the subscriber.*/
  ACE_UINT32 to = 1;

  /*Start house keeping of timer's contents now.*/
  parent.purgeTid().timerType(DHCP::PURGE_TIMER_MSG_ID);
  parent.purgeTid().chaddrLen(parent.ctx().chaddrLen());
  parent.purgeTid().chaddr(parent.ctx().chaddr());

  /*Start a timer of 1seconds*/
  long timerId = parent.getDhcpServerUser().start_timer(to, (const void *)parent.purgeInst());
  parent.purgeTid().tid(timerId);
}

void DhcpServerStateRelease::do_onExit(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRelease::do_onExit\n");
}

ACE_UINT32 DhcpServerStateRelease::do_offer(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::do_offer\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::do_discover(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::do_discover\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::do_request(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::do_request\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::do_requestAck(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::do_requestAck\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::do_leaseTO(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::do_leaseTO\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::do_release(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::do_release\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::do_nack(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::do_nack\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::do_decline(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::do_decline\n");
  return(0);
}

/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateRelease::do_guardTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateRelease::do_guardTimerExpiry\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::do_leaseTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateRelease::do_leaseTimerExpiry\n");
  return(0);
}

#endif /*__DHCP_SERVER_STATE_RELEASE_CC__*/
