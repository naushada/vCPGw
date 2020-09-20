#ifndef __DHCP_SERVER_STATE_LEASE_EXPIRE__CC__
#define __DHCP_SERVER_STATE_LEASE_EXPIRE__CC__

#include "DhcpServer.h"
#include "DhcpServerUser.h"
#include "DhcpServerStateLeaseExpire.h"
#include "DhcpServerStateRelease.h"
#include "ace/Log_Msg.h"
#include "ace/SString.h"

DhcpServerStateLeaseExpire *DhcpServerStateLeaseExpire::m_instance = nullptr;

DhcpServerStateLeaseExpire *DhcpServerStateLeaseExpire::instance()
{
  ACE_TRACE("DhcpServerStateLeaseExpire::instance");
  if(!m_instance)
  {
    ACE_NEW_NORETURN(m_instance, DhcpServerStateLeaseExpire());
  }

  return(m_instance);
}

DhcpServerStateLeaseExpire *DhcpServerStateLeaseExpire::get_instance(void)
{
  return(m_instance);
}

void DhcpServerStateLeaseExpire::onEntryImpl(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::onEntryImpl");
  /*Starting the lease timer, upon its expiry, subscriber IP will be purged off..*/
  ACE_UINT32 to = parent.lease();

  /*Start house keeping of timer's contents now.*/
  parent.purgeTid().timerType(DHCP::PURGE_TIMER_MSG_ID);
  parent.purgeTid().chaddrLen(parent.ctx().chaddrLen());
  parent.purgeTid().chaddr(parent.ctx().chaddr());

  /*Start a timer of 2seconds*/
  long timerId = parent.getDhcpServerUser().start_timer(to, (const void *)parent.purgeInst());
  parent.purgeTid().tid(timerId);
}

void DhcpServerStateLeaseExpire::onExitImpl(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::onExitImpl");
  /*stop leaseExpiry Timer now.*/
  parent.getDhcpServerUser().stop_timer(parent.purgeTid().tid());
}

ACE_UINT32 DhcpServerStateLeaseExpire::offerImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::offerImpl");
  return(0);
}

ACE_UINT32 DhcpServerStateLeaseExpire::discoverImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::discoverImpl");
  return(0);
}

ACE_UINT32 DhcpServerStateLeaseExpire::requestAckImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::requestAckImpl");
  return(0);
}

ACE_UINT32 DhcpServerStateLeaseExpire::leaseTOImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::leaseTOImpl");
  return(0);
}

ACE_UINT32 DhcpServerStateLeaseExpire::nackImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::nackImpl");
  return(0);
}

ACE_UINT32 DhcpServerStateLeaseExpire::declineImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::declineImpl");
  return(0);
}

ACE_UINT32 DhcpServerStateLeaseExpire::releaseImpl(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::releaseImpl\n");

  parent.getDhcpServerUser().stop_timer(parent.purgeTid().tid());

  /*just kick the state machine so that, a timer is started.*/
  parent.setState(DhcpServerStateRelease::instance());

  return(0);
}

/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateLeaseExpire::guardTimerExpiryImpl(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::guardTimerExpiryImpl\n");

  return(0);
}

/*lease Timer is started when IP address assignment is done successfully.*/
ACE_UINT32 DhcpServerStateLeaseExpire::leaseTimerExpiryImpl(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::leaseTimerExpiryImpl\n");
  guardTimerExpiry(parent, act);
  return(0);
}

ACE_UINT32 DhcpServerStateLeaseExpire::requestImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::requestImpl\n");

  /*Stop the timer now.*/
  parent.getDhcpServerUser().stop_timer(parent.purgeTid().tid());
  /*Prepare Request ACK.*/
  ACE_Message_Block &mb = buildResponse(parent, in, inLen);

  ACE_CString cha((const char *)parent.ctx().chaddr(), parent.ctx().chaddrLen());
  parent.getDhcpServerUser().sendResponse(cha, (ACE_Byte *)mb.rd_ptr(), mb.length());

  delete &mb;

  /*Reset the timer now*/
  parent.getDhcpServerUser().reset_timer(parent.purgeTid().tid(), parent.lease());
  return(0);
}

DhcpServerStateLeaseExpire::DhcpServerStateLeaseExpire()
{
  ACE_TRACE("DhcpServerStateLeaseExpire::DhcpServerStateLeaseExpire\n");
  ACE_CString desc("DhcpServerStateLeaseExpire");
  m_description = desc;
}

DhcpServerStateLeaseExpire::~DhcpServerStateLeaseExpire()
{
  ACE_TRACE("DhcpServerStateLeaseExpire::~DhcpServerStateLeaseExpire\n");
  m_instance = NULL;
}

#endif /*__DHCP_SERVER_STATE_LEASE_EXPIRE__CC__*/
