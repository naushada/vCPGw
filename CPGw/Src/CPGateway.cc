#ifndef __CPGATEWAY_CC__
#define __CPGATEWAY_CC__

#include <sys/time.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <linux/tcp.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "ace/config-lite.h"
#include "ace/Basic_Types.h"
#include "ace/Message_Block.h"
#include "ace/Reactor.h"
#include "ace/Event_Handler.h"
#include "ace/SString.h"
#include "ace/SOCK_Dgram.h"
#include "ace/Log_Msg.h"
#include "ace/INET_Addr.h"

#include "CommonIF.h"
#include "CPGateway.h"
#include "CPGatewayState.h"
#include "CPGatewayStateActivated.h"

#include "Arp.h"
#include "Dns.h"
#include "DhcpServer.h"

#include "CfgMgr.h"


void CPGateway::setState(CPGatewayState *st)
{
  ACE_TRACE("CPGateway::setState\n");

  if(m_state)
    getState().onExit(*this);

  m_state = st;
  getState().onEntry(*this);
}

CPGatewayState &CPGateway::getState(void)
{
  ACE_TRACE("CPGateway::getState\n");
  return(*m_state);
}

DNS::CPGwDns &CPGateway::getDnsUser(void)
{
  ACE_TRACE("CPGateway::getDnsUser\n");
  return(*m_dnsUser);
}

ARP::CPGwArp &CPGateway::getArpUser(void)
{
  ACE_TRACE("CPGateway::getArpUser\n");
  return(*m_arpUser);
}

DhcpServerUser &CPGateway::getDhcpServerUser(void)
{
  ACE_TRACE("CPGateway::getDhcpServerUser\n");
  return(*m_dhcpUser);
}

int CPGateway::sendResponse(ACE_CString chaddr, ACE_Byte *in, ACE_UINT32 inLen)
{
  struct sockaddr_ll sa;
  int sentLen = -1;
  ACE_UINT16 offset = 0;
  socklen_t addrLen = sizeof(sa);

  ACE_OS::memset((void *)&sa, 0, sizeof(sa));

  sa.sll_family = AF_PACKET;
  sa.sll_protocol = htons(TransportIF::ETH_P_ALL);
  sa.sll_ifindex = get_index();
  sa.sll_halen = TransportIF::ETH_ALEN;

  ACE_OS::memcpy((void *)sa.sll_addr, (void *)chaddr.c_str(), TransportIF::ETH_ALEN);

  do
  {
    sentLen = sendto(handle(), (const char *)&in[offset],
                     (inLen - offset),0, (struct sockaddr *)&sa,
                     addrLen);
    if(sentLen > 0)
    {
      offset += sentLen;
      if(!(inLen -offset))
      {
        sentLen = 0;
      }
    }
  }while((sentLen == -1) && (errno == EINTR));

  return(sentLen);
}

int CPGateway::handle_input(ACE_HANDLE fd)
{
  ACE_TRACE("CPGateway::handle_input\n");

  struct sockaddr_ll sa;
  int addr_len = sizeof(sa);
  ACE_INT32 len = -1;

  ACE_NEW_NORETURN(m_mb, ACE_Message_Block(CommonIF::SIZE_64MB));

  len = ACE_OS::recvfrom(fd, m_mb->wr_ptr(),
                         CommonIF::SIZE_64MB, 0, (struct sockaddr *)&sa, &addr_len);
  if(len < 0)
  {
    ACE_ERROR((LM_ERROR, "%Irecvfrom failed for handle %d\n", fd));
    return(0);
  }

  /*Update the length od data in m_mb*/
  m_mb->wr_ptr(len);

  char *dd = m_mb->rd_ptr();
#if 0
  for(int idx = 0; idx < len; idx++)
  {
    if(!(idx%16))
      ACE_DEBUG((LM_DEBUG, "\n"));

    ACE_DEBUG((LM_DEBUG, "%0.2X ", dd[idx] & 0xFF));
  }

  ACE_DEBUG((LM_DEBUG, "%I CPGateway length %u\n", len));
#endif

  /*Check wheather CPGateway is administrative locked/error state.*/
  getState().processRequest(*this, (ACE_Byte *)m_mb->rd_ptr(), m_mb->length());

  /*re-claim the allocated Memory now.*/
  delete m_mb;
  return(0);
}

