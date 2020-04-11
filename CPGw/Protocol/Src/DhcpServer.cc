#ifndef __DHCP_SERVER_CC__
#define __DHCP_SERVER_CC__

#include "ace/Reactor.h"
#include "ace/Event_Handler.h"
#include "ace/SString.h"
#include "ace/SOCK_Dgram.h"
#include "ace/Log_Msg.h"

#include "DhcpServer.h"
#include "DhcpServerStateDiscover.h"

DHCP::Server::Server(DhcpServerUser *usr, ACE_CString mac)
{
  m_dhcpServerUser = usr;

  setMacAddress(mac);
  /*context of DHCP Client's dhcp-header.*/
  m_ctx = new RFC2131::DhcpCtx();

  /*The start state is Discover.*/
  setState(DhcpServerStateDiscover::instance());
}


DHCP::Server::Server()
{
  /*context of DHCP Client's dhcp-header.*/
  m_ctx = new RFC2131::DhcpCtx();

  /*The start state is Discover.*/
  setState(DhcpServerStateDiscover::instance());
}

DHCP::Server::~Server()
{
  ACE_TRACE("DHCP::Server::~Server\n");
  delete m_ctx;
  m_ctx = NULL;

  DHCP::ElemDef_iter iter = m_optionMap.begin();
  RFC2131::DhcpOption *opt = NULL;

  for(; iter != m_optionMap.end(); iter++)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    opt = (RFC2131::DhcpOption *)((*iter).int_id_);
    m_optionMap.unbind(opt->getTag());
    delete opt;
  }
}

void DHCP::Server::setState(DhcpServerState *st)
{
  ACE_TRACE("DHCP::Server::setState\n");

  if(m_state)
  {
    m_state->onExit(*this);
  }

  m_state = st;
  m_state->onEntry(*this);
}

DhcpServerState &DHCP::Server::getState(void)
{
  ACE_TRACE("DHCP::Server::getState\n");
  return(*m_state);
}

void DHCP::Server::xid(ACE_UINT32 xid)
{
  m_ctx->xid(xid);
}

ACE_UINT32 DHCP::Server::xid(void)
{
  return(m_ctx->xid());
}

RFC2131::DhcpCtx &DHCP::Server::ctx(void)
{
  return(*m_ctx);
}

DHCP::ElemDef &DHCP::Server::optionMap(void)
{
  return(m_optionMap);
}

ACE_UINT32 DHCP::Server::start()
{
  ACE_TRACE("DHCP::Server::start\n");
#if 0
  ACE_Reactor::instance()->register_handler(this,
                                            ACE_Event_Handler::READ_MASK |
                                            ACE_Event_Handler::TIMER_MASK);
#endif
  return(0);
}

ACE_UINT32 DHCP::Server::stop()
{
  ACE_TRACE("DHCP::Server::stop\n");
  return(0);
}

DhcpServerUser &DHCP::Server::getDhcpServerUser(void)
{
  return(*m_dhcpServerUser);
}

void DHCP::Server::setDhcpServerUser(DhcpServerUser *usr)
{
  m_dhcpServerUser = usr;
}

void DHCP::Server::lease(ACE_UINT32 to)
{
  /*lease is in hour unit.*/
  m_lease = to * 60 * 60;
}

ACE_UINT32 DHCP::Server::lease(void)
{
  return(m_lease);
}

ACE_CString &DHCP::Server::sname(void)
{
  return(m_sname);
}

void DHCP::Server::sname(ACE_CString &sn)
{
  m_sname = sn;
}

ACE_CString &DHCP::Server::getMacAddress(void)
{
  return(m_macAddress);
}

void DHCP::Server::setMacAddress(ACE_CString mac)
{
  m_macAddress = mac;
}

ACE_UINT32 DHCP::Server::ipAddr(void)
{
  return(m_ipAddr);
}

void DHCP::Server::ipAddr(ACE_UINT32 ip)
{
  m_ipAddr = ip;
}

#endif /*__DHCP_SERVER_CC__*/
