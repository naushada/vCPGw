#ifndef __DHCP_SERVER_STATE_REQUEST_CC__
#define __DHCP_SERVER_STATE_REQUEST_CC__

#include "DhcpServerStateRelease.h"
#include "DhcpServerStateLeaseExpire.h"
#include "DhcpServerStateRequest.h"
#include "DhcpServer.h"
#include "DhcpServerUser.h"
#include "ace/Log_Msg.h"

DhcpServerStateRequest *DhcpServerStateRequest::m_instance = nullptr;

DhcpServerStateRequest *DhcpServerStateRequest::instance()
{
  ACE_TRACE("DhcpServerStateRequest::instance\n");
  if(!m_instance)
  {
    ACE_NEW_NORETURN(m_instance, DhcpServerStateRequest());
  }

  return(m_instance);
}

DhcpServerStateRequest *DhcpServerStateRequest::get_instance(void)
{
  return(m_instance);
}

DhcpServerStateRequest::DhcpServerStateRequest()
{
  ACE_TRACE("DhcpServerStateRequest::instance\n");
  ACE_CString desc("DhcpServerStateRequest");
  m_description = desc;
}

DhcpServerStateRequest::~DhcpServerStateRequest()
{
  ACE_TRACE("DhcpServerStateRequest::~DhcpServerStateRequest\n");
  m_instance = NULL;
}

void DhcpServerStateRequest::onEntryImpl(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRequest::onEntryImpl\n");

  /*2 seconds.*/
  ACE_UINT32 to = 2;

  /*Start house keeping of timer's contents now.*/
  parent.purgeTid().timerType(DHCP::PURGE_TIMER_MSG_ID);
  parent.purgeTid().chaddrLen(parent.ctx().chaddrLen());
  parent.purgeTid().chaddr(parent.ctx().chaddr());

  /*Start a timer of 2seconds*/
  long timerId = parent.getDhcpServerUser().start_timer(to, (const void *)parent.purgeInst());
  parent.purgeTid().tid(timerId);
}

void DhcpServerStateRequest::onExitImpl(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRequest::onExitImpl\n");
  parent.getDhcpServerUser().stop_timer(parent.purgeTid().tid());
}

ACE_UINT32 DhcpServerStateRequest::offerImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequest::offerImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequest::discoverImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequest::discoverImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequest::requestImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequest::requestImpl\n");

  parent.getDhcpServerUser().stop_timer(parent.purgeTid().tid());

  /*Prepare Request ACK.*/
  ACE_Message_Block &mb = buildResponse(parent, in, inLen);

  ACE_CString cha((const char *)parent.ctx().chaddr(), parent.ctx().chaddrLen());
  parent.getDhcpServerUser().sendResponse(cha, (ACE_Byte *)mb.rd_ptr(), mb.length());

  delete &mb;

  /*Move to next State.*/
  parent.setState(DhcpServerStateLeaseExpire::instance());
  return(0);
}

ACE_UINT32 DhcpServerStateRequest::requestAckImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequest::requestAckImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequest::leaseTOImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequest::leaseTOImpl\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequest::releaseImpl(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequest::releaseImpl\n");
  /*stop leaseExpiry Timer now.*/
  parent.getDhcpServerUser().stop_timer(parent.purgeTid().tid());
  parent.setState(DhcpServerStateRelease::instance());

  return(0);
}

ACE_UINT32 DhcpServerStateRequest::nackImpl(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequest::nackImpl\n");

  return(0);
}

ACE_UINT32 DhcpServerStateRequest::declineImpl(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequest::declineImpl\n");

  return(0);
}

/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateRequest::guardTimerExpiryImpl(DHCP::Server &parent, const void *act)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l DhcpServerStateRequest::guardTimerExpiryImpl\n")));
  return(0);
}

/*lease Timer is started when IP address assignment is done successfully.*/
ACE_UINT32 DhcpServerStateRequest::leaseTimerExpiryImpl(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateRequest::leaseTimerExpiryImpl\n");
  guardTimerExpiry(parent, act);
  return(0);
}

#endif /*__DHCP_SERVER_STATE_REQUEST_CC__*/
