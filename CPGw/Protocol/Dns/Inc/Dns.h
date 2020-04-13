#ifndef __DNS_H__
#define __DNS_H__

#include "ace/Basic_Types.h"
#include "ace/SString.h"
#include "ace/Containers.h"

class CPGateway;

namespace DNS
{
  static const ACE_UINT8 QUERY = 0;
  static const ACE_UINT8 INVERSE_QUERY = 1;
  static const ACE_UINT8 STATUS = 2;

  /*RR-Code*/
  static const ACE_UINT8 RCODE_NO_ERROR = 0;
  static const ACE_UINT8 RCODE_FORMAT_ERROR = 1;
  static const ACE_UINT8 RCODE_SERVER_FAILURE = 2;
  static const ACE_UINT8 RCODE_NAME_ERROR = 3;
  static const ACE_UINT8 RCODE_NOT_IMPLEMENTED = 4;
  static const ACE_UINT8 RCODE_REFUSED = 5;

  /*RR-Type*/
  static const ACE_UINT8 RRCODE_A = 1;
  static const ACE_UINT8 RRCODE_NA = 2;
  static const ACE_UINT8 RRCODE_MD = 3;
  static const ACE_UINT8 RRCODE_MF = 4;
  static const ACE_UINT8 RRCODE_CNAME = 5;
  static const ACE_UINT8 RRCODE_SOA = 6;
  static const ACE_UINT8 RRCODE_MB = 7;
  static const ACE_UINT8 RRCODE_MG = 8;
  static const ACE_UINT8 RRCODE_MR = 9;
  static const ACE_UINT8 RRCODE_AWKS = 11;
  static const ACE_UINT8 RRCODE_PTR = 12;
  static const ACE_UINT8 RRCODE_HINFO = 13;
  static const ACE_UINT8 RRCODE_MX = 14;
  static const ACE_UINT8 RRCODE_TXT = 15;
  static const ACE_UINT8 RRCODE_ALL = 255;

  /*RR-Class*/
  static const ACE_UINT16 RRCLASS_IN = 1;
  static const ACE_UINT16 RRCLASS_CS = 2;
  static const ACE_UINT16 RRCLASS_CH = 3;
  static const ACE_UINT16 RRCLASS_HS = 4;
  static const ACE_UINT16 RRCLASS_AXFR = 5;
  static const ACE_UINT16 RRCLASS_MAILB = 6;
  static const ACE_UINT16 RRCLASS_MAILA = 7;
  static const ACE_UINT16 RRCLASS_ANY = 255;

  typedef struct QHdr
  {
    ACE_Byte m_len;
    ACE_Byte m_value[128];

    QHdr()
    {
      m_len = 0;
      ACE_OS::memset((void *)m_value, 0, sizeof(m_value));
    }

    void len(ACE_Byte len)
    {
      m_len =len;
    }

    ACE_Byte len(void)
    {
      return(m_len);
    }

    ACE_Byte *value(void)
    {
      return(m_value);
    }

    void value(ACE_Byte *val)
    {
      ACE_OS::memcpy((void *)m_value, (const void *)val, len());
    }
  }QHdr;

  typedef struct QData
  {
    ACE_Unbounded_Stack<QHdr *> m_qHdrList;
    /*Query Type.*/
    ACE_UINT16 m_qtype;
    /*Query Class.*/
    ACE_UINT16 m_qclass;

    QData()
    {
      m_qtype = 0;
      m_qclass = 0;
    }

    ~QData()
    {
    }

    void qtype(ACE_UINT16 type)
    {
      m_qtype = type;
    }

    ACE_UINT16 qtype(void)
    {
      return (m_qtype);
    }

    ACE_UINT16 qclass(void)
    {
      return(m_qclass);
    }

    void qclass(ACE_UINT16 cls)
    {
      m_qclass = cls;
    }

  }QData;

  class CPGwDns
  {
  private:
    ACE_CString m_hostName;
    ACE_CString m_domainName;
    ACE_CString m_macAddress;
    ACE_UINT32 m_ipAddr;
    ACE_Unbounded_Stack<QData *> m_qDataList;

    CPGateway *m_parent;
  public:
    CPGwDns(CPGateway *parent, ACE_CString macAddr, ACE_CString hostName,
            ACE_CString domainName, ACE_UINT32 ip);
    ~CPGwDns();

    ACE_CString &hostName(void);
    void hostName(ACE_CString hname);

    ACE_CString &domainName(void);
    void domainName(ACE_CString dName);

    ACE_CString &macAddr(void);
    void macAddr(ACE_CString mac);

    ACE_UINT32 ipAddr(void);
    void ipAddr(ACE_UINT32 ip);

    ACE_UINT32 processRequest(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen);
    ACE_Message_Block *buildResponse(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen);
    void processQdcount(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen, ACE_UINT16 qdcount);
    void processAncount(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen, ACE_UINT16 ancount);
    void processNscount(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen, ACE_UINT16 nscount);
    void processArcount(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen, ACE_UINT16 arcount);
    void processDnsQury(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen);

  };
}

#endif /*__DNS_H__*/
