#ifndef __CPGATEWAY_STATE_CC__
#define __CPGATEWAY_STATE_CC__

#include "ace/Log_Msg.h"
#include "ace/SString.h"
#include "ace/Basic_Types.h"

#include "commonIF.h"
#include "CPGateway.h"
#include "CPGatewayState.h"

//#include "DhcpServerUser.h"

CPGatewayState::CPGatewayState()
{
  ACE_TRACE("CPGatewayState::CPGatewayState\n");

  ACE_CString dsc("CPGatewayState");
  description(dsc);
}

CPGatewayState::~CPGatewayState()
{
  ACE_TRACE("CPGatewayState::~CPGatewayState\n");
}

ACE_UINT32 CPGatewayState::processRequest(CPGateway &parent,
                                          ACE_Byte *in,
                                          ACE_UINT32 inLen)
{
  ACE_TRACE("CPGatewayState::processRequest\n");

  ACE_DEBUG((LM_DEBUG, "CPGatewayState::processRequest len %u\n", inLen));

  for(int idx = 0; idx < inLen; idx++)
  {
    if(!(idx%16))
      ACE_DEBUG((LM_DEBUG, "\n"));

    ACE_DEBUG((LM_DEBUG, "%0.2X ", in[idx] & 0xFF));
  }

  TransportIF::ETH *ethHdr = (TransportIF::ETH *)in;

  if(TransportIF::ETH_P_IP == ntohs(ethHdr->proto))
  {
    /*Ethernet Header is followed by IP Packet.*/
    TransportIF::IP *ipHdr = (TransportIF::IP *)&in[sizeof(TransportIF::ETH)];
    if(TransportIF::IP_UDP == ipHdr->proto)
    {
      /*IP Packet is followed by UDP Payload.*/
      TransportIF::UDP *udpHdr = (TransportIF::UDP *)&in[sizeof(TransportIF::ETH) +
                                                         sizeof(TransportIF::IP)];

      if((TransportIF::DHCP_CLIENT_PORT == ntohs(udpHdr->src_port)) &&
         (TransportIF::DHCP_SERVER_PORT == ntohs(udpHdr->dest_port)))
      {
        parent.getDhcpServerUser().processRequest(in, inLen);
      }
      //else if()
      {
        /*DNS Packet.*/
      }
    }
    else if(TransportIF::IP_TCP == ipHdr->proto)
    {
      /*TCP Packet*/
    }
  }
  else if(TransportIF::ETH_P_ARP == ntohs(ethHdr->proto))
  {
    /*ARP Packet for MAC resolve.*/
  }
  else if(TransportIF::ETH_P_EAPOL == ntohs(ethHdr->proto))
  {
    /*EAP Over Lan - First Authnticate and the Assign IP address.*/
  }
  else if(TransportIF::ETH_P_PPP == ntohs(ethHdr->proto))
  {
  }
  else if(TransportIF::ETH_P_8021Q == ntohs(ethHdr->proto))
  {
  }
  else if(ntohs(ethHdr->proto) <= 1500)
  {
    /*802.3 llc frame.*/
  }

  return(0);
}

ACE_UINT32 CPGatewayState::lock(CPGateway &parent)
{
  ACE_TRACE("CPGatewayState::lock\n");
  return(0);
}

ACE_UINT32 CPGatewayState::unlock(CPGateway &parent)
{
  ACE_TRACE("CPGatewayState::unlock\n");
  return(0);
}

void CPGatewayState::onEntry(CPGateway &parent)
{
  ACE_TRACE("CPGatewayState::onEntry\n");
}

void CPGatewayState::onExit(CPGateway &parent)
{
  ACE_TRACE("CPGatewayState::onExit\n");
}

ACE_CString CPGatewayState::description(void)
{
  ACE_TRACE("CPGatewayState::description\n");
  return(m_description);
}

void CPGatewayState::description(ACE_CString dsc)
{
  ACE_TRACE("CPGatewayState::description\n");
  m_description = dsc;
}

#endif /*__CPGATEWAY_STATE_CC__*/
