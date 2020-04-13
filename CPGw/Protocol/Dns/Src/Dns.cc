#ifndef __DNS_CC__
#define __DNS_CC__

#include "ace/Message_Block.h"
#include "CommonIF.h"
#include "Dns.h"

DNS::CPGwDns::CPGwDns(CPGateway *parent, ACE_CString mac,
                      ACE_CString hName, ACE_CString dName,
                      ACE_UINT32 ip)
{
  m_parent = parent;
  macAddr(mac);
  hostName(hName);
  domainName(dName);
  ipAddr(ip);
}

DNS::CPGwDns::~CPGwDns()
{
}

ACE_CString &DNS::CPGwDns::hostName(void)
{
  return(m_hostName);
}

void DNS::CPGwDns::hostName(ACE_CString hName)
{
  m_hostName = hName;
}

ACE_CString &DNS::CPGwDns::domainName(void)
{
  return(m_domainName);
}

void DNS::CPGwDns::domainName(ACE_CString dName)
{
  m_domainName = dName;
}

ACE_UINT32 DNS::CPGwDns::ipAddr(void)
{
  return(m_ipAddr);
}

void DNS::CPGwDns::ipAddr(ACE_UINT32 ip)
{
  m_ipAddr = ip;
}

ACE_CString &DNS::CPGwDns::macAddr(void)
{
  return(m_macAddress);
}

void DNS::CPGwDns::macAddr(ACE_CString mac)
{
  m_macAddress = mac;
}

void DNS::CPGwDns::processQuery(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen, ACE_UINT16 qdcount)
{
  ACE_Byte *qData = (ACE_Byte *)&in[sizeof(TransportIF::ETH) +
                                    sizeof(TransportIF::IP)  +
                                    sizeof(TransportIF::UDP) +
                                    sizeof(TransportIF::DNS)];
  ACE_Byte len = 0;
  ACE_Byte offset = 0;

  while(qdcount)
  {
    len = qData[offset++];

    DNS::QData *data = NULL;
    ACE_NEW_NORETURN(data, DNS::QData());

    while(len)
    {
      DNS::QHdr *qHdr = NULL;
      ACE_NEW_NORETURN(qHdr, DNS::QHdr());

      qHdr->len(len);
      qHdr->value((ACE_Byte *)&qData[offset]);

      /*remember it into list (which is STACK) for later use.*/
      data->m_qHdrList.push(qHdr);
      offset += len;

      /*Now update the length.*/
      len = qData[offset++];
    }

    data->qtype(ntohs(*((ACE_UINT16 *)&qData[offset])));
    offset += 2;
    data->qclass(ntohs(*((ACE_UINT16 *)&qData[offset])));
    offset += 2;
    m_qDataList.push(data);
    /*Process next query data.*/
    qdcount--;
  }
}

ACE_UINT32 DNS::CPGwDns::processRequest(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  TransportIF::DNS *dnsHdr = (TransportIF::DNS *)&in[sizeof(TransportIF::ETH) +
                                                     sizeof(TransportIF::IP)  +
                                                     sizeof(TransportIF::UDP)];

  if(dnsHdr && (DNS::QUERY == dnsHdr->opcode))
  {
    if(dnsHdr->qdcount)
    {
      /*This is DNS Query, Process it.*/
      processQuery(parent, in, inLen, dnsHdr->qdcount);
    }

    if(dnsHdr->ancount)
    {
    }

    if(dnsHdr->nscount)
    {
    }

    if(dnsHdr->arcount)
    {
    }
  }

  return(0);
}

ACE_Message_Block *DNS::CPGwDns::buildResponse(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  ACE_Message_Block *mb = NULL;

  ACE_NEW_NORETURN(mb, ACE_Message_Block(512));

  return(mb);
}
#endif /*__DNS_CC__*/