ACE_INT32 CPGateway::get_index(void)
{
  ACE_HANDLE handle = -1;
  struct ifreq ifr;
  ACE_INT32 retStatus = CommonIF::FAILURE;

  ACE_TRACE("CPGateway::get_index\n");

  do
  {
    handle = ACE_OS::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(handle < 0)
    {
      ACE_ERROR((LM_ERROR, "%Isocket creation failed\n"));
      break;
    }

    ACE_OS::memset((void *)&ifr, 0, sizeof(struct ifreq));
    ifr.ifr_addr.sa_family = AF_INET;
    ACE_OS::strncpy(ifr.ifr_name, (const char *)m_ethInterface.c_str(), TransportIF::ETH_ALEN);

    if(ACE_OS::ioctl(handle, SIOCGIFHWADDR, &ifr) < 0)
    {
      ACE_ERROR((LM_ERROR, "%IMAC Address retrieval Failed for handle %d", handle));
      ACE_OS::close(handle);
      break;
    }

    m_macAddress.set(ifr.ifr_addr.sa_data, TransportIF::ETH_ALEN, 1);

    ACE_DEBUG((LM_ERROR, ACE_TEXT("%D %M %N:%l chaddr %X:"), ifr.ifr_addr.sa_data[0] & 0xFF));
    ACE_DEBUG((LM_ERROR, "%X:", ifr.ifr_addr.sa_data[1] & 0xFF));
    ACE_DEBUG((LM_ERROR, "%X:", ifr.ifr_addr.sa_data[2] & 0xFF));
    ACE_DEBUG((LM_ERROR, "%X:", ifr.ifr_addr.sa_data[3] & 0xFF));
    ACE_DEBUG((LM_ERROR, "%X:", ifr.ifr_addr.sa_data[4] & 0xFF));
    ACE_DEBUG((LM_ERROR, "%X\n",ifr.ifr_addr.sa_data[5] & 0xFF));

    if(ACE_OS::ioctl(handle, SIOCGIFINDEX, &ifr) < 0)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Retrieval of ethernet Index failed for handle %d\n"), handle));
      ACE_OS::close(handle);
      break;
    }

    ACE_OS::close(handle);
    retStatus = ifr.ifr_ifindex;

  }while(0);

  return(retStatus);
}

ACE_INT32 CPGateway::open(void)
{
  ACE_HANDLE handle = -1;
  const char option = 1;
  struct ifreq ifr;
  struct sockaddr_ll sa;
  struct packet_mreq mr;
  ACE_INT32 retStatus = CommonIF::FAILURE;

  do
  {
    handle = ACE_OS::socket(PF_PACKET, SOCK_RAW, htons(TransportIF::ETH_P_ALL));

    if(handle < 0)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Creation of handle for RAW Socket Failed\n")));
      break;
    }

    ACE_OS::setsockopt(handle, SOL_SOCKET, TCP_NODELAY, &option, sizeof(option));
    ACE_OS::setsockopt(handle, SOL_SOCKET, SO_BROADCAST, &option, sizeof(option));
    ACE_OS::memset((void *)&ifr, 0, sizeof(ifr));
    ACE_OS::strncpy(ifr.ifr_name, m_ethInterface.c_str(), (IFNAMSIZ - 1));

    if(ACE_OS::ioctl(handle, SIOCGIFFLAGS, &ifr) < 0)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Retrieval of IFFLAG failed for handle %d intfName %s\n"), handle, ifr.ifr_name));
      ACE_OS::close(handle);
      break;
    }

    ifr.ifr_flags |= IFF_PROMISC | IFF_NOARP;

    if(ACE_OS::ioctl(handle, SIOCSIFFLAGS, &ifr) < 0)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Seting of PROMISC and NOARP failed for handle %d\n"), handle));
      ACE_OS::close(handle);
      break;
    }

    ACE_OS::memset((void *)&mr, 0, sizeof(mr));
    mr.mr_ifindex = get_index();
    mr.mr_type = PACKET_MR_PROMISC;

    if(ACE_OS::setsockopt(handle, SOL_PACKET, PACKET_ADD_MEMBERSHIP, (const char *)&mr, sizeof(mr)) < 0)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Adding membership failed for handle %d\n"), handle));
      ACE_OS::close(handle);
      break;
    }

    ACE_OS::memset((void *)&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(TransportIF::ETH_P_ALL);
    sa.sll_ifindex = get_index();

    if(ACE_OS::bind(handle, (struct sockaddr *)&sa, sizeof(sa)) < 0)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l bind failed for handle %d\n"), handle));
      ACE_OS::close(handle);
      break;
    }

    set_handle(handle);
    retStatus = CommonIF::SUCCESS;

  }while(0);

  return(retStatus);
}

