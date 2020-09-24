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

typedef struct TIMER_ID
{
  TIMER_ID()
  {
    m_tid = 0;
    m_timerType = 0;
    m_chaddrLen = 0;
    ACE_OS::memset((void *)m_chaddr, 0, sizeof(m_chaddr));
  }

  long m_tid;
  ACE_UINT32 m_timerType;
  ACE_UINT8 m_chaddrLen;
  ACE_Byte m_chaddr[16];

  long tid(void)
  {
    return(m_tid);
  }

  void tid(long t)
  {
    m_tid = t;
  }

  ACE_UINT32 timerType(void)
  {
    return(m_timerType);
  }

  void timerType(ACE_UINT32 tType)
  {
    m_timerType = tType;
  }

  ACE_UINT8 chaddrLen(void)
  {
    return(m_chaddrLen);
  }

  void chaddrLen(ACE_UINT8 len)
  {
    m_chaddrLen = len;
  }

  ACE_Byte *chaddr(void)
  {
    return(m_chaddr);
  }

  void chaddr(ACE_Byte *cha)
  {
    ACE_OS::memcpy((void *)m_chaddr, (const void *)cha, chaddrLen());
  }

}TIMER_ID;


namespace DHCP
{
  static const ACE_UINT32 PURGE_TIMER_MSG_ID = 0x01;
  static const ACE_UINT32 LEASE_TIMER_MSG_ID = 0x02;

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
    /*TimerId of purging the cache/instance. This is used to stop the timer explicitly.*/
    TIMER_ID *m_purgeTid;
    ACE_Message_Block *m_mb;
    ACE_CString m_description;

    /*DHCP FSM - State Machine Instance.*/
    DhcpServerState *m_state;

    /*dhcp header is in context.*/
    RFC2131::DhcpCtx *m_ctx;
    ElemDef m_optionMap;
    /*Back Pointrt to parent.*/
    DhcpServerUser *m_dhcpServerUser;

    /*DHCP Client's Host Name.*/
    ACE_CString m_sname;

    /*IP Address of DHCP Client.*/
    ACE_UINT32 m_ipAddr;
    ACE_UINT32 m_subnetMask;
    ACE_UINT32 m_dnsIP;
    ACE_UINT16 m_mtu;
    ACE_UINT32 m_lease;

    /*DHCP's Server macAddress.*/
    ACE_CString m_macAddress;
    ACE_CString m_myIP;
    ACE_CString m_hostName;
    ACE_CString m_domainName;

  public:
    Server();
    Server(DhcpServerUser *user, ACE_CString mac, ACE_CString ip,
           ACE_CString hostName, ACE_CString domainName, ACE_UINT16 mtu,
           ACE_UINT32 lease, ACE_UINT32 dnsIP, ACE_UINT32 subnetMask,
           ACE_UINT32 clientIP);

    virtual ~Server();

    /* This method is invoked with instance of derived class as argument.*/
    void setState(DhcpServerState *st);
    DhcpServerState &getState(void);

    ACE_UINT32 start();
    ACE_UINT32 stop();

    void lease(ACE_UINT32 to);
    ACE_UINT32 lease(void);

    ACE_CString &sname(void);
    void sname(ACE_CString &sname);

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

    void purgeTid(TIMER_ID *tId);
    TIMER_ID &purgeTid(void);
    TIMER_ID *purgeInst(void);

    ACE_UINT32 subnetMask(void);
    void subnetMask(ACE_UINT32 mask);
    ACE_UINT32 dnsIP(void);
    void dnsIP(ACE_UINT32 ip);
    ACE_UINT16 mtu(void);
    void mtu(ACE_UINT16 m);
  };
}


#endif /*__DHCP_SERVER_H__*/
