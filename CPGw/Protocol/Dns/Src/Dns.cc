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
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l domainName is %s\n"), dName.c_str()));
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

ACE_UINT16 DNS::CPGwDns::chksumIP(void *in, ACE_UINT32 inLen)
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

ACE_UINT16 DNS::CPGwDns::chksumUDP(TransportIF::IP *ip)
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


ACE_UINT32 DNS::CPGwDns::buildDnsResponse(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen, ACE_Message_Block &mb)
{
  TransportIF::ETH *reqEthHdr = (TransportIF::ETH *)in;
  TransportIF::IP *reqIPHdr = (TransportIF::IP *)&in[sizeof(TransportIF::ETH)];
  TransportIF::UDP *reqUdpHdr = (TransportIF::UDP *)&in[sizeof(TransportIF::ETH) +
                                                        sizeof(TransportIF::IP)];
  TransportIF::DNS *reqDnsHdr = (TransportIF::DNS *)&in[sizeof(TransportIF::ETH) +
                                                        sizeof(TransportIF::IP) +
                                                        sizeof(TransportIF::UDP)];
  ACE_Byte *out = (ACE_Byte *)mb.wr_ptr();

  TransportIF::ETH *rspEthHdr = (TransportIF::ETH *)out;

  TransportIF::IP *rspIPHdr = (TransportIF::IP *)&out[sizeof(TransportIF::ETH)];

  TransportIF::UDP *rspUdpHdr = (TransportIF::UDP *)&out[sizeof(TransportIF::ETH) +
                                                         sizeof(TransportIF::IP)];

  TransportIF::DNS *rspDnsHdr = (TransportIF::DNS *)&out[sizeof(TransportIF::ETH) +
                                                         sizeof(TransportIF::IP) +
                                                         sizeof(TransportIF::UDP)];

  /*Prepare Ethernet MAC Header.*/
  ACE_OS::memcpy((void *)rspEthHdr->dest, reqEthHdr->src, TransportIF::ETH_ALEN);
  ACE_OS::memcpy((void *)rspEthHdr->src, reqEthHdr->dest, TransportIF::ETH_ALEN);
  rspEthHdr->proto = reqEthHdr->proto;

  /*Prepare IP Header.*/
  ACE_OS::memcpy((void *)rspIPHdr, reqIPHdr, sizeof(TransportIF::IP));
  rspIPHdr->src_ip = reqIPHdr->dest_ip;
  rspIPHdr->dest_ip = reqIPHdr->src_ip;

  /*check sum would be calculated first.*/
  rspIPHdr->chksum = 0;
  /*This will be updated later.*/
  rspIPHdr->tot_len = 0;

  /*Prepare UDP Header.*/
  rspUdpHdr->src_port = reqUdpHdr->dest_port;
  rspUdpHdr->dest_port = reqUdpHdr->src_port;
  /*This will be calculated later.*/
  rspUdpHdr->len = 0;
  rspUdpHdr->chksum = 0;

  /*Prepare DNS Header.*/
  rspDnsHdr->xid = reqDnsHdr->xid;
  rspDnsHdr->rd = reqDnsHdr->rd;
  rspDnsHdr->tc = 0;
  rspDnsHdr->aa = 1;
  rspDnsHdr->opcode = 0;
  rspDnsHdr->qr = 1;
  rspDnsHdr->rcode = DNS::RCODE_NO_ERROR;
  rspDnsHdr->z = 0;
  rspDnsHdr->ra = 0;

  /*This will be updated later.*/
  rspDnsHdr->qdcount = 0;
  rspDnsHdr->ancount = htons(2);
  rspDnsHdr->nscount = htons(1);
  rspDnsHdr->arcount = 0;

  /*Update the length.*/
  mb.wr_ptr(sizeof(TransportIF::ETH) +
            sizeof(TransportIF::IP) +
            sizeof(TransportIF::UDP) +
            sizeof(TransportIF::DNS));

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l Response Length is %u\n"), mb.length()));
  ACE_UINT8 qdcount = 0;

  /*Build Query Section.*/
  buildQDSection(qdcount, mb);
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l Response Length QDsection is %u\n"), mb.length()));

  /*Number of queries now.*/
  rspDnsHdr->qdcount = htons(qdcount);

  /*AN Section Now.*/
  /*AN(1)*/
  std::vector<ACE_CString> dName;
  std::vector<ACE_CString > hName;

  getDomainNameFromQuery(dName);

  /*Is this domain name is controlled by vCPGateway?*/
  if(domainName() != dName[0])
  {
    /*Domain is not ours.*/
    getHostNameFromQuery(hName);

    ACE_TCHAR fqdn[255];
    ACE_OS::snprintf(fqdn, sizeof(fqdn), "%s.%s",hName[0].c_str(), dName[0].c_str());
    ACE_CString fqdnStr(fqdn);
    buildRRSection(fqdnStr, htonl(ipAddr()), mb);
    buildRRSection(hostName(), htonl(ipAddr()), mb);
  }
  else
  {
    getHostNameFromQuery(hName);
    ACE_CString hh(hName[0].c_str());

    if(hh == hostName())
    {
      buildRRSection(domainName(), htonl(ipAddr()), mb);
      buildRRSection(hostName(), htonl(ipAddr()), mb);
    }
    else
    {
      ACE_UINT32 hIP = 0;
      /*Find in Hash Map to get the IP.*/
      ACE_Byte *IP = parent.getDhcpServerUser().getResolverIP(hh);
      if(!IP)
      {
        /*The Hostname is not control by CPGateway.*/
        hIP = ipAddr();
        /*Domain is ours. but host is different one.*/
        ACE_TCHAR fqdn[255];
        ACE_OS::snprintf(fqdn, sizeof(fqdn), "%s.%s",hName[0].c_str(), dName[0].c_str());
        ACE_CString fqdnStr(fqdn);
        buildRRSection(fqdnStr, htonl(ipAddr()), mb);
        buildRRSection(hostName(), htonl(ipAddr()), mb);
      }
      else
      {
        buildRRSection(dName[0], htonl(hIP), mb);
        hIP = atoi((const char *)IP);
        buildRRSection(hh, htonl(hIP), mb);
        delete IP;
      }

    }
  }

  ACE_TCHAR ns_fqdn[255];
  ACE_OS::snprintf(ns_fqdn, sizeof(ns_fqdn), "%s.%s",hostName().c_str(), domainName().c_str());
  ACE_CString nsFqdnStr(ns_fqdn);
  /*NS section Now.*/
  buildNSSection(nsFqdnStr, mb);

  /*Update IP Payload length Now.*/
  rspIPHdr->tot_len = htons(mb.length() - sizeof(TransportIF::ETH));
  /*Update the UDP Payload length Now.*/
  rspUdpHdr->len = htons(mb.length() - (sizeof(TransportIF::ETH) + sizeof(TransportIF::IP)));
  /*Calculate Check sum Now.*/

  rspIPHdr->chksum = chksumIP(rspIPHdr, (4 * rspIPHdr->len));
  //rspUdpHdr->chksum = chksumUDP(rspIPHdr);
  return(0);
}

