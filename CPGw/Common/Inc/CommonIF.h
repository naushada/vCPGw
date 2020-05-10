#ifndef __COMMONIF_H__
#define __COMMONIF_H__

#include <iostream>
#include <functional>
#include <ace/Basic_Types.h>

namespace CommonIF {

  static const ACE_UINT8 ENT_CPGW     = 0x81;
  static const ACE_UINT8 ENT_AAA      = 0x82;
  static const ACE_UINT8 ENT_CFGMGR   = 0x83;
  static const ACE_UINT8 ENT_SYSMGR   = 0x84;
  static const ACE_UINT8 ENT_PROCMGR  = 0x85;

  static const ACE_UINT8 OFFSET       = 0x80;
  static const ACE_UINT8 CONTAINER    = 0x80;
  static const ACE_UINT8 INST1        = 0x01;
  static const ACE_UINT8 INST2        = 0x02;
  static const ACE_UINT8 INST3        = 0x03;
  static const ACE_UINT8 INST4        = 0x04;

  static const ACE_UINT8 FAILURE = 1;
  static const ACE_UINT8 SUCCESS = 0;

  static const ACE_UINT32 SIZE_64MB          = (1 << 26);
  static const ACE_UINT32 SIZE_32MB          = (1 << 25);
  static const ACE_UINT32 SIZE_16MB          = (1 << 24);
  static const ACE_UINT32 SIZE_8MB           = (1 << 23);
  static const ACE_UINT32 SIZE_4MB           = (1 << 22);
  static const ACE_UINT32 SIZE_2MB           = (1 << 21);
  static const ACE_UINT32 SIZE_1MB           = (1 << 20);
  static const ACE_UINT32 SIZE_1KB           = (1 << 10);

  ACE_UINT32 get_hash32(const ACE_UINT8 *nodeTag);
  ACE_UINT16 get_ipc_port(ACE_UINT8 entity, ACE_UINT8 instance);
  ACE_UINT16 get_ipc_port(ACE_UINT32 taskId);
  ACE_UINT32 get_task_id(ACE_UINT8 entity, ACE_UINT8 instance);

  typedef struct _entNameIdTab
  {
    ACE_UINT8 m_entId;
    const ACE_TCHAR *m_entName;
  }_entNameIdTab_t;

  typedef struct _cmHeader
  {
    ACE_UINT32 m_procId;
    ACE_UINT8 m_entId;
    ACE_UINT8 m_instId;
  }_cmHeader_t;

  typedef struct _cmMessage
  {
    _cmHeader_t m_dst;
    _cmHeader_t m_src;
    ACE_UINT32 m_msgType;
    ACE_UINT32 m_messageLen;
    ACE_Byte m_message[1];

  }_cmMessage_t;

  static const _entNameIdTab_t m_entTable[] = {
    {ENT_CPGW,    "CPGateway"},
    {ENT_AAA,     "AAA"},
    {ENT_CFGMGR,  "CFGMGR"},
    {ENT_SYSMGR,  "SYSMGR"},
    {ENT_PROCMGR, "PROCMGR"},

    /*This shall be last entry*/
    {0, nullptr}
  };

  static const ACE_UINT32 MSG_CPGW_CFGMGR_BASE = 0x00010000;
  static const ACE_UINT32 MSG_CPGW_CFGMGR_CONFIG_REQ = (MSG_CPGW_CFGMGR_BASE + 1);

  static const ACE_UINT32 MSG_PROCMGR_SYSMGR_BASE = 0x00020000;
  static const ACE_UINT32 MSG_PROCMGR_SYSMGR_PROCESS_SPAWN_RSP = (MSG_PROCMGR_SYSMGR_BASE + 1);
  static const ACE_UINT32 MSG_PROCMGR_SYSMGR_PROCESS_DIED_IND    = (MSG_PROCMGR_SYSMGR_BASE + 2);

  static const ACE_UINT32 MSG_SYSMGR_PROCMGR_BASE = 0x00030000;
  static const ACE_UINT32 MSG_SYSMGR_PROCMGR_PROCESS_SPAWN_REQ = (MSG_SYSMGR_PROCMGR_BASE + 1);

  static const ACE_UINT32 MSG_CFGMGR_CPGW_BASE = 0x00040000;
  static const ACE_UINT32 MSG_CFGMGR_CPGW_CONFIG_REQ = (MSG_CFGMGR_CPGW_BASE + 1);

};

namespace TransportIF {
  static const ACE_UINT16 DHCP_CLIENT_PORT = 68;
  static const ACE_UINT16 DHCP_SERVER_PORT = 67;
  static const ACE_UINT16 DNS_SERVER_PORT = 53;

  static const ACE_Byte DHCP_COOKIE_LEN = 4;

  /*ARP Request.*/
  static const ACE_UINT8 ARPOP_REQUEST = 1;
  /*ARP Reply.*/
  static const ACE_UINT8 ARPOP_REPLY = 2;
  /*RARP Request.*/
  static const ACE_UINT8 ARPOP_RREQUEST = 3;
  /*RARP Reply.*/
  static const ACE_UINT8 ARPOP_RREPLY = 4;
  /*InARP Request.*/
  static const ACE_UINT8 ARPOP_InREQUEST = 8;
  /*InARP Reply.*/
  static const ACE_UINT8 ARPOP_InREPLY = 9;
  /*(ATM) ARP NAK*/
  static const ACE_UINT8 ARPOP_NAK = 10;
  /*Ethernet 10Mbps.*/
  static const ACE_UINT8 ARPHRD_ETHER = 1;

