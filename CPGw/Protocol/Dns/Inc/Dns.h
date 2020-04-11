#ifndef __DNS_H__
#define __DNS_H__

#include "ace/Basic_Types.h"
#include "ace/SString.h"

class CPGateway;

namespace DNS
{
  class CPGwDns
  {
  private:
    ACE_CString m_hostName;
    ACE_CString m_domainName;
    ACE_CString m_macAddress;
    ACE_UINT32 m_ipAddr;

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

  };
}

#endif /*__DNS_H__*/