ACE_HANDLE CPGateway::handle()
{
  return(m_handle);
}

void CPGateway::set_handle(ACE_HANDLE handle)
{
  m_handle = handle;
}

ACE_HANDLE CPGateway::get_handle() const
{
  ACE_TRACE("CPGateway::get_handle");
  return(const_cast<CPGateway *>(this)->handle());
}

CPGateway::CPGateway(ACE_CString portName, ACE_CString ip)
{
  ACE_DEBUG((LM_DEBUG, "CPGateway::CPGateway\n"));
  ethIntfName(portName);
  ipAddr(ip);
}

/*Constructor*/
CPGateway::CPGateway(ACE_CString intfName, ACE_CString ip,
                     ACE_UINT8 entity, ACE_UINT8 instance,
                     ACE_CString nodeTag)
{
  ACE_TRACE("CPGateway::CPGateway\n");
  ethIntfName(intfName);
  ipAddr(ip);

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l IP %s len %u\n"), ipAddr().c_str(), ipAddr().length()));

  if(open() < 0)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l open for ethernet Interface %s failed\n"), intfName.c_str()));
  }

  struct in_addr adr;
  inet_aton(ipAddr().c_str(), &adr);

  ACE_NEW_NORETURN(m_dhcpUser, DhcpServerUser(this));

  /*Mske CPGateway state machine Activated State.*/
  setState(CPGatewayStateActivated::instance());

  /*Instantiating ARP instance.*/
  ACE_NEW_NORETURN(m_arpUser, ARP::CPGwArp(this, getMacAddress()));

  ACE_INET_Addr addr;
  addr.set_address((const char *)ipAddr().c_str(), ipAddr().length());
  char hname[255];
  addr.get_host_name(hname, sizeof(hname));

  ACE_CString hhname((const char *)hname);
  hostName(hhname);

  ACE_CString dName("balaagh.net");
  domainName(dName);


  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l IP Addr 0x%X\n"), adr.s_addr));
  /*Instantiating DNS instance.*/
  ACE_NEW_NORETURN(m_dnsUser, DNS::CPGwDns(this, getMacAddress(), hostName(),
                                           domainName(), adr.s_addr));

}

CPGateway::CPGateway(ACE_CString intfName)
{
  ACE_TRACE("CPGateway::CPGateway\n");

  m_ethInterface = intfName;

  ACE_NEW_NORETURN(m_dhcpUser, DhcpServerUser(this));

  if(open() < 0)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l open for ethernet Interface %s failed\n"), intfName.c_str()));
  }

  /*Make CPGateway state machine Activated State.*/
  setState(CPGatewayStateActivated::instance());
}

CPGateway::~CPGateway()
{
  ACE_TRACE("CPGateway::~CPGateway\n");
  delete m_dhcpUser;
  delete m_DHCPConfInst;
  m_DHCPConfInst = nullptr;
  m_dhcpUser = nullptr;
}

void CPGateway::ipAddr(ACE_CString ip)
{
  m_ipAddress = ip;
}

void CPGateway::ethIntfName(ACE_CString eth)
{
  m_ethInterface = eth;
}

ACE_CString &CPGateway::getMacAddress(void)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l MAC- %X:"), m_macAddress.c_str()[0] & 0xFF));
  ACE_DEBUG((LM_DEBUG, "%X:", m_macAddress.c_str()[1] & 0xFF));
  ACE_DEBUG((LM_DEBUG, "%X:", m_macAddress.c_str()[2] & 0xFF));
  ACE_DEBUG((LM_DEBUG, "%X:", m_macAddress.c_str()[3] & 0xFF));
  ACE_DEBUG((LM_DEBUG, "%X:", m_macAddress.c_str()[4] & 0xFF));
  ACE_DEBUG((LM_DEBUG, "%X\n", m_macAddress.c_str()[5] & 0xFF));
  return(m_macAddress);
}

