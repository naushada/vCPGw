#ifndef __DNS_CC__
#define __DNS_CC__

#include "ace/Message_Block.h"
#include "CommonIF.h"
#include "Dns.h"
#include "DhcpServerUser.h"
#include "CPGateway.h"

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

void DNS::CPGwDns::processDnsQury(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  DNS::QData **elm = NULL;
  DNS::QHdr **label = NULL;

  ACE_Unbounded_Stack_Iterator<DNS::QData *> iter(m_qDataList);
  for(iter.first(); !iter.done(); iter.advance())
  {
    iter.next(elm);
    DNS::QHdr *qData[2];

    if(!(*elm)->m_qHdrList.pop(qData[0]) &&
       !(*elm)->m_qHdrList.pop(qData[1]))
    {
      ACE_TCHAR dName[255];
      ACE_OS::snprintf(dName, sizeof(dName), "%s.%s",
                       qData[0]->value(),
                       qData[1]->value());

      ACE_DEBUG((LM_DEBUG, "The domain Name is %s\n", dName));

      if(!ACE_OS::strncmp((const ACE_TCHAR *)domainName().c_str(),
                          (const ACE_TCHAR *)dName,
                          domainName().length()))
      {
        /*Internal DNS Request.*/
        /*re-claim the memory now.*/
        delete qData[0];
        delete qData[1];
        ACE_DEBUG((LM_DEBUG, "Domain Name is matched %s\n", domainName().c_str()));
      }
    }

    ACE_Unbounded_Stack_Iterator<DNS::QHdr *> inIter((*elm)->m_qHdrList);
    /*Now the stack size would have been reduced.*/
    /*Find-out the host name.*/
    ACE_TCHAR hName[255];
    int hostLen = 0;
    ACE_OS::memset((void *)hName, '.', sizeof(hName));

    for(inIter.first(); !inIter.done(); inIter.advance())
    {
      inIter.next(label);
      hostLen += ACE_OS::snprintf(&hName[hostLen], (sizeof(hName) - hostLen),
                                  "%s", (*label)->value());
      delete(*label);
    }

    ACE_CString hh((const ACE_TCHAR *)hName, hostLen);
    /*Find in Hash Map to get the IP.*/
    ACE_TCHAR *IP = parent.getDhcpServerUser().getResolverIP(hh);
    if(!IP)
    {
      /*The Host is not controled by CPGateway.*/
    }
    else
    {
      /*IP is maintained by CPGateway.*/
    }
  }
}

void DNS::CPGwDns::processQdcount(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen, ACE_UINT16 qdcount)
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

  processDnsQury(parent, in, inLen);
}

void DNS::CPGwDns::processAncount(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen, ACE_UINT16 ancount)
{
}

void DNS::CPGwDns::processNscount(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen, ACE_UINT16 nscount)
{
}

void DNS::CPGwDns::processArcount(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen, ACE_UINT16 arcount)
{
}



ACE_UINT32 DNS::CPGwDns::processRequest(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  TransportIF::DNS *dnsHdr = (TransportIF::DNS *)&in[sizeof(TransportIF::ETH) +
                                                     sizeof(TransportIF::IP)  +
                                                     sizeof(TransportIF::UDP)];

  if(dnsHdr && (DNS::QUERY == dnsHdr->opcode))
  {
    if(ntohs(dnsHdr->qdcount))
    {
      /*This is DNS Query, Process it.*/
      processQdcount(parent, in, inLen, dnsHdr->qdcount);
    }

    if(ntohs(dnsHdr->ancount))
    {
      processAncount(parent, in, inLen, dnsHdr->ancount);
    }

    if(ntohs(dnsHdr->nscount))
    {
      processNscount(parent, in, inLen, dnsHdr->nscount);
    }

    if(ntohs(dnsHdr->arcount))
    {
      processArcount(parent, in, inLen, dnsHdr->arcount);
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
