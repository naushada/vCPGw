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

#include "commonIF.h"
#include "CPGateway.h"
#include "CPGatewayState.h"
#include "CPGatewayStateActivated.h"

#include "DhcpServer.h"

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


DhcpServerUser &CPGateway::getDhcpServerUser(void)
{
  ACE_TRACE("CPGateway::getState\n");
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
  for(int idx = 0; idx < len; idx++)
  {
    if(!(idx%16))
      ACE_DEBUG((LM_DEBUG, "\n"));

    ACE_DEBUG((LM_DEBUG, "%0.2X ", dd[idx] & 0xFF));
  }

  ACE_DEBUG((LM_DEBUG, "%I CPGateway length %u\n", len));
  /*Check wheather CPGateway is administrative locked/error state.*/
  getState().processRequest(*this, (ACE_Byte *)m_mb->rd_ptr(), m_mb->length());

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

    ACE_DEBUG((LM_ERROR, "sa_data[0] 0x%X ", ifr.ifr_addr.sa_data[0] & 0xFF));
    ACE_DEBUG((LM_ERROR, "sa_data[1] 0x%X ", ifr.ifr_addr.sa_data[1] & 0xFF));
    ACE_DEBUG((LM_ERROR, "sa_data[2] 0x%X ", ifr.ifr_addr.sa_data[2] & 0xFF));
    ACE_DEBUG((LM_ERROR, "sa_data[3] 0x%X ", ifr.ifr_addr.sa_data[3] & 0xFF));
    ACE_DEBUG((LM_ERROR, "sa_data[4] 0x%X ", ifr.ifr_addr.sa_data[4] & 0xFF));
    ACE_DEBUG((LM_ERROR, "sa_data[5] 0x%X\n",ifr.ifr_addr.sa_data[5] & 0xFF));

    ACE_DEBUG((LM_DEBUG, "The MAC[0] 0x%0.2X ", m_macAddress.c_str()[0] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "The MAC[1] 0x%0.2X ", m_macAddress.c_str()[1] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "The MAC[2] 0x%0.2X ", m_macAddress.c_str()[2] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "The MAC[3] 0x%0.2X ", m_macAddress.c_str()[3] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "The MAC[4] 0x%0.2X ", m_macAddress.c_str()[4] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "The MAC[5] 0x%0.2X\n", m_macAddress.c_str()[5] & 0xFF));

    if(ACE_OS::ioctl(handle, SIOCGIFINDEX, &ifr) < 0)
    {
      ACE_ERROR((LM_ERROR, "%IRetrieval of ethernet Index failed for handle %d", handle));
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
      ACE_ERROR((LM_ERROR, "%ICreation of handle for RAW Socket Failed\n"));
      break;
    }

    ACE_OS::setsockopt(handle, SOL_SOCKET, TCP_NODELAY, &option, sizeof(option));
    ACE_OS::setsockopt(handle, SOL_SOCKET, SO_BROADCAST, &option, sizeof(option));
    ACE_OS::memset((void *)&ifr, 0, sizeof(ifr));
    ACE_OS::strncpy(ifr.ifr_name, m_ethInterface.c_str(), (IFNAMSIZ - 1));

    if(ACE_OS::ioctl(handle, SIOCGIFFLAGS, &ifr) < 0)
    {
      ACE_ERROR((LM_ERROR, "%IRetrieval of IFFLAG failed for handle %d intfName %s\n", handle, ifr.ifr_name));
      ACE_OS::close(handle);
      break;
    }

    ifr.ifr_flags |= IFF_PROMISC | IFF_NOARP;

    if(ACE_OS::ioctl(handle, SIOCSIFFLAGS, &ifr) < 0)
    {
      ACE_ERROR((LM_ERROR, "%ISeting of PROMISC and NOARP failed for handle %d\n", handle));
      ACE_OS::close(handle);
      break;
    }

    ACE_OS::memset((void *)&mr, 0, sizeof(mr));
    mr.mr_ifindex = get_index();
    mr.mr_type = PACKET_MR_PROMISC;

    if(ACE_OS::setsockopt(handle, SOL_PACKET, PACKET_ADD_MEMBERSHIP, (const char *)&mr, sizeof(mr)) < 0)
    {
      ACE_ERROR((LM_ERROR, "%IAdding membership failed for handle %d\n", handle));
      ACE_OS::close(handle);
      break;
    }

    ACE_OS::memset((void *)&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(TransportIF::ETH_P_ALL);
    sa.sll_ifindex = get_index();

    if(ACE_OS::bind(handle, (struct sockaddr *)&sa, sizeof(sa)) < 0)
    {
      ACE_ERROR((LM_ERROR, "%Ibind failed for handle %d\n", handle));
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

CPGateway::CPGateway(ACE_CString intfName, ACE_CString ip,
                     ACE_UINT8 entity, ACE_UINT8 instance,
                     ACE_CString nodeTag)
{
  ACE_TRACE("CPGateway::CPGateway\n");
  ethIntfName(intfName);
  ipAddr(ip);


  if(open() < 0)
  {
    ACE_ERROR((LM_ERROR, "%Iopen for ethernet Interface %s failed\n", intfName.c_str()));
  }

  ACE_NEW_NORETURN(m_dhcpUser, DhcpServerUser(this));
  /*Mske CPGateway state machine Activated State.*/
  setState(CPGatewayStateActivated::instance());
}

CPGateway::CPGateway(ACE_CString intfName)
{
  ACE_TRACE("CPGateway::CPGateway\n");

  m_ethInterface = intfName;

  ACE_NEW_NORETURN(m_dhcpUser, DhcpServerUser(this));

  if(open() < 0)
  {
    ACE_ERROR((LM_ERROR, "%Iopen for ethernet Interface %s failed\n", intfName.c_str()));
  }

  /*Make CPGateway state machine Activated State.*/
  setState(CPGatewayStateActivated::instance());
}

CPGateway::~CPGateway()
{
  ACE_TRACE("CPGateway::~CPGateway\n");
  delete m_dhcpUser;
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
  return(m_macAddress);
}

ACE_UINT8 CPGateway::start()
{
  ACE_TRACE("CPGateway::start\n");
  ACE_Time_Value to(5);
  ACE_CString ipAddrStr("127.0.0.1");
  ACE_CString nodetag("primary");


  ACE_Reactor::instance()->register_handler(this, ACE_Event_Handler::READ_MASK);

  while(1)
  {
    ACE_Reactor::instance()->handle_events(to);
  }

  return(0);
}

ACE_UINT8 CPGateway::stop()
{
  ACE_TRACE("CPGateway::stop\n");
  return(0);
}
int main(int argc, char *argv[])
{

  /*
   * argv[0] = ethernetInterfaceName
   * argv[1] = IPC IP Address = 127.0.0.1
   * argv[2] = entityID/facility
   * argv[3] = instanceId
   * argv[4] = nodeName
   * */
  CPGateway *cp = NULL;
  ACE_CString ipStr((const char *)argv[2]);
  ACE_CString nTag((const char *)argv[5]);

  ACE_NEW_NORETURN(cp, CPGateway(argv[1], ipStr,
                                 ACE_OS::atoi(argv[3]),
                                 ACE_OS::atoi(argv[4]),
                                 nTag));

  if(!cp->start())
  {
    ACE_ERROR((LM_ERROR, "%I CPGateway instantiation failed\n"));
    delete cp;
    return(-1);
  }

  return(0);
}

#endif /*__CPGATEWAY_CC__*/
