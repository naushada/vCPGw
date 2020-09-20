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

void DhcpServerStateRelease::onEntryImpl(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRelease::onEntryImpl\n");
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

void DhcpServerStateRelease::onExitImpl(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRelease::onExitImpl\n");
}

ACE_UINT32 DhcpServerStateRelease::offerImpl(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::offerImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::discoverImpl(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::discoverImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::requestImpl(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::requestImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::requestAckImpl(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::requestAckImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::leaseTOImpl(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::leaseTOImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::releaseImpl(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::releaseImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::nackImpl(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::nackImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::declineImpl(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRelease::declineImpl\n");
  return(0);
}

/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateRelease::guardTimerExpiryImpl(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateRelease::guardTimerExpiryImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRelease::leaseTimerExpiryImpl(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateRelease::leaseTimerExpiryImpl\n");
  return(0);
}

#endif /*__DHCP_SERVER_STATE_RELEASE_CC__*/
