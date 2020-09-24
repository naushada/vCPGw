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

void DhcpServerStateRequest::do_onEntry(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRequest::do_onEntry\n");

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

void DhcpServerStateRequest::do_onExit(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateRequest::do_onExit\n");
  parent.getDhcpServerUser().stop_timer(parent.purgeTid().tid());
}

ACE_UINT32 DhcpServerStateRequest::do_offer(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequest::do_offer\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequest::do_discover(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequest::do_discover\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequest::do_request(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequest::do_request\n");

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

ACE_UINT32 DhcpServerStateRequest::do_requestAck(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequest::do_requestAck\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequest::do_leaseTO(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequest::do_leaseTO\n");
  return(0);
}

ACE_UINT32 DhcpServerStateRequest::do_release(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequest::do_release\n");
  /*stop leaseExpiry Timer now.*/
  parent.getDhcpServerUser().stop_timer(parent.purgeTid().tid());
  parent.setState(DhcpServerStateRelease::instance());

  return(0);
}

ACE_UINT32 DhcpServerStateRequest::do_nack(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequest::do_nack\n");

  return(0);
}

ACE_UINT32 DhcpServerStateRequest::do_decline(DHCP::Server &parent,ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateRequest::do_decline\n");

  return(0);
}

/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateRequest::do_guardTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l DhcpServerStateRequest::do_guardTimerExpiry\n")));
  return(0);
}

/*lease Timer is started when IP address assignment is done successfully.*/
ACE_UINT32 DhcpServerStateRequest::do_leaseTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateRequest::do_leaseTimerExpiry\n");
  guardTimerExpiry(parent, act);
  return(0);
}

#endif /*__DHCP_SERVER_STATE_REQUEST_CC__*/
