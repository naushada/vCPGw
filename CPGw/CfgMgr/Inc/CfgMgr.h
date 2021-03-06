#ifndef __CFG_MGR_H__
#define __CFG_MGR_H__

#include "Json.h"
#include "UniIPC.h"

#include "ace/Basic_Types.h"
#include "ace/SString.h"
#include "ace/Hash_Map_Manager.h"
#include "ace/Null_Mutex.h"

typedef union _ipAddr
{
  ACE_Byte m_ips[4];
  ACE_UINT32 m_ipn;
}__attribute__((packed))_ipAddr_t;

typedef struct _CpVirtualNw
{
  ACE_Byte m_name[255];
  ACE_Byte m_type[32];
  _ipAddr m_ip;
  _ipAddr m_mask;
  ACE_Byte m_port[32];
}__attribute__((packed))_CpVirtualNw_t;

typedef struct _CpGwAAAInstance
{
  _CpVirtualNw_t m_nw;
  _ipAddr_t m_ip;
  ACE_Byte m_protocol[8];
  ACE_UINT16 m_auth_port;
  ACE_UINT16 m_acc_port;
  ACE_Byte m_admin_user[256];
  ACE_Byte m_admin_pwd[256];
  _ipAddr_t m_peer_ip;
  ACE_UINT16 m_peer_port;

}__attribute__((packed))_CpGwAAAInstance_t;

typedef struct _CpGwHTTPInstance
{
  _CpVirtualNw_t m_nw;
  _ipAddr_t m_ip;
  ACE_UINT16 m_port;
  _CpGwAAAInstance_t m_aaa;

}__attribute__((packed))_CpGwHTTPInstance_t;

typedef struct _CpDhcpProfile
{
  ACE_Byte m_mtu;
  _ipAddr m_ip;
  ACE_Byte m_domain_name[255];
  ACE_UINT32 m_lease;

}__attribute__((packed))_CpDhcpProfile_t;

typedef struct _CpGwAPZone
{
  ACE_Byte m_channel_no;
  ACE_Byte m_hw_mode[8];
  ACE_Byte m_auth_algo;
}__attribute__((packed))_CpGwAPZone_t;

typedef struct _CpGwAPInstance
{
  _CpGwAPZone_t m_zone;
  /*Virtual Network.*/
  _CpVirtualNw_t m_nw;
  ACE_Byte m_ap_name[255];
  ACE_Byte m_latitude[255];
  ACE_Byte m_longitude[255];
  ACE_Byte m_elevation[255];
}__attribute__((packed))_CpGwAPInstance_t;

typedef struct _CpGwAPPeer
{
  _CpGwAPZone_t m_zone;
  ACE_Byte m_ap_name[255];
  ACE_Byte m_ssid[255];
  ACE_Byte m_cc[64];
  ACE_Byte m_wap;
  ACE_Byte m_passphrase[255];
  ACE_Byte m_wpa_key_mgmt[16];
  ACE_Byte m_wpa_pairwise[16];
  ACE_Byte m_rsn_pairwise[16];
  ACE_Byte m_driver[16];
}__attribute__((packed))_CpGwAPPeer_t;


typedef struct _CpGwDHCPInstance
{
  /*Virtual Network.*/
  _CpVirtualNw_t m_nw;
  /*DHCP Server's Profile.*/
  _CpDhcpProfile_t m_profile;
  _ipAddr_t m_mask;
  _ipAddr_t m_ip;
  ACE_Byte m_host_name[255];
  _ipAddr_t m_start_ip;
  _ipAddr_t m_end_ip;
  ACE_Byte m_ip_count;
  /*Variable number of exclude ips.*/
  _ipAddr_t m_exclude_ip[255];

}__attribute__((packed))_CpGwDHCPInstance_t;

typedef struct _CpGwDHCPAgentInstance
{
  /*Virtual Network.*/
  _CpVirtualNw_t m_nw;
  _ipAddr_t m_mask;
  _ipAddr_t m_ip;
  ACE_Byte m_host_name[255];
  /*this is the DHCP Server IP.*/
  _ipAddr_t m_server_ip;

}__attribute__((packed))_CpGwDHCPAgentInstance_t;

typedef struct _CpGwCPGWInstance
{
  /*Virtual Network.*/
  _CpVirtualNw_t m_nw;
  _ipAddr_t m_mask;
  _ipAddr_t m_ip;
  ACE_Byte m_host_name[255];

}__attribute__((packed))_CpGwCPGWInstance_t;

typedef struct _InstanceConfig
{
  ACE_UINT8 m_DHCPInstCount;
  _CpGwDHCPInstance_t m_instDHCP[8];

  ACE_UINT8 m_DHCPAgentInstCount;
  _CpGwDHCPAgentInstance_t m_instDHCPAgent[8];

  ACE_UINT8 m_HTTPInstCount;
  _CpGwHTTPInstance_t m_instHTTP[8];

  ACE_UINT8 m_AAAInstCount;
  _CpGwAAAInstance_t m_instAAA[4];

  ACE_UINT8 m_CPGWInstCount;
  _CpGwCPGWInstance_t m_instCPGW[4];

    ACE_UINT8 m_APInstCount;
  _CpGwAPInstance_t m_instAP[8];

}__attribute__((packed))_InstanceConfig_t;

