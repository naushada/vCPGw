#ifndef __DHCP_SERVER_USER_H__
#define __DHCP_SERVER_USER_H__

#include "DhcpServer.h"

#include "ace/SString.h"
#include "ace/Hash_Map_Manager.h"
#include "ace/Null_Mutex.h"
#include <ace/Event_Handler.h>
#include <ace/Basic_Types.h>

class CPGateway;

/*Hash Map based on client's MAC address.*/
typedef ACE_Hash_Map_Manager<ACE_CString, DHCP::Server*, ACE_Null_Mutex>DhcpServerInstMap_t;
typedef ACE_Hash_Map_Manager<ACE_CString, DHCP::Server *, ACE_Null_Mutex>::iterator DhcpServerInstMap_iter;

/*Hash Map based on IP Address of client. whose value is MAC Address.*/
typedef ACE_Hash_Map_Manager<ACE_UINT32, ACE_CString , ACE_Null_Mutex>SessionMap_t;
typedef ACE_Hash_Map_Manager<ACE_UINT32, ACE_CString, ACE_Null_Mutex>::iterator SessionMap_iter;

/*HostName to IP Mapping.*/
typedef ACE_Hash_Map_Manager<ACE_CString, ACE_CString , ACE_Null_Mutex>ResolverMap_t;
typedef ACE_Hash_Map_Manager<ACE_UINT32, ACE_CString, ACE_Null_Mutex>::iterator ResolverMap_iter;

typedef struct TIMER_ID
{
  TIMER_ID()
  {
    m_timerType = 0;
    m_chaddrLen = 0;
    ACE_OS::memset((void *)m_chaddr, 0, sizeof(m_chaddr));
  }

  ACE_UINT32 m_timerType;
  ACE_UINT8 m_chaddrLen;
  ACE_Byte m_chaddr[16];

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

class DhcpServerUser : public ACE_Event_Handler
{
private:
  /*Hash Map whose key is mac address of DHCP Client.*/
  DhcpServerInstMap_t m_instMap;
  /*Subscriber Map based on IP Address. Key=IP, Value=MacAddress.*/
  SessionMap_t m_sessMap;
  /*HostName to IP map for Resolving IP.*/
  ResolverMap_t m_name2IPMap;
  /*Guard Timer for next request.*/
  long m_guardTid;
  /*lease Expire timeout.*/
  long m_leaseTid;
  /*back pointer*/
  CPGateway *m_cpGw;

public:

  DhcpServerUser(CPGateway *parent);
  virtual ~DhcpServerUser();

  void guardTid(long gTid);
  long guardTid(void);

  void leaseTid(long lTid);
  long leaseTid(void);

  CPGateway &cpGw(void);

  int sendResponse(ACE_CString cha, ACE_Byte *in, ACE_UINT32 inLen);
  ACE_UINT32 processRequest(ACE_Byte *inPtr, ACE_UINT32 inLen);
  ACE_UINT8 isSubscriberFound(ACE_CString macAddress);
  ACE_UINT8 createSubscriber(ACE_CString macAddress);
  ACE_UINT8 addSubscriber(DHCP::Server *sess, ACE_CString macAddress);
  DHCP::Server *getSubscriber(ACE_CString macAddress);
  ACE_UINT8 deleteSubscriber(ACE_CString macAddress);

  void addSession(ACE_UINT32 ip, ACE_CString mac);
  void deleteSession(ACE_UINT32 ipAddr);

  void addResolver(ACE_CString hName, ACE_CString ip);
  void deleteResolver(ACE_CString hName);
  void updateResolver(ACE_CString hName, ACE_CString ip);
  ACE_Byte *getResolverIP(ACE_CString &hName);

  ACE_UINT8 isSubscriberAuthenticated(ACE_CString macAddress);

  ACE_INT32 process_timeout(const void *act);

  ACE_HANDLE handle_timeout(ACE_Time_Value &tv,
                           const void *act=0);

  long start_timer(ACE_UINT32 delay, const void *act,
                   ACE_Time_Value interval = ACE_Time_Value::zero);

  void stop_timer(long timerId);
};

class DhcpConf
{
private:
  static DhcpConf *m_instance;
  ACE_UINT8 m_mtu;
  ACE_UINT32 m_subnetMask;
  ACE_UINT32 m_dns;
  ACE_Byte m_domainName[255];
  ACE_UINT32 m_serverIp;
  ACE_Byte m_serverName[255];
  ACE_UINT32 m_leaseTime;

public:
  DhcpConf();
  DhcpConf(ACE_UINT8 mtu, ACE_UINT32 subnetMask, ACE_UINT32 dns,
           ACE_Byte *domainName, ACE_UINT32 serverIp, ACE_Byte *serverName,
           ACE_UINT32 lease);
  ~DhcpConf();
  static DhcpConf *instance();
};



#endif /*__DHCP_SERVER_USER_H__*/
