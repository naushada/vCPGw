#ifndef __DHCP_SERVER_STATE_CC__
#define __DHCP_SERVER_STATE_CC__

#include "ace/Log_Msg.h"

#include "DhcpServerState.h"
#include "DhcpCommon.h"
#include "DhcpServer.h"

DhcpServerState::DhcpServerState()
{
  ACE_TRACE("DhcpServerState::DhcpServerState\n");
  ACE_CString desc("DhcpServerState");
  m_description = desc;
}

DhcpServerState::~DhcpServerState()
{
  ACE_TRACE("DhcpServerState::~DhcpServerState\n");
}

void DhcpServerState::onEntry(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerState::onEntry\n");
}

void DhcpServerState::onExit(DHCP::Server &parent)
{
  ACE_TRACE("DhcpServerState::onExit\n");
}

/*DHCP Offer */
ACE_UINT32 DhcpServerState::offer(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerState::offer\n");
  return(0);
}

/*DHCP Discover */
ACE_UINT32 DhcpServerState::discover(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerState::discover\n");
  return(0);
}

/*DHCP Request */
ACE_UINT32 DhcpServerState::request(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerState::request\n");
  return(0);
}

/*DHCP Request Ack */
ACE_UINT32 DhcpServerState::requestAck(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerState::requestAck\n");
  return(0);
}

/*Lease Time Out */
ACE_UINT32 DhcpServerState::leaseTO(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerState::leaseTO\n");
  return(0);
}

/*Release  */
ACE_UINT32 DhcpServerState::release(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerState::release\n");
  return(0);
}

/*NACK  */
ACE_UINT32 DhcpServerState::nack(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerState::nack\n");
  return(0);
}

/*NACK  */
ACE_UINT32 DhcpServerState::decline(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerState::nack\n");
  return(0);
}