ACE_CString &CPGateway::ipAddr(void)
{
  return(m_ipAddress);
}

ACE_CString &CPGateway::hostName(void)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l The Host Name is %s\n"), m_hostName.c_str()));
  return(m_hostName);
}

void CPGateway::hostName(ACE_CString hName)
{
  m_hostName = hName;
}

ACE_CString &CPGateway::domainName(void)
{
  return(m_domainName);
}

void CPGateway::domainName(ACE_CString dName)
{
  m_domainName = dName;
}

void CPGateway::inst(ACE_UINT8 ins)
{
  m_inst = ins;
}

ACE_UINT8 CPGateway::inst(void)
{
  return(m_inst);
}

ACE_UINT8 CPGateway::start()
{
  ACE_TRACE("CPGateway::start\n");

  /*Feed this instance to Reactor's loop*/
  ACE_Reactor::instance()->register_handler(this, ACE_Event_Handler::READ_MASK);

  return(0);
}

ACE_UINT8 CPGateway::stop()
{
  ACE_TRACE("CPGateway::stop\n");
  return(0);
}

int CPGateway::processConfigRsp(ACE_Byte *in, ACE_UINT32 inLen)
{

  struct in_addr addr;

  ACE_UINT8 ins = inst();

  if(ins > 0)
    ins -= 1;

  CommonIF::_cmMessage_t *rsp = (CommonIF::_cmMessage_t *)in;
  _CpGwConfigs_t *config = (_CpGwConfigs_t *)rsp->m_message;

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l DHCPInstCnt %u \n"), config->m_instance.m_DHCPInstCount));
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_nw %s \n"), config->m_instance.m_instDHCP[ins].m_nw.m_name));
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_type %s \n"), config->m_instance.m_instDHCP[ins].m_nw.m_type));
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_port %s \n"), config->m_instance.m_instDHCP[ins].m_nw.m_port));

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l DHCPAgentInstCnt %u \n"), config->m_instance.m_DHCPAgentInstCount));
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l HTTPInstCnt %u \n"), config->m_instance.m_HTTPInstCount));
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l AAAInstCnt %u \n"), config->m_instance.m_AAAInstCount));
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l CPGWInstCnt %u \n"), config->m_instance.m_CPGWInstCount));

  addr.s_addr = config->m_instance.m_instCPGW[ins].m_ip.m_ipn;
  ACE_TCHAR *ipStr = inet_ntoa(addr);

  ACE_CString ip((const char *)ipStr);
  ACE_CString intf((const char *)config->m_instance.m_instCPGW[ins].m_nw.m_port);
  ACE_CString hname((const char *)config->m_instance.m_instCPGW[ins].m_host_name);
  ACE_CString dname((const char *)config->m_instance.m_instDHCP[ins].m_profile.m_domain_name);

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l ipStr %s\n"), ipStr));
  ethIntfName(intf);
  ipAddr(ip);
  hostName(hname);
  domainName(dname);

  /*Populating DHCPConf now.*/
  ACE_NEW_NORETURN(m_DHCPConfInst, DHCPConf());

  DHCPConfInst().mtu(config->m_instance.m_instDHCP[ins].m_profile.m_mtu);
  DHCPConfInst().dnsIP(config->m_instance.m_instDHCP[ins].m_profile.m_ip.m_ipn);
  DHCPConfInst().leaseTime(config->m_instance.m_instDHCP[ins].m_profile.m_lease);
  DHCPConfInst().dnsName(dname);
  DHCPConfInst().subnetMask(config->m_instance.m_instDHCP[ins].m_mask.m_ipn);
  DHCPConfInst().serverIP(config->m_instance.m_instDHCP[ins].m_ip.m_ipn);
  DHCPConfInst().serverName(ACE_CString((const char *)config->m_instance.m_instDHCP[ins].m_host_name));
  DHCPConfInst().startIP(config->m_instance.m_instDHCP[ins].m_start_ip.m_ipn);
  DHCPConfInst().endIP(config->m_instance.m_instDHCP[ins].m_end_ip.m_ipn);

  if(open() < 0)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l opening of interface failed\n")));
  }

  ACE_NEW_NORETURN(m_dhcpUser, DhcpServerUser(this));

  /*Mske CPGateway state machine Activated State.*/
  setState(CPGatewayStateActivated::instance());

  /*Instantiating ARP instance.*/
  ACE_NEW_NORETURN(m_arpUser, ARP::CPGwArp(this, getMacAddress()));

  /*Instantiating DNS instance.*/
  ACE_NEW_NORETURN(m_dnsUser, DNS::CPGwDns(this, getMacAddress(),
                                           hostName(),
                                           domainName(),
                                           config->m_instance.m_instCPGW[ins].m_ip.m_ipn));
  if(start())
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l CPGateway instantiation failed\n")));
    return(-1);
  }

  return(0);
}

