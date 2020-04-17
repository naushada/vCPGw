#ifndef __DHCP_SERVER_H__
#define __DHCP_SERVER_H__

#include "DhcpCommon.h"
#include "DhcpServerState.h"
#include <ace/Basic_Types.h>
#include <ace/SString.h>
#include <ace/Hash_Map_Manager.h>
#include <ace/Null_Mutex.h>
#include <ace/Event_Handler.h>

class DhcpServerState;
class DhcpServerUser;

namespace DHCP
{
  static const ACE_UINT32 EXPECTED_REQUEST_GUARD_TIMER_ID = 0x01;
  static const ACE_UINT32 LEASE_GUARD_TIMER_ID = 0x02;

  static const ACE_UINT32 SIZE_64MB = (1 << 26);
  static const ACE_UINT32 SIZE_32MB = (1 << 25);
  static const ACE_UINT32 SIZE_16MB = (1 << 24);
  static const ACE_UINT32 SIZE_8MB  = (1 << 23);
  static const ACE_UINT32 SIZE_4MB  = (1 << 22);
  static const ACE_UINT32 SIZE_2MB  = (1 << 21);
  static const ACE_UINT32 SIZE_1MB  = (1 << 20);
  static const ACE_INT32 SUCCESS = 0;
  static const ACE_INT32 FAILURE = -1;

  typedef ACE_Hash_Map_Manager<ACE_UINT8, RFC2131::DhcpOption*, ACE_Null_Mutex>ElemDef;
  typedef ACE_Hash_Map_Manager<ACE_UINT8, RFC2131::DhcpOption *, ACE_Null_Mutex>::iterator ElemDef_iter;

  class Server
  {
  private:
    ACE_Message_Block *m_mb;
    ACE_CString m_description;
    /*State Machine Instance.*/
    DhcpServerState *m_state;
    /*dhcp header is in context.*/
    RFC2131::DhcpCtx *m_ctx;
    ElemDef m_optionMap;
    /*Back Pointrt to parent.*/
    DhcpServerUser *m_dhcpServerUser;
    ACE_UINT32 m_lease;
    /*DHCP Client's Host Name.*/
    ACE_CString m_sname;
    /*IP Address of DHCP Client.*/
    ACE_UINT32 m_ipAddr;

    /*DHCP's Server macAddress.*/
    ACE_CString m_macAddress;
    ACE_CString m_myIP;
    ACE_CString m_hostName;
    ACE_CString m_domainName;

  public:
    Server();
    Server(DhcpServerUser *user, ACE_CString mac, ACE_CString ip,
           ACE_CString hostName, ACE_CString domainName);
    virtual ~Server();

    void setState(DhcpServerState *st);
    DhcpServerState &getState(void);

    ACE_UINT32 start();
    ACE_UINT32 stop();

    void lease(ACE_UINT32 to);
    ACE_UINT32 lease(void);

    ACE_CString &sname(void);
    void sname(ACE_CString &sname);

    void xid(ACE_UINT32 xid);
    ACE_UINT32 xid(void);
    RFC2131::DhcpCtx &ctx(void);
    ElemDef &optionMap(void);

    DhcpServerUser &getDhcpServerUser(void);
    void setDhcpServerUser(DhcpServerUser *usr);

    ACE_CString &getMacAddress(void);
    void setMacAddress(ACE_CString mac);

    ACE_UINT32 ipAddr(void);
    void ipAddr(ACE_UINT32 ip);
    void ipAddr(ACE_CString ip);
    void setMyIP(ACE_CString ip);
    ACE_CString &getMyIP(void);

    ACE_CString &hostName(void);
    void hostName(ACE_CString &hName);
    ACE_CString &domainName(void);
    void domainName(ACE_CString &dName);

  };
}


#endif /*__DHCP_SERVER_H__*/