ACE_UINT16 DhcpServerState::chksumIP(void *in, ACE_UINT32 inLen)
{
  ACE_UINT32 sum = 0;
  const ACE_UINT16 *ip = (ACE_UINT16 *)in;

  while(inLen > 1)
  {
    sum += *ip++;

    if(sum & 0x80000000)
    {
      sum = (sum & 0xFFFF) + (sum >> 16);
    }

    inLen -= 2;
  }

  if(inLen)
  {
    sum += (ACE_UINT32) *(ACE_Byte *)ip;
  }

  /*wrapping up into 2 bytes.*/
  while(sum >> 16)
  {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  /*1's complement.*/
  return(~sum);
}

ACE_UINT16 DhcpServerState::chksumUDP(TransportIF::IP *ip)
{
  ACE_Byte *pseudoPtr = NULL;
  ACE_UINT16 ipHdrLen = 0;
  ACE_UINT16 tmpLen = 0;
  ACE_UINT16 offset = 0;
  ACE_UINT16 chksum = 0;

  ipHdrLen = ip->len * 4;
  tmpLen = ntohs(ip->tot_len) - ipHdrLen + 12;
  ACE_NEW_NORETURN(pseudoPtr, ACE_Byte(tmpLen));

  ACE_OS::memset((void *)pseudoPtr, 0, tmpLen);
  *((ACE_UINT32 *)&pseudoPtr[offset]) = ip->src_ip;
  offset += 4;
  *((ACE_UINT32 *)&pseudoPtr[offset]) = ip->dest_ip;
  offset += 4;
  /*reserved Byte*/
  pseudoPtr[offset] = 0;
  offset += 1;
  /*Protocol UDP*/
  pseudoPtr[offset] = 17;
  offset += 1;
  /*length of UDP Header + Payload.*/
  pseudoPtr[offset] = htons(ip->tot_len - ipHdrLen);
  offset += 2;

  ACE_OS::memcpy((void *)&pseudoPtr[offset],
                 (const void *)&pseudoPtr[ipHdrLen],
                 ntohs(ip->tot_len - ipHdrLen));

  offset += ntohs(ip->tot_len - ipHdrLen);
  chksum = chksumIP((void *)pseudoPtr, offset);
  delete pseudoPtr;

  return(chksum);
}

ACE_Message_Block &DhcpServerState::buildResponse(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 len)
{
  ACE_Message_Block *mb = NULL;
  ACE_UINT32 offset = 0;
  ACE_Byte *rsp = NULL;

  ACE_NEW_NORETURN(mb, ACE_Message_Block(2048));

  rsp = (ACE_Byte *)mb->wr_ptr();

  TransportIF::ETH *eth = (TransportIF::ETH *)rsp;
  TransportIF::IP *ip = (TransportIF::IP *)&rsp[sizeof(TransportIF::ETH)];

  TransportIF::UDP *udp = (TransportIF::UDP *)&rsp[sizeof(TransportIF::ETH) +
                                                   sizeof(TransportIF::IP)];

  TransportIF::DHCP *dhcp = (TransportIF::DHCP *)&rsp[sizeof(TransportIF::ETH) +
                                                      sizeof(TransportIF::IP)  +
                                                      sizeof(TransportIF::UDP)];

  /*Populating Ethernet Header.*/
  ACE_OS::memcpy((void *)eth->src,
                 (const void *)parent.getMacAddress().c_str() ,
                 TransportIF::ETH_ALEN);

  ACE_OS::memcpy((void *)eth->dest, (const void *)&in[TransportIF::ETH_ALEN], TransportIF::ETH_ALEN);

  eth->proto = htons(TransportIF::ETH_P_IP);
  mb->wr_ptr(sizeof(TransportIF::ETH));

  /*Populating IP Header.*/
  ip->len = 0x5;
  ip->ver = 0x4;
  ip->tos = 0x00;
  /*length shall be updated later. header length + Payload Len.*/
  ip->tot_len = 0x0000;
  ip->id = htons(0x00);
  /*bit0 -R(Reserved), bit1 - DF(Don't Fragment). bit2 - MF(More Fragment)*/
  ip->flags = htons(1 << 14);
  /*Maximum number of hops.*/
  ip->ttl = 0x10;
  /*1 - ICMP, 2 - IGMP, 6 - TCP, 17 - UDP*/
  ip->proto = 0x11;
  ip->chksum = 0x0000;

  struct in_addr addr;
  inet_aton(parent.getMyIP().c_str(), &addr);

  ip->src_ip = addr.s_addr;
  ip->dest_ip = 0x00;

  /*Populating UDP Header.*/
  udp->src_port = htons(67);
  udp->dest_port = htons(68);
  udp->len = 0x0000;
  udp->chksum = 0x0000;

  /*Populating DHCP Header.*/

  /*Boot Reply.*/
  dhcp->op = 0x02;
  /*ETHERNET_10Mb*/
  dhcp->htype = 0x1;
  dhcp->hlen = parent.ctx().chaddrLen();
  /*decremented at every router and is discarded when it becomes 0*/
  dhcp->hops = 0x5;
  dhcp->xid = parent.ctx().xid();
  dhcp->secs = 0x00;
  dhcp->flags = 0x00;
  /*This will be filled by DHCP Client.*/
  dhcp->ciaddr = 0x00;
  /*Assigining IP Address to DHCP Client.*/
  dhcp->yiaddr = 0x00;
  dhcp->giaddr = 0x00;

  ACE_OS::memcpy((void *)dhcp->chaddr,
                 (const void *)parent.ctx().chaddr(),
                 (size_t)parent.ctx().chaddrLen());

  /*Fill DHCP Server Host Name.*/

  /*Filling up the dhcp option.*/
  offset = sizeof(TransportIF::ETH) + sizeof(TransportIF::IP) +
           sizeof(TransportIF::UDP) + sizeof(TransportIF::DHCP);

  ACE_Byte cookie[] = {0x63, 0x82, 0x53, 0x63};
  ACE_OS::memcpy((void *)&rsp[offset], cookie, TransportIF::DHCP_COOKIE_LEN);
  offset += TransportIF::DHCP_COOKIE_LEN;

  RFC2131::DhcpOption *elm = NULL;
  ACE_UINT8 tag = RFC2131::OPTION_MESSAGE_TYPE;

  rsp[offset++] = RFC2131::OPTION_MESSAGE_TYPE;
  /*length of one byte.*/
  rsp[offset++] = 1;

  if(parent.optionMap().find(tag, elm) != -1)
  {
    /*Found the Message Type.*/
    if(RFC2131::DISCOVER == elm->m_value[0])
    {
      rsp[offset++] = RFC2131::OFFER;
    }
    else if(RFC2131::REQUEST == elm->m_value[0])
    {
      rsp[offset++] = RFC2131::ACK;
      /*Update DHCP Client IP Address Now.*/
      ip->dest_ip = htonl(parent.ipAddr());
    }
  }

  /*Parameter list.*/
  tag = RFC2131::OPTION_PARAMETER_REQUEST_LIST;
  elm = NULL;
  if(parent.optionMap().find(tag, elm) != -1)
  {
    ACE_UINT32 idx = 0;
    ACE_DEBUG((LM_DEBUG, "%I length of DHCP Param List is %u\n", elm->m_len));

    for(idx = 0; idx < elm->m_len; idx++)
    {
      switch(elm->m_value[idx])
      {
      case RFC2131::OPTION_SUBNET_MASK:
        rsp[offset++] = RFC2131::OPTION_SUBNET_MASK;
        rsp[offset++] = 4;
        *((ACE_UINT32 *)&rsp[offset]) = htonl(0xFFFFFF00);
        offset += 4;
        break;

      case RFC2131::OPTION_ROUTER:
        rsp[offset++] = RFC2131::OPTION_ROUTER;
        rsp[offset++] = 4;
        *((ACE_UINT32 *)&rsp[offset]) = htonl(0x01020304);
        offset += 4;
        break;

      case RFC2131::OPTION_TIME_SERVER:
        rsp[offset++] = RFC2131::OPTION_TIME_SERVER;
        rsp[offset++] = 4;
        *((ACE_UINT32 *)&rsp[offset]) = htonl(0x01020304);
        offset += 4;
        break;

      case RFC2131::OPTION_NAME_SERVER:
        rsp[offset++] = RFC2131::OPTION_NAME_SERVER;
        rsp[offset++] = 4;
        *((ACE_UINT32 *)&rsp[offset]) = htonl(0x08080808);
        offset += 4;
        break;

      case RFC2131::OPTION_HOST_NAME:
        rsp[offset++] = RFC2131::OPTION_HOST_NAME;
        rsp[offset++] = parent.hostName().length();
          /*Host Machine Name to be updated.*/;
        ACE_OS::memcpy((void *)&rsp[offset], parent.hostName().c_str(),
                       parent.hostName().length());
        offset += parent.hostName().length();
        break;

      case RFC2131::OPTION_DOMAIN_NAME:
        rsp[offset++] = RFC2131::OPTION_DOMAIN_NAME;
        rsp[offset++] = parent.domainName().length();
          /*Host Machine Name to be updated.*/;
        ACE_OS::memcpy((void *)&rsp[offset], parent.domainName().c_str(),
                       parent.domainName().length());
        offset += parent.domainName().length();
        break;

      case RFC2131::OPTION_MTU:
        rsp[offset++] = RFC2131::OPTION_MTU;
        rsp[offset++] = 2;
          /*Host Machine Name to be updated.*/;
        *((ACE_UINT32 *)&rsp[offset]) = htonl(0x00);
        offset += 4;
        break;

      case RFC2131::OPTION_BROADCAST_ADDRESS:
        rsp[offset++] = RFC2131::OPTION_BROADCAST_ADDRESS;
        rsp[offset++] = 0;
          /*Host Machine Name to be updated.*/;
        *((ACE_UINT32 *)&rsp[offset]) = htonl(0x00);
        offset += 4;
        break;

      case RFC2131::OPTION_NIS_DOMAIN:
        rsp[offset++] = RFC2131::OPTION_NIS_DOMAIN;
        rsp[offset++] = 0;
          /*Host Machine Name to be updated.*/;
        *((ACE_UINT32 *)&rsp[offset]) = htonl(0x00);
        offset += 4;
        break;

      case RFC2131::OPTION_NIS:
        rsp[offset++] = RFC2131::OPTION_NIS;
        rsp[offset++] = 0;
          /*Host Machine Name to be updated.*/;
        *((ACE_UINT32 *)&rsp[offset]) = htonl(0x00);
        offset += 4;
        break;

      case RFC2131::OPTION_NTP_SERVER:
        rsp[offset++] = RFC2131::OPTION_NTP_SERVER;
        rsp[offset++] = 0;
          /*Host Machine Name to be updated.*/;
        *((ACE_UINT32 *)&rsp[offset]) = htonl(0x00);
        offset += 4;
        break;

      case RFC2131::OPTION_REQUESTED_IP_ADDRESS:
        rsp[offset++] = RFC2131::OPTION_REQUESTED_IP_ADDRESS;
        rsp[offset++] = 0;
          /*Host Machine Name to be updated.*/;
        *((ACE_UINT32 *)&rsp[offset]) = htonl(0x00);
        offset += 4;
        break;

      case RFC2131::OPTION_IP_LEASE_TIME:
        rsp[offset++] = RFC2131::OPTION_IP_LEASE_TIME;
        rsp[offset++] = 0;
          /*Host Machine Name to be updated.*/;
        *((ACE_UINT32 *)&rsp[offset]) = htonl(0x00);
        offset += 4;
        break;

      case RFC2131::OPTION_OVERLOAD:
        rsp[offset++] = RFC2131::OPTION_OVERLOAD;
        rsp[offset++] = 0;
          /*Host Machine Name to be updated.*/;
        *((ACE_UINT32 *)&rsp[offset]) = htonl(0x00);
        offset += 4;
        break;

      case RFC2131::OPTION_SERVER_IDENTIFIER:
        rsp[offset++] = RFC2131::OPTION_SERVER_IDENTIFIER;
        rsp[offset++] = 0;
          /*Host Machine Name to be updated.*/;
        *((ACE_UINT32 *)&rsp[offset]) = htonl(0x00);
        offset += 4;
        break;

      default:
        break;
      }
    }
  }

  rsp[offset++] = RFC2131::OPTION_IP_LEASE_TIME;
  rsp[offset++] = 4;
  *((ACE_UINT32 *)&rsp[offset]) = htonl(parent.lease());
  offset += 4;

  rsp[offset++] = RFC2131::OPTION_MTU;
  rsp[offset++] = 2;
  *((ACE_UINT16 *)&rsp[offset]) = htons(0x00);
  offset += 2;

  rsp[offset++] = RFC2131::OPTION_SERVER_IDENTIFIER;
  rsp[offset++] = parent.getMacAddress().length();
  ACE_OS::memcpy((void *)&rsp[offset], parent.getMacAddress().c_str(),
                 parent.getMacAddress().length());

  rsp[offset++] = RFC2131::OPTION_END;

  /*Update the lentgh of IP and UDP header now.*/
  ip->tot_len = htons(offset - sizeof(TransportIF::ETH));
  udp->len = htons(offset - (sizeof(TransportIF::ETH) + sizeof(TransportIF::IP)));

  /*calculate the checksum now.*/
  ip->chksum = chksumIP(ip, (sizeof(ACE_UINT32) * ip->len));
  udp->chksum = chksumUDP(ip);
  /*Update the length*/
  mb->wr_ptr(offset);

  return(*mb);
}

/*Populate DHCP Options.*/
ACE_UINT32 DhcpServerState::populateDhcpOption(DHCP::Server &parent, ACE_Byte *dhcpOption,
                                               ACE_UINT32 optionLen)
{
  ACE_TRACE("DhcpServerState::populateDhcpOption\n");

  ACE_UINT32 offset = 0;

  while(optionLen > 0)
  {
    switch(dhcpOption[offset])
    {
    case RFC2131::OPTION_END:
      optionLen = 0;
      break;
    default:

      RFC2131::DhcpOption *elm = NULL;
      ACE_UINT8 tag = dhcpOption[offset];

      if(parent.optionMap().find(tag, elm) == -1)
      {
        /*Not found in the MAP.*/
        ACE_NEW_NORETURN(elm, RFC2131::DhcpOption());

        elm->m_tag = dhcpOption[offset++];
        elm->m_len = dhcpOption[offset++];
        ACE_OS::memcpy((void *)elm->m_value, (const void *)&dhcpOption[offset], elm->m_len);

        offset += elm->m_len;
        /*Add it into MAP now.*/
        parent.optionMap().bind(elm->m_tag, elm);
      }
      else
      {
        /*Found in the Map , Update with new contents received now.*/
        elm->m_tag = dhcpOption[offset++];
        elm->m_len = dhcpOption[offset++];

        ACE_OS::memcpy((void *)elm->m_value, (const void *)&dhcpOption[offset], elm->m_len);

        offset += elm->m_len;
      }
    }
  }

  return(0);
}

/*Populating DHCP Header*/
ACE_UINT32 DhcpServerState::populateDhcpHeader(DHCP::Server &parent, TransportIF::DHCP *dhcpHeader)
{
  ACE_TRACE("DhcpServerState::populateDhcpHeader\n");
  parent.ctx().xid(dhcpHeader->xid);
  parent.ctx().ciaddr(dhcpHeader->ciaddr);
  parent.ctx().yiaddr(dhcpHeader->yiaddr);
  parent.ctx().siaddr(dhcpHeader->siaddr);
  parent.ctx().giaddr(dhcpHeader->giaddr);

  parent.ctx().chaddrLen(dhcpHeader->hlen);
  parent.ctx().chaddr(dhcpHeader->chaddr);

  /*dhcp client's host name.*/
  parent.ctx().sname(dhcpHeader->sname);

  return(0);
}

/*RX*/
ACE_UINT32 DhcpServerState::rx(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("DhcpServerState::rx\n");

  TransportIF::DHCP *dhcpHdr = (TransportIF::DHCP *)&in[sizeof(TransportIF::ETH) +
                                                        sizeof(TransportIF::IP)  +
                                                        sizeof(TransportIF::UDP)];
  /*populate dhcp header into internal Context.*/
  populateDhcpHeader(parent, dhcpHdr);

  TransportIF::UDP *udpHdr = (TransportIF::UDP *)&in[sizeof(TransportIF::ETH) +
                                                     sizeof(TransportIF::IP)];
  /*Number of bytes in dhcpOptional Payload.*/
  ACE_UINT32 dhcpOptionLen = ntohs(udpHdr->len) - sizeof(TransportIF::DHCP);

  /*dhcpOption Offset.*/
  ACE_Byte *dhcpOption = (ACE_Byte *)&in[sizeof(TransportIF::ETH)  +
                                         sizeof(TransportIF::IP)   +
                                         sizeof(TransportIF::UDP)  +
                                         sizeof(TransportIF::DHCP) +
                                         TransportIF::DHCP_COOKIE_LEN];

  /*populate dhcp option into Hash Map.*/
  populateDhcpOption(parent, dhcpOption, dhcpOptionLen);

  ACE_UINT8 msgType = 0;
  RFC2131::DhcpOption *elm = NULL;

  if(parent.optionMap().find(RFC2131::OPTION_MESSAGE_TYPE, elm) != -1)
  {
    ACE_Byte val[8];
    ACE_Byte len = 0;
    len = elm->getValue(val);
    msgType =(ACE_UINT8)val[0];
    ACE_DEBUG((LM_DEBUG, "messageType tag(%u) is found\n", msgType));
  }

  switch(msgType)
  {
    case RFC2131::DISCOVER:
      /*Kick the state Machine.*/
      parent.getState().discover(parent, in, inLen);
      break;
    case RFC2131::REQUEST:
      /*Kick the state Machine.*/
      parent.getState().request(parent, in, inLen);
      break;
    case RFC2131::RELEASE:
      /*Kick the state Machine.*/
      parent.getState().release(parent, in, inLen);
      break;
    case RFC2131::DECLINE:
      /*Kick the state Machine.*/
      parent.getState().decline(parent, in, inLen);
      break;
    case RFC2131::NACK:
      /*Kick the state Machine.*/
      parent.getState().nack(parent, in, inLen);
      break;
    default:
      ACE_ERROR((LM_ERROR, "%I the DHCP opcode %u is not supported\n", msgType));
      break;
  }

  return(0);
}


/*Guard Timer is stared if next request is expected to complete the Flow.*/
ACE_UINT32 DhcpServerState::guardTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerState::guardTimerExpiry\n");
  return(0);
}

/*lease Timer is started when IP address assignment is done successfully.*/
ACE_UINT32 DhcpServerState::leaseTimerExpiry(DHCP::Server &parent, const void *act)
{
  ACE_TRACE("DhcpServerState::leaseTimerExpiry\n");
  return(0);
}

#endif /*__DHCP_SERVER_STATE_CC__*/