int CPGateway::processIpcMessage(ACE_Message_Block *mb)
{
  ACE_Byte *in = (ACE_Byte *)mb->rd_ptr();
  ACE_UINT32 len = (ACE_UINT32)mb->length();

  CommonIF::_cmMessage_t *msg = (CommonIF::_cmMessage_t *)in;

  ACE_UINT32 msgType = msg->m_msgType;
  ACE_UINT32 msgLen = msg->m_messageLen;

  switch(msgType)
  {
  case CommonIF::MSG_CFGMGR_CPGW_CONFIG_RSP:
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l Config Response Received length %u\n"),len));
    processConfigRsp(in, len);
    break;
  default:
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Unhandled Message Type %u\n"),
               msgType));
    break;
  }

  return(0);
}

void CPGateway::IPCIF(UniIPCIF *parent)
{
  m_IPCIF = parent;
}

UniIPCIF &CPGateway::IPCIF(void)
{
  return(*m_IPCIF);
}

DHCPConf &CPGateway::DHCPConfInst(void)
{
  return(*m_DHCPConfInst);
}

void CPGateway::DHCPConfInst(DHCPConf *inst)
{
  m_DHCPConfInst = inst;
}

int main(int argc, char *argv[])
{

  /*
   * argv[0] = Program/Binary Name
   * argv[1] = IPC IP Address = 127.0.0.1
   * argv[2] = instanceId
   * argv[3] = nodeName/NodeTag
   * argv[4] = portName (interface portName, eth0 or ens1)
   * */
  /*Start UniIPC Interface to get the CPGateway configuration.*/
  UniIPCIF *ipc = nullptr;

  ACE_CString ip(argv[1]);
  ACE_UINT8 ent = CommonIF::ENT_CPGW;
  ACE_UINT8 ins = ACE_OS::atoi(argv[2]);
  ACE_CString nodeTag(argv[3]);

  ACE_NEW_RETURN(ipc, UniIPCIF(ACE_Thread_Manager::instance(), ip, ent,
                               ins, nodeTag), -1);
  ipc->buildAndSendConfigReq();

  CPGateway *cp = nullptr;
  ACE_NEW_NORETURN(cp, CPGateway());

  /*There could be multiple copy of CPGateway instance.*/
  cp->inst(ins);

  /*Remember back pointer.*/
  ipc->CPGWIF(cp);
  cp->IPCIF(ipc);

  /*Resume the Daemon Thread*/
  ipc->resume();

  /*start the Reactor's main loop.*/
  ipc->start();

  return(0);
}

/*UniIPC related member function/method.*/

UniIPCIF::UniIPCIF(ACE_Thread_Manager *thrMgr, ACE_CString ip, ACE_UINT8 ent, ACE_UINT8 inst, ACE_CString nodeTag) :
  UniIPC(thrMgr, ip, ent, inst, nodeTag)
{
  ACE_Reactor::instance()->register_handler(this, ACE_Event_Handler::READ_MASK);

  /*Build and send Config Req to CFGMGR.*/
}

UniIPCIF::~UniIPCIF()
{
}

ACE_HANDLE UniIPCIF::get_handle() const
{
  ACE_TRACE("UniIPCIF::get_handle");
  return(const_cast<UniIPCIF *>(this)->handle());
}

ACE_UINT32 UniIPCIF::handle_ipc(ACE_Message_Block *mb)
{
  /*Post request to the Active Object.*/
  putq(mb);
  return(0);
}