typedef struct _PeerConfig
{
}__attribute__((packed))_PeerConfig_t;

typedef struct _CpGwConfigs
{
  _InstanceConfig_t m_instance;
  _PeerConfig_t m_peer;

}__attribute__((packed))_CpGwConfigs_t;

typedef ACE_Hash_Map_Manager<ACE_CString, _CpGwAAAInstance_t*, ACE_Null_Mutex>AAAInstMap_t;
typedef ACE_Hash_Map_Manager<ACE_CString, _CpGwHTTPInstance_t*, ACE_Null_Mutex>HTTPInstMap_t;
typedef ACE_Hash_Map_Manager<ACE_CString, _CpGwAPInstance_t*, ACE_Null_Mutex>APInstMap_t;
typedef ACE_Hash_Map_Manager<ACE_CString, _CpGwDHCPInstance_t*, ACE_Null_Mutex>DHCPInstMap_t;
typedef ACE_Hash_Map_Manager<ACE_CString, _CpGwDHCPAgentInstance_t*, ACE_Null_Mutex>DHCPAgentInstMap_t;
typedef ACE_Hash_Map_Manager<ACE_CString, _CpGwCPGWInstance_t*, ACE_Null_Mutex>CPGWInstMap_t;
/*Iterator.*/
typedef ACE_Hash_Map_Manager<ACE_CString, _CpGwAAAInstance_t*, ACE_Null_Mutex>::iterator AAAInstMap_Iter_t;
typedef ACE_Hash_Map_Manager<ACE_CString, _CpGwHTTPInstance_t*, ACE_Null_Mutex>::iterator HTTPInstMap_Iter_t;
typedef ACE_Hash_Map_Manager<ACE_CString, _CpGwAPInstance_t*, ACE_Null_Mutex>::iterator APInstMap_Iter_t;
typedef ACE_Hash_Map_Manager<ACE_CString, _CpGwDHCPInstance_t*, ACE_Null_Mutex>::iterator DHCPInstMap_Iter_t;
typedef ACE_Hash_Map_Manager<ACE_CString, _CpGwDHCPAgentInstance_t*, ACE_Null_Mutex>::iterator DHCPAgentInstMap_Iter_t;
typedef ACE_Hash_Map_Manager<ACE_CString, _CpGwCPGWInstance_t*, ACE_Null_Mutex>::iterator CPGWInstMap_Iter_t;


class CfgMgr : public UniIPC
{
public:

  CfgMgr(ACE_CString &schema);
  ~CfgMgr();

  CfgMgr(ACE_Thread_Manager *thrMgr, ACE_CString ip,
         ACE_UINT8 entId, ACE_UINT8 instId, ACE_CString nodeTag);

  AAAInstMap_t &aaa(void);
  HTTPInstMap_t &http(void);
  APInstMap_t &ap(void);
  DHCPInstMap_t &dhcp(void);
  DHCPAgentInstMap_t &agent(void);
  CPGWInstMap_t &cpgw(void);

  ACE_INT32 processCfg(void);
  ACE_INT32 processDHCPServerCfg(void);
  ACE_INT32 processDHCPAgentCfg(void);
  ACE_INT32 processHTTPServerCfg(void);
  ACE_INT32 processCPGWCfg(void);
  ACE_INT32 processAPCfg(void);
  ACE_INT32 processAAACfg(void);

  ACE_INT32 publishCpGwConfig(void);

  ACE_Byte purgeDHCP(void);
  ACE_Byte purgeDHCPAgent(void);
  ACE_Byte purgeAAA(void);
  ACE_Byte purgeAPInst(void);
  ACE_Byte purgeHTTP(void);
  ACE_Byte purgeCPGW(void);

  void display(void);
  void displayDHCP(void);
  void displayDHCPAgent(void);
  void displayAAA(void);
  void displayAPInst(void);
  void displayHTTP(void);
  void displayCPGW(void);

  ACE_Byte start(void);
  ACE_Byte stop(void);

  int svc(void);
  ACE_UINT32 handle_ipc(ACE_Message_Block *mb);
  ACE_HANDLE get_handle(void) const;

  /*Build config and send to respective process.*/
  int buildConfigResponse(ACE_Byte *in, ACE_UINT32 len, ACE_Message_Block &mb);
  int processIPCMessage(ACE_Message_Block &mb);
  int buildIPCHeader(ACE_Byte *in, ACE_Message_Block &mb);

  void schema(ACE_CString sc);

private:
  AAAInstMap_t m_aaa;
  HTTPInstMap_t m_http;
  APInstMap_t m_ap;
  DHCPInstMap_t m_dhcp;
  DHCPAgentInstMap_t m_agent;
  CPGWInstMap_t m_cpgw;

  JSON *m_cpGwCfg;
  ACE_CString m_schema;
};

#endif /*__CFG_MGR_H__*/
