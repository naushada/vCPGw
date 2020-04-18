#ifndef __DHCP_SERVER_STATE_DISCOVER_CC__
#define __DHCP_SERVER_STATE_DISCOVER_CC__

#include "DhcpServerStateLeaseExpire.h"
#include "DhcpServerUser.h"
#include "DhcpServerStateDiscover.h"
#include "DhcpServer.h"
#include "DhcpServerStateRequest.h"
#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"

DhcpServerStateDiscover *DhcpServerStateDiscover::m_instance = NULL;

DhcpServerStateDiscover *DhcpServerStateDiscover::instance()
{
  ACE_TRACE("DhcpServerStateDiscover::instance\n");

  if(!m_instance)
  {
    ACE_NEW_NORETURN(m_instance, DhcpServerStateDiscover());
  }

  return(m_instance);
}

DhcpServerStateDiscover::DhcpServerStateDiscover()
{
  ACE_TRACE("DhcpServerStateDiscover::DhcpServerStateDiscover\n");
  ACE_CString desc("DhcpServerStateDiscover");
  m_description = desc;
}

DhcpServerStateDiscover::~DhcpServerStateDiscover()
{
  ACE_TRACE("DhcpServerStateDiscover::~DhcpServerStateDiscover\n");
  delete m_instance;
  m_instance = NULL;
}

void DhcpServerStateDiscover::onEntry(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateDiscover::onEntry\n");
}

void DhcpServerStateDiscover::onExit(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerStateDiscover::onExit\n");
}

ACE_UINT32 DhcpServerStateDiscover::offer(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateDiscover::offer\n");
  return(0);
}

/*
 * @brief This member function processes DHCP Request in this state.
 *        DHCP Request can come in this state if DHCP server is crahsed
 *        or DHCP client supports fast commit and execute/exchange just two
 *        messages REQUEST/ACK..
 * @param Reference to dhcp server instance.
 * @param received byte buffer.
 * @param length of received byte buffer.
 *
 * @return 0.
 * */
ACE_UINT32 DhcpServerStateDiscover::request(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateDiscover::request\n");
  /*Prepare Request ACK.*/
  ACE_Message_Block &mb = buildResponse(parent, in, inLen);

  ACE_CString cha((const char *)parent.ctx().chaddr(), parent.ctx().chaddrLen());
  parent.getDhcpServerUser().sendResponse(cha, (ACE_Byte *)mb.rd_ptr(), mb.length());

  delete &mb;
  /*start the leaseTimer again by kicking the state machine.*/
  parent.setState(DhcpServerStateLeaseExpire::instance());

  return(0);
}

ACE_UINT32 DhcpServerStateDiscover::requestAck(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateDiscover::offer\n");
  return(0);
}

ACE_UINT32 DhcpServerStateDiscover::leaseTO(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateDiscover::offer\n");
  return(0);
}

/*
 * @brief This member function processes DHCP Discover request in this state.
 *
 * @param Reference to dhcp server instance.
 * @param received byte buffer.
 * @param length of received byte buffer.
 *
 * @return 0.
 * */
ACE_UINT32 DhcpServerStateDiscover::discover(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerStateDiscover::discover\n");

  /*Prelare DHCP Offer Message .*/
  ACE_Message_Block &mb = buildResponse(parent, inPtr, inLen);

  ACE_CString cha((const char *)parent.ctx().chaddr(), parent.ctx().chaddrLen());
  parent.getDhcpServerUser().sendResponse(cha, (ACE_Byte *)mb.rd_ptr(), mb.length());

  delete &mb;
  /*Move to next state to process DHCP Request.*/
  parent.setState(DhcpServerStateRequest::instance());
  return(0);
}

/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerStateDiscover::guardTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateDiscover::guardTimerExpiry\n");
  return(0);
}

/*lease Timer is started when IP address assignment is done successfully.*/
ACE_UINT32 DhcpServerStateDiscover::leaseTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerStateDiscover::leaseTimerExpiry\n");
  return(0);

}


#endif /*__DHCP_SERVER_STATE_DISCOVER_CC__*/
