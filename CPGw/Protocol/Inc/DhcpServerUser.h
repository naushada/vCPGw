#ifndef __DHCP_SERVER_USER_H__
#define __DHCP_SERVER_USER_H__

#include <list>

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

typedef std::list<ACE_UINT32>IPPoolList_t;
typedef std::list<ACE_UINT32>::iterator IPPoolList_Iter_t;

class DhcpServerUser : public ACE_Event_Handler
{
private:
  /*Hash Map whose key is mac address of DHCP Client.*/
  DhcpServerInstMap_t m_instMap;
  /*Subscriber Map based on IP Address. Key=IP, Value=MacAddress.*/
  SessionMap_t m_sessMap;
  /*HostName to IP map for Resolving IP.*/
  ResolverMap_t m_name2IPMap;
  /*back pointer*/
  CPGateway *m_cpGw;

  /*Populating IP from sIP and eIP*/
  IPPoolList_t m_ipPoolList;
  IPPoolList_Iter_t m_selectedIP;

public:

  DhcpServerUser(CPGateway *parent);
  virtual ~DhcpServerUser();

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

  ACE_HANDLE handle_timeout(const ACE_Time_Value &tv,
                           const void *act=0);

  long start_timer(ACE_UINT32 delay, const void *act,
                   ACE_Time_Value interval = ACE_Time_Value::zero);

  void stop_timer(long timerId);
  void reset_timer(long tId, ACE_UINT32 timeOutInSec);
  void populateIPPool(ACE_UINT32 mask, ACE_UINT32 sIP, ACE_UINT32 eIP);
  ACE_UINT32 getIPFromPool(void);
};


#endif /*__DHCP_SERVER_USER_H__*/