ACE_UINT32 DNS::CPGwDns::buildANSection(ACE_CString &anStr, ACE_UINT32 ip, ACE_Message_Block &mb)
{
  ACE_UINT32 ret = 0;
  ACE_UINT8 qdcount = 0;

  ret = buildQDSection(qdcount, mb);

  /*Update IP Address.*/
  *((ACE_UINT16 *)mb.wr_ptr()) = htons(4);
  mb.wr_ptr(2);
  *((ACE_UINT32 *)mb.wr_ptr()) = htonl(ip);
  mb.wr_ptr(4);

  return(0);
}

ACE_UINT32 DNS::CPGwDns::buildQDSection(ACE_UINT8 &qdcount, ACE_Message_Block &mb)
{
  /*Outer Iterator.*/
  std::vector<DNS::QData>::iterator iter;
  /*Inner Iterator.*/
  std::vector<DNS::QHdr>::iterator inIter;

  DNS::QData elm;
  DNS::QHdr qHdr;

  for(iter = m_qDataList.begin(); iter !=m_qDataList.end(); iter++)
  {
    elm = *iter;
    for(inIter = elm.m_qHdrList.begin(); inIter != elm.m_qHdrList.end(); inIter++)
    {
      qHdr = *inIter;
      *(mb.wr_ptr()) = qHdr.len();
      mb.wr_ptr(1);
      ACE_OS::memcpy((void *)mb.wr_ptr(), qHdr.value(), qHdr.len());
      mb.wr_ptr(qHdr.len());
    }

    /*Terminate the QNAME with 0 length.*/
    *(mb.wr_ptr()) = 0;
    mb.wr_ptr(1);

    *((ACE_UINT16 *)mb.wr_ptr()) = htons(elm.qtype());
    mb.wr_ptr(2);
    *((ACE_UINT16 *)mb.wr_ptr()) = htons(elm.qclass());
    mb.wr_ptr(2);

    /*Number of query section*/
    qdcount++;
  }

  return(0);
}

