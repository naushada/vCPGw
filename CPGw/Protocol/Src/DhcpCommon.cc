#ifndef __DHCP_COMMON_CC__
#define __DHCP_COMMON_CC__

#include "DhcpCommon.h"

RFC2131::DhcpCtx::DhcpCtx()
{
  m_xid = 0;
  m_ciaddr = 0;
  m_yiaddr = 0;
  m_siaddr = 0;
  m_giaddr = 0;
  m_chaddrLen = 0;
  ACE_OS::memset((void *)m_chaddr, 0, sizeof(m_chaddr));
  ACE_OS::memset((void *)m_sname, 0, sizeof(m_sname));
}

RFC2131::DhcpCtx::~DhcpCtx()
{
  ACE_DEBUG((LM_INFO, "RFC2131::DhcpCtx::~DhcpCtx\n"));
}

ACE_UINT32 RFC2131::DhcpCtx::xid(void)
{
  return(m_xid);
}

void RFC2131::DhcpCtx::xid(ACE_UINT32 xid)
{
  m_xid = xid;
}

ACE_UINT32 RFC2131::DhcpCtx::ciaddr(void)
{
  return(m_ciaddr);
}
void RFC2131::DhcpCtx::ciaddr(ACE_UINT32 ciaddr)
{
  m_ciaddr = ciaddr;
}

ACE_UINT32 RFC2131::DhcpCtx::yiaddr(void)
{
  return(m_yiaddr);
}
void RFC2131::DhcpCtx::yiaddr(ACE_UINT32 yiaddr)
{
  m_yiaddr = yiaddr;
}

ACE_UINT32 RFC2131::DhcpCtx::siaddr(void)
{
  return(m_siaddr);
}
void RFC2131::DhcpCtx::siaddr(ACE_UINT32 siaddr)
{
  m_siaddr = siaddr;
}

ACE_UINT32 RFC2131::DhcpCtx::giaddr(void)
{
  return(m_giaddr);
}
void RFC2131::DhcpCtx::giaddr(ACE_UINT32 giaddr)
{
  m_giaddr = giaddr;
}

ACE_UINT8 RFC2131::DhcpCtx::chaddrLen(void)
{
  return(m_chaddrLen);
}
void RFC2131::DhcpCtx::chaddrLen(ACE_UINT8 len)
{
  m_chaddrLen = len;
}

ACE_Byte *RFC2131::DhcpCtx::chaddr(void)
{
  return((ACE_Byte *)m_chaddr);
}

void RFC2131::DhcpCtx::getChaddr(ACE_Byte *mac)
{
  ACE_OS::memcpy((void *)mac, chaddr(), chaddrLen());
}

void RFC2131::DhcpCtx::chaddr(ACE_Byte *caddr)
{
  ACE_OS::memcpy((void *)m_chaddr, (const void *)caddr, chaddrLen());
  ACE_DEBUG((LM_DEBUG, "\nNN::\n"));
  ACE_DEBUG((LM_DEBUG, "%X:", m_chaddr[0]));
  ACE_DEBUG((LM_DEBUG, "%X:", m_chaddr[1]));
  ACE_DEBUG((LM_DEBUG, "%X:", m_chaddr[2]));
  ACE_DEBUG((LM_DEBUG, "%X:", m_chaddr[3]));
  ACE_DEBUG((LM_DEBUG, "%X:", m_chaddr[4]));
  ACE_DEBUG((LM_DEBUG, "%X\n", m_chaddr[5]));
  ACE_DEBUG((LM_DEBUG, "len %u\n", chaddrLen()));
}

ACE_Byte *RFC2131::DhcpCtx::sname(void)
{
  return((ACE_Byte *)m_sname);
}
void RFC2131::DhcpCtx::sname(ACE_Byte *sname)
{
  ACE_OS::strcpy((char *)m_sname,(const char *)sname);
}




#endif /*__DHCP_COMMON_CC__*/
