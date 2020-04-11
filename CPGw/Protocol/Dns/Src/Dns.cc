#ifndef __DNS_CC__
#define __DNS_CC__

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

#endif /*__DNS_CC__*/