ACE_UINT32 DNS::CPGwDns::buildNSSection(ACE_CString &name, ACE_Message_Block &mb)
{
  int idx;
  ACE_Byte label[128];
  ACE_UINT8 len = 0;

  for(idx = 0; idx < name.length(); idx++)
  {
    if('.' == name.c_str()[idx])
    {
      *(mb.wr_ptr()) = len;
      mb.wr_ptr(1);
      ACE_OS::memcpy((void *)mb.wr_ptr(), label, len);
      mb.wr_ptr(len);
      len = 0;
    }
    else
    {
      label[len++] = name.c_str()[idx];
    }
  }

  if(len)
  {
    *(mb.wr_ptr()) = len;
    mb.wr_ptr(1);
    ACE_OS::memcpy((void *)mb.wr_ptr(), label, len);
    mb.wr_ptr(len);
  }

  /*Terminate the QNAME with 0 as length.*/
  *(mb.wr_ptr()) = 0;
  mb.wr_ptr(1);

  *((ACE_UINT16 *)mb.wr_ptr()) = htons(DNS::RRCODE_NA);
  mb.wr_ptr(2);

  *((ACE_UINT16 *)mb.wr_ptr()) = htons(DNS::RRCLASS_IN);
  mb.wr_ptr(2);

  /*TTL - 120 seconds*/
  *((ACE_UINT32 *)mb.wr_ptr()) = htonl(0);
  mb.wr_ptr(4);

  /*RD Data Length.*/
  *((ACE_UINT16 *)mb.wr_ptr()) = htons(0);
  mb.wr_ptr(2);

  return(0);
}

ACE_UINT32 DNS::CPGwDns::buildRRSection(ACE_CString &name, ACE_UINT32 ip,
                                        ACE_Message_Block &mb)
{
  int idx;
  ACE_Byte label[128];
  ACE_UINT8 len = 0;

  for(idx = 0; idx < name.length(); idx++)
  {
    if('.' == name.c_str()[idx])
    {
      *(mb.wr_ptr()) = len;
      mb.wr_ptr(1);
      ACE_OS::memcpy((void *)mb.wr_ptr(), label, len);
      mb.wr_ptr(len);
      len = 0;
    }
    else
    {
      label[len++] = name.c_str()[idx];
    }
  }

  if(len)
  {
    *(mb.wr_ptr()) = len;
    mb.wr_ptr(1);
    ACE_OS::memcpy((void *)mb.wr_ptr(), label, len);
    mb.wr_ptr(len);
  }

  /*Terminate the QNAME with 0 as length.*/
  *(mb.wr_ptr()) = 0;
  mb.wr_ptr(1);

  *((ACE_UINT16 *)mb.wr_ptr()) = htons(DNS::RRCODE_A);
  mb.wr_ptr(2);

  *((ACE_UINT16 *)mb.wr_ptr()) = htons(DNS::RRCLASS_IN);
  mb.wr_ptr(2);

  /*TTL - 120 seconds*/
  *((ACE_UINT32 *)mb.wr_ptr()) = htonl(120);
  mb.wr_ptr(4);

  /*RD Data Length.*/
  *((ACE_UINT16 *)mb.wr_ptr()) = htons(4);
  mb.wr_ptr(2);

  /*RD Data Value.*/
  *((ACE_UINT32 *)mb.wr_ptr()) = htonl(ip);
  mb.wr_ptr(4);

  return(0);
}

void DNS::CPGwDns::getDomainNameFromQuery(std::vector<ACE_CString> &domainName)
{
  std::vector<DNS::QData>::iterator iter;
  DNS::QData elm;

  for(iter = m_qDataList.begin(); iter !=m_qDataList.end(); iter++)
  {
    DNS::QHdr qData[2];
    elm = *iter;
    qData[0] = elm.m_qHdrList[elm.m_qHdrList.size() -2];
    qData[1] = elm.m_qHdrList[elm.m_qHdrList.size() -1];

    ACE_TCHAR dName[255];
    int len = 0;

    /*format shall be abc.com/abc.net ect.*/
    len = ACE_OS::snprintf(dName, (sizeof(dName) -1), "%s.%s",
                           qData[0].value(),
                           qData[1].value());

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l The domain Name is %s\n"), dName));

    ACE_CString dd((const ACE_TCHAR *)dName, len);
    domainName.push_back(dd);
  }
}

