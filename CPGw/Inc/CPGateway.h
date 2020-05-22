#ifndef __CPGATEWAY_H__
#define __CPGATEWAY_H__

#include "UniIPC.h"
#include "DhcpServerUser.h"
#include "CPGatewayState.h"

#include "ace/Event_Handler.h"
#include "ace/SOCK_Dgram.h"
#include "ace/SString.h"
#include "ace/Hash_Map_Manager.h"
#include "ace/Null_Mutex.h"

namespace ARP {class CPGwArp;}
namespace DNS {class CPGwDns;}

class DHCPConf
{
private:
  //static DHCPConf *m_instance;
  ACE_UINT8 m_mtu;
  ACE_UINT32 m_dnsIP;
  ACE_UINT32 m_leaseTime;
  ACE_CString m_dnsName;
  ACE_UINT32 m_subnetMask;
  ACE_UINT32 m_serverIP;
  ACE_CString m_serverName;
  ACE_UINT32 m_startIP;
  ACE_UINT32 m_endIP;
  ACE_UINT32 m_excludeIP[255];

public:

  DHCPConf() = default;
  ~DHCPConf();
  //static DHCPConf *instance();

  ACE_UINT8 mtu(void);
  void mtu(ACE_UINT8 m);

  ACE_UINT32 dnsIP(void);
  void dnsIP(ACE_UINT32 ip);

  ACE_UINT32 leaseTime(void);
  void leaseTime(ACE_UINT32 l);

  void dnsName(ACE_CString d);
  ACE_CString &dnsName(void);

  ACE_UINT32 subnetMask(void);
  void subnetMask(ACE_UINT32 s);

  void serverIP(ACE_UINT32 ip);
  ACE_UINT32 serverIP(void);

  void serverName(ACE_CString s);
  ACE_CString &serverName(void);

  void startIP(ACE_UINT32 ip);
  ACE_UINT32 startIP(void);

  void endIP(ACE_UINT32 ip);
  ACE_UINT32 endIP(void);

};

class UniIPCIF : public UniIPC
{
public:
  UniIPCIF(ACE_Thread_Manager *thrMgr, ACE_CString ip, ACE_UINT8 ent,
           ACE_UINT8 inst, ACE_CString nodeTag);

  virtual ~UniIPCIF();
  ACE_UINT32 handle_ipc(ACE_Message_Block *mb);
  virtual ACE_HANDLE get_handle() const;

  int svc(void);

  ACE_UINT8 start(void);
  void stop(void);
  int processIpcMessage(ACE_Message_Block *mb);
  void buildAndSendConfigReq(void);

  CPGateway &CPGWIF(void);
  void CPGWIF(CPGateway *cpGw);

private:
  CPGateway *m_CPGWIF;

};

class CPGateway : public ACE_Event_Handler
{
  private:
    ACE_UINT8 m_inst;
    /*Ethernet Interface Name.*/
    ACE_CString m_ethInterface;
    ACE_HANDLE m_handle;
    ACE_Message_Block *m_mb;
    /*subnetMask mask for interface.*/
    ACE_CString m_subnetMask;
    /*ip Address for interface.*/
    ACE_CString m_ipAddress;
    /*self macAddress*/
    ACE_CString m_macAddress;
    ACE_UINT32 m_intfIndex;
    ACE_SOCK_Dgram m_dgram;

    /*hostName of the Machine.*/
    ACE_CString m_hostName;
    /*Domain Name.*/
    ACE_CString m_domainName;

    /*State-Machine for CPGateway. Note This will point to pointer to sub-class*/
    CPGatewayState *m_state;

    /*Instance of dhcpUser.*/
    DhcpServerUser *m_dhcpUser;
    /*Instance of ARP.*/
    ARP::CPGwArp *m_arpUser;
    /*Instance of DNS.*/
    DNS::CPGwDns *m_dnsUser;
    /*Interface for IPC messages.*/
    UniIPCIF *m_IPCIF;

    DHCPConf *m_DHCPConfInst;

  public:

    virtual ~CPGateway();
    CPGateway() = default;

    CPGateway(ACE_CString intfName, ACE_CString ip);
    CPGateway(ACE_CString intfName);
    CPGateway(ACE_CString intfName, ACE_CString ipAddr,
              ACE_UINT8 entity, ACE_UINT8 instance,
              ACE_CString nodeTag);

    ACE_UINT8 start();
    ACE_UINT8 stop();

    virtual ACE_HANDLE get_handle() const;
    virtual int handle_input(ACE_HANDLE handle);
    ACE_INT32 open(void);
    ACE_INT32 get_index(void);
    ACE_HANDLE handle(void);
    void set_handle(ACE_HANDLE handle);

    void setState(CPGatewayState *sm);
    CPGatewayState &getState(void);

    void setDhcpServerUser(DhcpServerUser *du);
    DhcpServerUser &getDhcpServerUser(void);
    int sendResponse(ACE_CString chaddr, ACE_Byte *in, ACE_UINT32 inLen);

    void ipAddr(ACE_CString ip);
    ACE_CString &ipAddr(void);

    void subnetMask(ACE_CString mask);
    ACE_CString &subnetMask(void);

    void inst(ACE_UINT8 ins);
    ACE_UINT8 inst(void);

    void ethIntfName(ACE_CString eth);
    ACE_CString &ethIntfName(void);

    ARP::CPGwArp &getArpUser(void);
    DNS::CPGwDns &getDnsUser(void);

    ACE_CString &getMacAddress(void);

    ACE_CString &hostName(void);
    void hostName(ACE_CString hName);

    ACE_CString &domainName(void);
    void domainName(ACE_CString dName);

    void IPCIF(UniIPCIF *ipc);
    UniIPCIF &IPCIF(void);

    int processConfigRsp(ACE_Byte *in, ACE_UINT32 inLen);
    int processIpcMessage(ACE_Message_Block *mb);

    DHCPConf &DHCPConfInst(void);
    void DHCPConfInst(DHCPConf *inst);

    ACE_INT32 configureIPAddress(void);
};

#endif /*__CPGATEWAY_H__*/
