#ifndef __DHCP_SERVER_STATE_LEASE_EXPIRE__CC__
#define __DHCP_SERVER_STATE_LEASE_EXPIRE__CC__

#include "DhcpServer.h"
#include "DhcpServerUser.h"
#include "DhcpServerStateLeaseExpire.h"
#include "DhcpServerStateRelease.h"
#include "ace/Log_Msg.h"
#include "ace/SString.h"

DhcpServerStateLeaseExpire *DhcpServerStateLeaseExpire::m_instance = NULL;

DhcpServerStateLeaseExpire *DhcpServerStateLeaseExpire::instance()
{
  ACE_TRACE("DhcpServerStateLeaseExpire::instance");
  if(!m_instance)
  {
    ACE_NEW_NORETURN(m_instance, DhcpServerStateLeaseExpire());
  }

  return(m_instance);
}

void DhcpServerStateLeaseExpire::onEntry(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::onEntry");
  /*Starting the lease timer, upon its expity, subscriber IP will be purged off..*/
  ACE_UINT32 to = parent.lease();

  TIMER_ID *act = new TIMER_ID();
  act->timerType(DHCP::LEASE_TIMER_ID);
  act->chaddrLen(parent.ctx().chaddrLen());
  ACE_OS::memcpy((void *)act->chaddr(), (const void *)parent.ctx().chaddr(),
                 parent.ctx().chaddrLen());

  /*Start a timer of leaseTO*/
  parent.getDhcpServerUser().leaseTid(parent.getDhcpServerUser().start_timer(to, (const void *)act));
}

void DhcpServerStateLeaseExpire::onExit(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::onExit");
  /*stop leaseExpiry Timer now.*/
  parent.getDhcpServerUser().stop_timer(parent.getDhcpServerUser().leaseTid());
}

ACE_UINT32 DhcpServerStateLeaseExpire::offer(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::offer");
  return(0);
}

ACE_UINT32 DhcpServerStateLeaseExpire::discover(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::discover");
  return(0);
}

ACE_UINT32 DhcpServerStateLeaseExpire::requestAck(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::requestAck");
  return(0);
}

ACE_UINT32 DhcpServerStateLeaseExpire::leaseTO(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::leaseTO");
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

ACE_UINT32 DhcpServerStateLeaseExpire::release(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::release\n");
  /*just kick the state machine so that, a timer is started.*/
  parent.setState(DhcpServerStateRelease::instance());

  return(0);
}

/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateLeaseExpire::guardTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::guardTimerExpiry\n");

  return(0);
}

/*lease Timer is started when IP address assignment is done successfully.*/
ACE_UINT32 DhcpServerStateLeaseExpire::leaseTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::leaseTimerExpiry\n");
  guardTimerExpiry(parent, act);
  return(0);
}

ACE_UINT32 DhcpServerStateLeaseExpire::request(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateLeaseExpire::request\n");

  /*Prepare Request ACK.*/
  ACE_Message_Block &mb = buildResponse(parent, in, inLen);

  ACE_CString cha((const char *)parent.ctx().chaddr());
  parent.getDhcpServerUser().sendResponse(cha, (ACE_Byte *)mb.rd_ptr(), mb.length());

  delete &mb;
  /*start the leaseTimer again by kicking the state machine.*/
  parent.setState(DhcpServerStateLeaseExpire::instance());
  return(0);
}

#endif /*__DHCP_SERVER_STATE_LEASE_EXPIRE__CC__*/