void DNS::CPGwDns::getHostNameFromQuery(std::vector<ACE_CString> &hostName)
{
  DNS::QData elm;
  DNS::QHdr label;

  ACE_TCHAR hName[255];
  int hostLen = 0;
  ACE_OS::memset((void *)hName, 0, sizeof(hName));
  int idx;
  std::vector<DNS::QData>::iterator iter;

  for(iter = m_qDataList.begin(); iter !=m_qDataList.end(); iter++)
  {
    elm = *iter;
    /*-2 is because last two elements will be domain name of example.net*/
    for(idx = 0; idx < (int)(elm.m_qHdrList.size() - 2); idx++)
    {
      label = elm.m_qHdrList[idx];
      hostLen += ACE_OS::snprintf(&hName[hostLen], (sizeof(hName) - hostLen),
                                  "%s.", label.value());
    }

    /*get rid of last dot. */
    hName[hostLen-1] = 0;

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l The hostName is %s\n"), hName));
    ACE_CString dd(hName, hostLen);
    hostName.push_back(dd);
  }
}

void DNS::CPGwDns::processDnsQury(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen)
{
  std::vector<ACE_CString> dName;
  std::vector<ACE_CString> hName;
  ACE_Byte *IP = NULL;

  getDomainNameFromQuery(dName);
  getHostNameFromQuery(hName);

  /*Process First Quesry in qdsection.*/
  ACE_CString &dd = dName[0];
  ACE_CString &hh = hName[0];

  ACE_Message_Block *mb = NULL;

  ACE_NEW_NORETURN(mb, ACE_Message_Block(CommonIF::SIZE_1KB));


  if(hh == hostName() && dd == domainName())
  {
    /*DNS Query is for CPGateway.*/
  }
  else if(hh != hostName() && dd == domainName())
  {
    /*Find in Hash Map to get the IP.*/
    IP = parent.getDhcpServerUser().getResolverIP(hh);

  }

  if(!IP)
  {
    /*The Host is not controled by CPGateway.*/
    //ACE_Message_Block *mb = buildAndSendResponse(parent, in, inLen, 
  }
  else
  {
    /*IP is maintained by CPGateway.*/
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

    DNS::QData data;

    while(len)
    {
      DNS::QHdr qHdr;

      qHdr.len(len);
      qHdr.value((ACE_Byte *)&qData[offset]);

      /*remember it into list (which is STACK) for later use.*/
      data.m_qHdrList.push_back(qHdr);
      offset += len;

      /*Now update the length.*/
      len = qData[offset++];
    }

    data.qtype(ntohs(*((ACE_UINT16 *)&qData[offset])));
    offset += 2;
    data.qclass(ntohs(*((ACE_UINT16 *)&qData[offset])));
    offset += 2;
    m_qDataList.push_back(data);
    /*Process next query data.*/
    qdcount--;
  }

  ACE_Message_Block *mb = nullptr;
  ACE_NEW_NORETURN(mb, ACE_Message_Block(CommonIF::SIZE_2MB));

  buildDnsResponse(parent, in, inLen, *mb);
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l the length is %u\n"), mb->length()));
  parent.sendResponse(macAddr(), (ACE_Byte *)mb->rd_ptr(), mb->length());

  /*clean the vector now.*/
  purgeQData();
  /*re-claim the memory now.*/
  mb->release();
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
      processQdcount(parent, in, inLen, ntohs(dnsHdr->qdcount));
    }

    if(ntohs(dnsHdr->ancount))
    {
      processAncount(parent, in, inLen, ntohs(dnsHdr->ancount));
    }

    if(ntohs(dnsHdr->nscount))
    {
      processNscount(parent, in, inLen, ntohs(dnsHdr->nscount));
    }

    if(ntohs(dnsHdr->arcount))
    {
      processArcount(parent, in, inLen, ntohs(dnsHdr->arcount));
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

void DNS::CPGwDns::purgeQData(void)
{
  /*Outer Iterator.*/
  std::vector<DNS::QData>::iterator iter;
  /*Inner Iterator.*/
  std::vector<DNS::QHdr>::iterator inIter;

  DNS::QData elm;

  for(iter = m_qDataList.begin(); iter !=m_qDataList.end();)
  {
    elm = *iter;
    for(inIter = elm.m_qHdrList.begin(); inIter != elm.m_qHdrList.end();)
    {
      inIter = elm.m_qHdrList.erase(inIter);
    }

    iter = m_qDataList.erase(iter);
  }
}


#endif /*__DNS_CC__*/
