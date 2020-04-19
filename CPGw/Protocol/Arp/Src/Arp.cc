#ifndef __ARP_CC__
#define __ARP_CC__

#include "Arp.h"
#include "CPGateway.h"

ARP::CPGwArp::CPGwArp(CPGateway *parent, ACE_CString mac)
{
  ACE_TRACE("ARP::CPGwArp::CPGwArp\n");

  m_parent = parent;
  m_maccAddress = mac;
}

ARP::CPGwArp::~CPGwArp()
{
  ACE_TRACE("ARP::CPGwArp::~CPGwArp\n");
}

ACE_UINT32 ARP::CPGwArp::processRequest(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("ARP::CPGwArp::processRequest\n");

  ACE_Message_Block *mb = NULL;
  ACE_Byte bmac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  TransportIF::ETH *ethHdr = (TransportIF::ETH *)in;

  if(!ACE_OS::memcmp((void *)ethHdr->dest, (const void *)bmac, TransportIF::ETH_ALEN))
  {
    TransportIF::ARP *arpReq = (TransportIF::ARP *)&in[sizeof(TransportIF::ETH)];

    if(!ntohl(arpReq->sender_ip))
    {
      /*Sender is ensuring it's IP address assignment.*/
      return(1);
    }

    if(ntohl(arpReq->sender_ip) == ntohl(arpReq->target_ip))
    {
      /*This Gratuitos ARP Request.*/
      return(1);
    }

    if((TransportIF::ARP_PROTO_IPv4 ==  ntohs(arpReq->proto)) &&
       (TransportIF::ARPOP_REQUEST == arpReq->opcode) &&
       (TransportIF::ETH_ALEN == arpReq->hlen))
    {
      mb = buildResponse(parent, in, inLen);
      parent.sendResponse(getMac(), (ACE_Byte *)mb->rd_ptr(), mb->length());
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l ARP Response of length %u sent\n"), mb->length()));
      mb->release();
      return(0);
    }

  }

  return(1);
}

ACE_Message_Block *ARP::CPGwArp::buildResponse(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_TRACE("ARP::CPGwArp::buildResponse\n");
  ACE_Message_Block *mb = NULL;

  ACE_NEW_NORETURN(mb, ACE_Message_Block(256));

  /*Response ARP Pointer.*/
  TransportIF::ETH *rspEthHdr = (TransportIF::ETH *)mb->wr_ptr();
  TransportIF::ARP *rspArpHdr = (TransportIF::ARP *)&mb->wr_ptr()[sizeof(TransportIF::ETH)];

  /*Request ARP Pointer.*/
  TransportIF::ETH *reqEthHdr = (TransportIF::ETH *)in;
  TransportIF::ARP *reqArpHdr = (TransportIF::ARP *)&in[sizeof(TransportIF::ETH)];

  /*building Ethernet Header.*/
  ACE_OS::memcpy((void *)rspEthHdr->dest, (const void *)reqEthHdr->src, TransportIF::ETH_ALEN);
  ACE_OS::memcpy((void *)rspEthHdr->src, (const void *)getMac().c_str(), TransportIF::ETH_ALEN);
  rspEthHdr->proto = reqEthHdr->proto;

  /*building ARP Header.*/
  rspArpHdr->hdr = reqArpHdr->hdr;
  rspArpHdr->proto = reqArpHdr->proto;
  rspArpHdr->hlen = reqArpHdr->hlen;
  rspArpHdr->plen = reqArpHdr->plen;
  rspArpHdr->opcode = htons(TransportIF::ARPOP_REPLY);

  ACE_OS::memcpy((void *)rspArpHdr->sender_ha, (const void *)getMac().c_str(), TransportIF::ETH_ALEN);
  ACE_OS::memcpy((void *)rspArpHdr->target_ha, (const void *)reqArpHdr->sender_ha, TransportIF::ETH_ALEN);

  rspArpHdr->sender_ip = reqArpHdr->target_ip;
  rspArpHdr->target_ip = reqArpHdr->sender_ip;

  /*Remember Peer IP and MAC.*/
  peerIp(ntohs(reqArpHdr->sender_ip));
  ACE_CString mac((const char *)reqArpHdr->sender_ha, reqArpHdr->hlen);
  peerMac(mac);

  mb->wr_ptr(sizeof(TransportIF::ETH) + sizeof(TransportIF::ARP));
  return(mb);
}

ACE_CString &ARP::CPGwArp::getMac(void)
{
  return(m_maccAddress);
}

ACE_UINT32 ARP::CPGwArp::peerIp(void)
{
  return(m_peerIp);
}

void ARP::CPGwArp::peerIp(ACE_UINT32 ip)
{
  m_peerIp = ip;
}

ACE_CString &ARP::CPGwArp::peerMac(void)
{
  return(m_peerMac);
}

void ARP::CPGwArp::peerMac(ACE_CString mac)
{
  m_peerMac = mac;
}


#endif /*__ARP_CC__*/
