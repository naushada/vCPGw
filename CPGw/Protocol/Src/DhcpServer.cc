#ifndef __DHCP_SERVER_CC__
#define __DHCP_SERVER_CC__

#include "ace/Reactor.h"
#include "ace/Event_Handler.h"
#include "ace/SString.h"
#include "ace/SOCK_Dgram.h"
#include "ace/Log_Msg.h"

#include "DhcpServer.h"
#include "DhcpServerStateDiscover.h"

DHCP::Server::Server(DhcpServerUser *parent, ACE_CString mac,
                     ACE_CString ip, ACE_CString hName,
                     ACE_CString dName)
{
  setDhcpServerUser(parent);
  setMacAddress(mac);
  hostName(hName);
  domainName(dName);
  lease(5);
  ipAddr(0);
  setMyIP(ip);
  m_mb = NULL;
  m_description.set("DhcpServer");
  m_state = NULL;
  m_optionMap.unbind_all();

  /*context of DHCP Client's dhcp-header.*/
  ACE_NEW_NORETURN(m_ctx, RFC2131::DhcpCtx());

  /*The start state is Discover.*/
  setState(DhcpServerStateDiscover::instance());
}


DHCP::Server::Server()
{
  /*context of DHCP Client's dhcp-header.*/
  ACE_NEW_NORETURN(m_ctx, RFC2131::DhcpCtx());

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

void DHCP::Server::setMyIP(ACE_CString ip)
{
  m_myIP = ip;
}

ACE_CString &DHCP::Server::getMyIP(void)
{
  return(m_myIP);
}

void DHCP::Server::ipAddr(ACE_CString IPStr)
{
  ACE_UINT32 ip = ipAddr();
  ACE_UINT16 dontCare = 8080;
  ACE_INET_Addr addr(dontCare, ip);
  const char *ipStr = addr.get_host_addr();

  if(ipStr)
  {
    IPStr.set(ipStr, ACE_OS::strlen(ipStr));
  }
}

ACE_CString &DHCP::Server::hostName(void)
{
  return(m_hostName);
}

void DHCP::Server::hostName(ACE_CString &hName)
{
  m_hostName = hName;
}

ACE_CString &DHCP::Server::domainName(void)
{
  return(m_domainName);
}

void DHCP::Server::domainName(ACE_CString &dName)
{
  m_domainName = dName;
}

#endif /*__DHCP_SERVER_CC__*/