  static const ACE_UINT16 ARP_PROTO_IPv4 = 0x0800;
  /*Octets in one ethernet address.*/
  static const ACE_UINT8 ETH_ALEN = 6;
  /*ARP Packet*/
  static const ACE_UINT16 ETH_P_ARP = 0x0806;
  static const ACE_UINT16 ETH_P_IP = 0x0800;
  static const ACE_UINT16 ETH_P_ALL = 0x0003;
  static const ACE_UINT16 ETH_P_WOL = 0x0842;
  static const ACE_UINT16 ETH_P_ETHBR = 0x6558;
  static const ACE_UINT16 ETH_P_8021Q = 0x8100;
  static const ACE_UINT16 ETH_P_IPX = 0x8137;
  static const ACE_UINT16 ETH_P_IPv6 = 0x86DD;
  static const ACE_UINT16 ETH_P_PPP = 0x880B;
  static const ACE_UINT16 ETH_P_PPPOED = 0x8863;
  static const ACE_UINT16 ETH_P_PPPOES = 0x8864;
  static const ACE_UINT16 ETH_P_EAPOL = 0x888E;
  /*Packet Type.*/
  static const ACE_UINT8 IP_ICMP = 1;
  static const ACE_UINT8 IP_IGMP = 2;
  static const ACE_UINT8 IP_TCP = 6;
  static const ACE_UINT8 IP_UDP = 17;

  typedef struct ARP
  {
    /*HW Type.*/
    ACE_UINT16 hdr;
    /*Protocol Type.*/
    ACE_UINT16 proto;
    /*ARP HLEN.*/
    ACE_Byte hlen;
    /*Length of Protocol Address.*/
    ACE_Byte plen;
    /*ARP OP-CODE.*/
    ACE_UINT16 opcode;
    ACE_Byte sender_ha[ETH_ALEN];
    ACE_UINT32 sender_ip;
    ACE_Byte target_ha[ETH_ALEN];
    ACE_UINT32 target_ip;
  }__attribute__((packed))ARP;

  typedef struct ETH
  {
    /*destination MAC Address.*/
    ACE_Byte dest[ETH_ALEN];
    /*source MAC Address.*/
    ACE_Byte src[ETH_ALEN];
    /*Packet Type ID.*/
    ACE_UINT16 proto;
  }__attribute__((packed))ETH;

  typedef struct IP
  {
    ACE_UINT32 len:4;
    ACE_UINT32 ver:4;
    ACE_UINT32 tos:8;
    ACE_UINT32 tot_len:16;

    ACE_UINT16 id;
    ACE_UINT16 flags;

    ACE_UINT32 ttl:8;
    ACE_UINT32 proto:8;
    ACE_UINT32 chksum:16;

    ACE_UINT32 src_ip;
    ACE_UINT32 dest_ip;
  }__attribute__((packed))IP;

  typedef struct UDP
  {
    ACE_UINT16 src_port;
    ACE_UINT16 dest_port;
    ACE_UINT16 len;
    ACE_UINT16 chksum;
  }__attribute__((packed))UDP;

  typedef struct ICMP
  {
    ACE_Byte type;
    ACE_Byte code;
    ACE_UINT16 chksum;
    ACE_UINT16 id;
  }__attribute__((packed))ICMP;

  typedef struct DHCP
  {
    ACE_Byte op;
    ACE_Byte htype;
    ACE_Byte hlen;
    ACE_Byte hops;
    /*Random Transaction ID.*/
    ACE_UINT32 xid;
    ACE_UINT16 secs;
    ACE_UINT16 flags;
    ACE_UINT32 ciaddr;
    ACE_UINT32 yiaddr;
    ACE_UINT32 siaddr;
    ACE_UINT32 giaddr;
    ACE_Byte chaddr[16];
    ACE_Byte sname[64];
    ACE_Byte file[128];
  }__attribute__((packed))DHCP;

  typedef struct DNS
  {
    /*unique transaction id to map request into response.*/
    ACE_UINT16 xid;
    /*Recursion desired.*/
    ACE_UINT8 rd:1;
    /*Is message truncated.*/
    ACE_UINT8 tc:1;
    /*Is it authoritative Answer*/
    ACE_UINT8 aa:1;
    /*Query Type.*/
    ACE_UINT8 opcode:4;
    /*request(0), response(1)*/
    ACE_UINT8 qr:1;

    /*Response Code*/
    ACE_UINT8 rcode:4;
    /*RFU*/
    ACE_UINT8 z:3;
    /*Recursion Available.*/
    ACE_UINT8 ra:1;
    /*number of entries in Query section.*/
    ACE_UINT16 qdcount;
    /*Number of RR (Resource Record) in Answer Section.*/
    ACE_UINT16 ancount;
    /*Number of Name Server Record.*/
    ACE_UINT16 nscount;
    /*Resource Record in Additional Section.*/
    ACE_UINT16 arcount;

  }__attribute__((packed))DNS;

};

















#endif /*__COMMONIF_H__*/