ACE_UINT8 UniIPCIF::start(void)
{
  ACE_Time_Value to(5);

  while(1)
  {
    ACE_Reactor::instance()->handle_events(to);
  }

  return(0);
}

int UniIPCIF::svc(void)
{
  ACE_Message_Block *mb = nullptr;

  /*The control will be blocked if queue is empty.*/
  for(;-1 != getq(mb);)
  {
    /*Process the Command.*/
    if(mb->msg_type() == ACE_Message_Block::MB_HANGUP)
    {
      mb->release();
      break;
    }

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l dequeue equest\n")));

    /*Process the Command Now.*/
    CPGWIF().processIpcMessage(mb);
    /*reclaim the heap memory now. allocated by the sender*/
    mb->release();
  }

  return(0);
}

void UniIPCIF::CPGWIF(CPGateway *parent)
{
  m_CPGWIF = parent;
}

CPGateway &UniIPCIF::CPGWIF(void)
{
  return(*m_CPGWIF);
}

void UniIPCIF::buildAndSendConfigReq(void)
{
  ACE_Message_Block *mb = nullptr;
  ACE_NEW_NORETURN(mb, ACE_Message_Block(CommonIF::SIZE_1KB));

  CommonIF::_cmMessage_t *req = (CommonIF::_cmMessage_t *)mb->wr_ptr();
  req->m_dst.m_procId = get_self_procId();
  req->m_dst.m_entId = CommonIF::ENT_CFGMGR;
  req->m_dst.m_instId = CommonIF::INST1;

  req->m_src.m_procId = get_self_procId();
  req->m_src.m_entId = CommonIF::ENT_CPGW;
  req->m_src.m_instId = CommonIF::INST1;
  req->m_msgType = CommonIF::MSG_CPGW_CFGMGR_CONFIG_REQ;
  req->m_messageLen = 0;

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l messageType %u\n"), req->m_msgType));
  mb->wr_ptr(sizeof(CommonIF::_cmMessage_t));
  send_ipc((ACE_Byte *)mb->rd_ptr(), (ACE_UINT32)mb->length());
}

/*DHCPConf Section...*/
#if 0
DHCPConf *DHCPConf::m_instance = nullptr;

DHCPConf *DHCPConf::instance()
{
  if(nullptr == m_instance)
    ACE_NEW_NORETURN(m_instance, DHCPConf());

  return(m_instance);
}
#endif

ACE_UINT8 DHCPConf::mtu(void)
{
  return(m_mtu);
}

void DHCPConf::mtu(ACE_UINT8 m)
{
  m_mtu = m;
}

void DHCPConf::dnsIP(ACE_UINT32 ip)
{
  m_dnsIP = ip;
}

ACE_UINT32 DHCPConf::dnsIP(void)
{
  return(m_dnsIP);
}

void DHCPConf::leaseTime(ACE_UINT32 l)
{
  m_leaseTime = l;
}

ACE_UINT32 DHCPConf::leaseTime(void)
{
  return(m_leaseTime);
}

void DHCPConf::dnsName(ACE_CString d)
{
  m_dnsName = d;
}

ACE_CString &DHCPConf::dnsName(void)
{
  return(m_dnsName);
}

void DHCPConf::subnetMask(ACE_UINT32 s)
{
  m_subnetMask = s;
}

ACE_UINT32 DHCPConf::subnetMask(void)
{
  return(m_subnetMask);
}

void DHCPConf::serverIP(ACE_UINT32 ip)
{
  m_serverIP = ip;
}

ACE_UINT32 DHCPConf::serverIP(void)
{
  return(m_serverIP);
}

void DHCPConf::serverName(ACE_CString s)
{
  m_serverName = s;
}

ACE_CString &DHCPConf::serverName(void)
{
  return(m_serverName);
}

ACE_UINT32 DHCPConf::startIP(void)
{
  return(m_startIP);
}

void DHCPConf::startIP(ACE_UINT32 ip)
{
  m_startIP = ip;
}

void DHCPConf::endIP(ACE_UINT32 ip)
{
  m_endIP = ip;
}

ACE_UINT32 DHCPConf::endIP(void)
{
  return(m_endIP);
}

DHCPConf::~DHCPConf()
{
  //m_instance = nullptr;
}
#endif /*__CPGATEWAY_CC__*/
