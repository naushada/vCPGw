#ifndef __DHCP_COMMON_H__
#define __DHCP_COMMON_H__

#include <ace/Basic_Types.h>
#include "ace/SString.h"

namespace RFC2131
{
  /*DHCP Message Type*/
  static const ACE_UINT8 DISCOVER = 1;
  static const ACE_UINT8 OFFER = 2;
  static const ACE_UINT8 REQUEST = 3;
  static const ACE_UINT8 DECLINE = 4;
  static const ACE_UINT8 ACK = 5;
  static const ACE_UINT8 NACK = 6;
  static const ACE_UINT8 RELEASE = 7;

  /*DHCP OPTIONS*/
  static const ACE_UINT8 OPTION_PADD = 0;
  static const ACE_UINT8 OPTION_SUBNET_MASK = 1;
  static const ACE_UINT8 OPTION_ROUTER = 3;
  static const ACE_UINT8 OPTION_TIME_SERVER = 4;
  static const ACE_UINT8 OPTION_NAME_SERVER = 5;
  static const ACE_UINT8 OPTION_DNS = 6;
  static const ACE_UINT8 OPTION_LOG_SERVER = 7;
  static const ACE_UINT8 OPTION_QUOTE_SERVER = 8;
  static const ACE_UINT8 OPTION_IMPRESS_SERVER = 10;
  static const ACE_UINT8 OPTION_ROUTER_LOCATION_SERVER = 11;
  static const ACE_UINT8 OPTION_HOST_NAME = 12;
  static const ACE_UINT8 OPTION_DOMAIN_NAME = 15;
  /*Interface MTU*/
  static const ACE_UINT8 OPTION_MTU = 26;
  static const ACE_UINT8 OPTION_BROADCAST_ADDRESS = 28;
  static const ACE_UINT8 OPTION_NIS_DOMAIN = 40;
  static const ACE_UINT8 OPTION_NIS = 41;
  static const ACE_UINT8 OPTION_NTP_SERVER = 42;
  static const ACE_UINT8 OPTION_VENDOR_SPECIFIC_INFO = 43;
  static const ACE_UINT8 OPTION_REQUESTED_IP_ADDRESS = 50;
  static const ACE_UINT8 OPTION_IP_LEASE_TIME = 51;
  static const ACE_UINT8 OPTION_OVERLOAD = 52;
  static const ACE_UINT8 OPTION_MESSAGE_TYPE = 53;
  static const ACE_UINT8 OPTION_SERVER_IDENTIFIER = 54;
  static const ACE_UINT8 OPTION_PARAMETER_REQUEST_LIST = 55;
  static const ACE_UINT8 OPTION_MESSAGE = 56;
  static const ACE_UINT8 OPTION_MESSAGE_SIZE = 57;
  static const ACE_UINT8 OPTION_RENEWAL_TIME_T1 = 58;
  static const ACE_UINT8 OPTION_RENEWAL_TIME_T2 = 59;
  static const ACE_UINT8 OPTION_CLASS_IDENTIFIER = 60;
  static const ACE_UINT8 OPTION_CLIENT_IDENTIFIER = 61;
  static const ACE_UINT8 OPTION_RAPID_COMMIT = 80;
  static const ACE_UINT8 OPTION_AUTO_CONFIGURE = 116;
  static const ACE_UINT8 OPTION_END = 255;

  class DhcpCtx
  {
  private:
    /*Transaction ID*/
    ACE_UINT32 m_xid;
    /*client ip address*/
    ACE_UINT32 m_ciaddr;
    /*assigned/newly allocated ip address*/
    ACE_UINT32 m_yiaddr;
    /*DHCP Server ip address*/
    ACE_UINT32 m_siaddr;
    /*For dhcp Agent, It's a gateway IP*/
    ACE_UINT32 m_giaddr;
    /*MAC Address length*/
    ACE_UINT8 m_chaddrLen;
    /*client mac address*/
    ACE_Byte m_chaddr[64];
    /*client's host name*/
    ACE_Byte m_sname[64];

  public:
    DhcpCtx()
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

    ~DhcpCtx()
    {
    }

    ACE_UINT32 xid(void)
    {
      return(m_xid);
    }
    void xid(ACE_UINT32 xid)
    {
      m_xid = xid;
    }

    ACE_UINT32 ciaddr(void)
    {
      return(m_ciaddr);
    }
    void ciaddr(ACE_UINT32 ciaddr)
    {
      m_ciaddr = ciaddr;
    }

    ACE_UINT32 yiaddr(void)
    {
      return(m_yiaddr);
    }
    void yiaddr(ACE_UINT32 yiaddr)
    {
      m_yiaddr = yiaddr;
    }

    ACE_UINT32 siaddr(void)
    {
      return(m_siaddr);
    }
    void siaddr(ACE_UINT32 siaddr)
    {
      m_siaddr = siaddr;
    }

    ACE_UINT32 giaddr(void)
    {
      return(m_giaddr);
    }
    void giaddr(ACE_UINT32 giaddr)
    {
      m_giaddr = giaddr;
    }

    ACE_UINT32 chaddrLen(void)
    {
      return(m_chaddrLen);
    }
    void chaddrLen(ACE_UINT8 len)
    {
      m_chaddrLen = len;
    }

    ACE_Byte *chaddr(void)
    {
      return((ACE_Byte *)m_chaddr);
    }
    void chaddr(ACE_Byte *chaddr)
    {
      ACE_OS::memcpy((void *)m_chaddr, (const void *)chaddr, chaddrLen());
    }

    ACE_Byte *sname(void)
    {
      return((ACE_Byte *)m_sname);
    }
    void sname(ACE_Byte *sname)
    {
      ACE_OS::strcpy((char *)m_sname,(const char *)sname);
    }
  };

  struct DhcpOption
  {
    DhcpOption()
    {
      m_tag = 0;
      m_len = 0;
      ACE_OS::memset((void *)m_value, 0, sizeof(m_value));
    }

    ~DhcpOption()
    {
    }

    ACE_UINT8 m_tag;
    ACE_UINT8 m_len;
    ACE_Byte m_value[255];

    ACE_UINT8 getTag(void)
    {
      return(m_tag);
    }

    void setTag(ACE_UINT8 tag)
    {
      m_tag = tag;
    }

    ACE_UINT8 getLen(void)
    {
      return(m_len);
    }

    void setLen(ACE_UINT8 len)
    {
      m_len = len;
    }

    void setValue(ACE_Byte *value, ACE_UINT8 len)
    {
      setLen(len);
      ACE_OS::memcpy((void *)m_value, (const void *)&value, (size_t)len);
    }

    ACE_Byte getValue(ACE_Byte *value)
    {
      ACE_OS::memcpy((void *)value, (const void *)m_value, (size_t)getLen());
      return(getLen());
    }
  };
}

#endif /*__DHCP_COMMON_H__*/
