#ifndef __CFG_CPGW_H__
#define __CFG_CPGW_H__

#include "ace/Basic_Types.h"

typedef union _ipAddr
{
  ACE_Byte m_ips[4];
  ACE_UINT32 m_ipn;
}_ipAddr_t;

typedef struct _CpGwAAAInstance
{
  _CpVirtualNw_t m_nw;
  _ipAddr_t m_ip;
  ACE_Byte m_protocol[8];
  ACE_UINT16 m_auth_port;
  ACE_UINT16 m_acc_port;
  _ipAddr_t m_peer_ip;
  ACE_UINT16 m_peer_port;

}_CpGwAAAInstance_t;

struct _CpGwHTTPInsatnces
{
  _CpGwHTTPInsatnce_t m_instance;
  _CpGwHTTPInsatnces *m_next;
}_CpGwHTTPInsatnces_t;

typedef struct _CpGwHTTPInsatnce
{
  _CpVirtualNw_t m_nw;
  _ipAddr_t m_ip;
  ACE_UINT16 m_port;

}_CpGwHTTPInsatnce_t;

typedef struct _CpGwAPInstance
{
  ACE_Byte m_ap_name[255];
  ACE_Byte m_ssid[255];
  ACE_Byte m_cc[64];
  ACE_Byte m_hw_mode[8];
  ACE_Byte m_channel_no;
  ACE_Byte m_auth_algo;
  ACE_Byte m_wap;
  ACE_Byte m_passphrase[255];
  ACE_Byte m_wpa_key_mgmt[16];
  ACE_Byte m_wpa_pairwise[16];
  ACE_Byte m_rsn_pairwise[16];
  ACE_Byte m_driver[16];
}_CpGwAPInstance_t;

struct _CpGwAPInstances
{
  _CpGwAPInstances_t m_instance;
  struct _CpGwAPInstances *m_next;
}_CpGwAPInstances_t;

typedef struct _CpGwDhcpInstance
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

}_CpGwDhcpInstance_t;

struct _CpGwDhcpInstances
{
  _CpGwDhcpInstance_t m_instance;
  _CpGwDhcpInstances *m_next;

}_CpGwDhcpInstances_t;

typedef struct _CpVirtualNw
{
  ACE_Byte m_name[255];
  ACE_Byte m_type[32];
  _ipAddr m_ip;
  _ipAddr m_mask;
  ACE_Byte m_port[32];
}_CpVirtualNw_t;

struct _CpVirtualNws
{
  _CpVirtualNw_t m_instance;
  _CpVirtualNws *m_next;
}_CpVirtualNws_t;

typedef struct _CpDhcpProfile
{
  ACE_Byte m_mtu;
  _ipAddr m_dns_ip;
  ACE_Byte m_domain_name[255];
  ACE_UINT32 m_lease;

}_CpDhcpProfile_t;

struct _CpDhcpProfiles
{
  _CpDhcpProfile_t m_instance;
  _CpDhcpProfiles *m_next;

}_CpDhcpProfiles_t;

typedef struct _CpGwPeers
{
}_CpGwPeers_t;

typedef struct _CpGwInstances
{
  _CpGwDhcpInstances_t *m_dhcp_instances;
  _CpGwAPInstances_t *m_ap_instances;

}_CpGwInstances_t;


#endif /*__CFG_CPGW_H__*/
