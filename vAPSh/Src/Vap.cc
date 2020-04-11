#ifndef __VAP_CC__
#define __VAP_CC__

#include "Vap.h"

CtrlIF *CtrlIF::m_instance = NULL;

CtrlIF *CtrlIF::instance()
{
  if(NULL == m_instance)
  {
    m_instance = new CtrlIF();
  }

  return(m_instance);
}

CtrlIF::CtrlIF()
{
  do
  {
    if(-1 == m_unixAddr.set("/var/run/vapgCtrl"))
    {
      ACE_ERROR((LM_ERROR, "Setting of Address Failed\n"));
      break;
    }

    #ifdef DEBUG
    ACE_OS::unlink(m_unixAddr.get_path_name());
    struct sockaddr_un *ss = (struct sockaddr_un *)m_unixAddr.get_addr();
    ACE_DEBUG((LM_DEBUG, "family %d path %s\n", ss->sun_family, ss->sun_path));
    #endif

    ACE_OS::unlink(m_unixAddr.get_path_name());
    if(-1 == m_unixDgram.open(m_unixAddr, PF_UNIX))
    {
      ACE_ERROR((LM_ERROR,"Unix Socket Creation Failed\n"));
      break;
    }

    handle(m_unixDgram.get_handle());
    /*Note: Right after registering handler, ACE Framework calls get_handle
            to retrieve the handle. The handle is nothing but a fd
            (File Descriptor).*/
    ACE_Reactor::instance()->register_handler(this,
				                                      ACE_Event_Handler::READ_MASK);

  }while(0);
}

/*
 * @brief  This is the hook method of ACE Event Handler and is called by ACE Framework to retrieve the
 *         handle. The handle is nothing but it's fd - file descriptor.
 * @param  none
 * @return handle of type ACE_HANDLE
 */
ACE_HANDLE CtrlIF::get_handle(void) const
{
  return(const_cast<CtrlIF *>(this)->handle());
}

ACE_HANDLE CtrlIF::handle(void)
{
  return(m_handle);
}

void CtrlIF::handle(ACE_HANDLE handle)
{
  m_handle = handle;
}

/*
 * @brief  This is the hook method for application to define this member function and is invoked by 
 *         ACE Framework.
 * @param  handle in which read/recv/recvfrom to be called.
 * @return 0 for success else for failure.
 */
int CtrlIF::handle_input(ACE_HANDLE handle)
{
  char buff[1024];
  size_t len = sizeof(buff);

  memset((void *)buff, 0, sizeof(buff));
  do
  {
    /*UNIX socket for IPC.*/
    ACE_UNIX_Addr peer;
    int recv_len = -1;

    if((recv_len = m_unixDgram.recv(buff, len, peer)) < 0)
    {
      ACE_ERROR((LM_ERROR, "Receive from peer failed\n"));
      break;
    }

    ACE_DEBUG((LM_DEBUG, "%s", buff));
    processCtrlIFReq(buff, recv_len);

    if(-1 == m_unixDgram.send("OK", 2, peer))
    {
      ACE_ERROR((LM_ERROR, "Send to Peer Failed\n"));
      break;
    }

  }while(0);

  return(0);
}

int CtrlIF::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
{
  return(0);
}

void CtrlIF::processCtrlIFReq(char *req, size_t len)
{
  char cmdName[64];
  char cmdParam[1024];

  memset((void *)cmdName, 0, sizeof(cmdName));
  memset((void *)cmdParam, 0, sizeof(cmdParam));
  /*! scanning stops when space is encountered.*/
  sscanf(req, "%s", cmdName);

  if(!strncmp(cmdName, "SET", strlen(cmdName)))
  {
    /*SET Command followed by Param.*/
    parseSetParam(&req[4]);
  }
  else if(!strncmp(cmdName, "GET", strlen(cmdName)))
  {
    /*! GET Command*/
  }
  else if(!strncmp(cmdName, "MODIFY", strlen(cmdName)))
  {
    /*! MODIFY Command*/
  }
  else if(!strncmp(cmdName, "SAVE", strlen(cmdName)))
  {
    /*! SAVE Command*/
  }
  else if(!strncmp(cmdName, "LOAD", strlen(cmdName)))
  {
    /*! LOAD Command*/
  }
  else
  {
    /*! Invalid Command.*/
  }
}

void CtrlIF::parseSetParam(char *req)
{
  const char *delim = " ";
  char *ctx;
  char *tkn = NULL;

  tkn = strtok_r(req, delim, &ctx);

  while(tkn != NULL)
  {
    char param[64];
    char value[128];
    memset((void *)param, 0, sizeof(param));
    memset((void *)value, 0, sizeof(value));

    sscanf(tkn, "%s=%s", param, value);
    std::string key(param);
    std::string val(value);

    m_ctrlIFParam.insert(make_pair(key, val));
    tkn = strtok_r(NULL, delim, &ctx);
  }

  m_isConfigDone = true;

}

CtrlIF::~CtrlIF()
{

}

/** @brief This function sets the provided ip address and subnet mask to the interface
 *
 *  @param interface_name is the eth name
 *  @param ip_addr is the ip address to be set
 *  @param netmask_addr is the subnet mask to be set
 *
 *  @return upon success it returns 0 else < 0
 */
ACE_INT32 Vap::setIpAddr(ACE_CString &interface_name,
                         ACE_CString &ip_addr,
                         ACE_CString &netmask)
{
  ACE_HANDLE fd;
  struct ifreq ifr;

  fd = socket(AF_INET, SOCK_DGRAM, 0);

  memset((void *)&ifr, 0, sizeof(struct ifreq));
  strncpy((char *)ifr.ifr_name, (const char *)interface_name.rep(), IFNAMSIZ - 1);

  ifr.ifr_addr.sa_family = AF_INET;
  ifr.ifr_dstaddr.sa_family = AF_INET;
  ifr.ifr_netmask.sa_family = AF_INET;

  /*Make sure to null terminate*/
  ifr.ifr_name[IFNAMSIZ-1] = 0;

  ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = inet_addr(ip_addr.rep());

  if(ioctl(fd, SIOCSIFADDR, (void *) &ifr) < 0)
  {
    ACE_ERROR((LM_ERROR, "Setting of interface address failed\n"));
    close(fd);
    return(-1);
  }

  if(netmask.length())
  {
    ((struct sockaddr_in *) &ifr.ifr_netmask)->sin_addr.s_addr = inet_addr(netmask.rep());

    if(ioctl(fd, SIOCSIFNETMASK, (void *) &ifr) < 0)
    {
      ACE_ERROR((LM_ERROR, "%s:%d Setting of interface NETMASK failed\n", __FILE__, __LINE__));
      perror("netmask failed");
      close(fd);
      return(-2);
    }
  }

  close(fd);
  return(0);
}

/** @brief This function opens the ethernet interface for receiving ether net frame
 *
 *  @param none
 *  @return upon success it returns 0 else < 0
 */
ACE_INT32 Vap::openInterface(void)
{
  ACE_HANDLE fd = -1;
  ACE_INT32 option = 0;
  struct ifreq ifr;
  struct sockaddr_ll sa;

  /*RAW ethernet Socket*/
  fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

  if(fd < 0)
  {
    ACE_ERROR((LM_ERROR, "open of socket failed\n"));
    return(-1);
  }

  option = 1;

  setsockopt(fd, SOL_SOCKET, TCP_NODELAY,
             &option, sizeof(option));

  /*Enable to receive/Transmit Broadcast Frame*/
  option = 1;
  setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
             &option, sizeof(option));

  /* Set interface in promisc mode */
  struct packet_mreq mr;

  memset((void *)&ifr, 0, sizeof(ifr));
  strncpy((char *)ifr.ifr_name, (const char *)(intfName().rep()), IFNAMSIZ - 1);

  if(ioctl(fd, SIOCGIFFLAGS, &ifr) == -1)
  {
    ACE_ERROR((LM_ERROR, "%s: ioctl(SIOCGIFFLAGS)", strerror(errno)));
    close(fd);
    return(-2);
  }
  else
  {
    ifr.ifr_flags |= (IFF_PROMISC | IFF_NOARP);

    if(ioctl (fd, SIOCSIFFLAGS, &ifr) == -1)
    {
      ACE_ERROR((LM_ERROR, "%s: Could not set flag IFF_PROMISC", strerror(errno)));
      close(fd);
      return(-3);
    }
  }

  memset((void *)&mr, 0, sizeof(mr));
  mr.mr_ifindex = index();
  mr.mr_type    = PACKET_MR_PROMISC;

  if(setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP,
                (char *)&mr, sizeof(mr)) < 0)
  {
    close(fd);
    return(-4);
  }

  /* Bind to particular interface */
  memset((void *)&sa, 0, sizeof(sa));
  sa.sll_family   = AF_PACKET;
  sa.sll_protocol = htons(ETH_P_ALL);
  sa.sll_ifindex  = index();

  if(bind(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
  {
    ACE_ERROR((LM_ERROR, "%s: bind(sockfd=%d) failed", strerror(errno), fd));
    close(fd);
    return(-5);
  }

  /*Remember this HANDLE for future communication*/
  handle(fd);
  /*Upon SUCCESS*/
  return(0);
}

ACE_CString &Vap::intfName(void)
{
  return(m_intfName);
}

void Vap::intfName(ACE_CString &intfName)
{
  m_intfName = intfName;
}

void Vap::macAddress(ACE_CString &macAddr)
{
  m_macAddress = macAddr;
}

ACE_CString &Vap::macAddress(void)
{
  return(m_macAddress);
}

ACE_HANDLE Vap::handle(void)
{
  return(m_handle);
}

void Vap::handle(ACE_HANDLE fd)
{
  m_handle = fd;
}

void Vap::index(ACE_INT32 idx)
{
  m_index = idx;
}

ACE_INT32 Vap::index(void)
{
  return(m_index);
}

ACE_CString &Vap::ipAddress(void)
{
  return(m_ipAddress);
}

void Vap::ipAddress(ACE_CString &ipAddr)
{
  m_ipAddress = ipAddr;
}

ACE_CString &Vap::subnetMask(void)
{
  return(m_subnetMask);
}

void Vap::subnetMask(ACE_CString &subnetMask)
{
  m_subnetMask = subnetMask;
}

Vap::Vap(ACE_Thread_Manager *thrMgr,
         ACE_CString &_intfName,
         ACE_CString &_ipAddr,
         ACE_CString &_subnetMask,
         ACE_UINT8 container,
         ACE_UINT8 component,
         ACE_UINT8 facility,
         ACE_UINT8 instance) : ACE_Task<ACE_MT_SYNCH>(thrMgr)
{
  intfName(_intfName);
  ipAddress(_ipAddr);
  subnetMask(_subnetMask);
  index(0);
  handle(-1);
  m_magic = 0xDEADBEEF;
}

Vap::Vap()
{
  m_magic = 0xDEADBEEF;
}

Vap::~Vap()
{
}

ACE_INT32 Vap::init(void)
{
  ACE_HANDLE fd = -1;
  struct ifreq ifr;
  memset((void *)&ifr, 0, sizeof(struct ifreq));

  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, intfName().rep(), IFNAMSIZ - 1);

  fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if(fd < 0)
  {
    ACE_ERROR((LM_ERROR, "Creation of fd failed\n"));
    return(-1);
  }

  /*Retrieving Ethernet interface index*/
  if(ioctl(fd, SIOCGIFINDEX, &ifr))
  {
    ACE_ERROR((LM_ERROR, "Getting index failed\n"));
    close(fd);
    return(-2);
  }

  index(ifr.ifr_ifindex);

  /*Retrieving MAC Address*/
  if(ioctl(fd, SIOCGIFHWADDR, &ifr) < 0)
  {
    ACE_ERROR((LM_ERROR, "Getting MAC failed\n"));
    close(fd);
    return(-3);
  }

  m_macAddress.set(ifr.ifr_hwaddr.sa_data, ETH_ALEN);

  /*IPC - Configuration Details.*/
  return(0);
}

ACE_INT32 Vap::stop(void)
{
  /*De-register from ACE_Reactor Framework now.*/
  ACE_Reactor::instance()->remove_handler(this, ACE_Event_Handler::READ_MASK);

  if(handle() > 0)
  {
    close(handle());
  }

  index(-1);

  return(0);

}/*stop*/

ACE_INT32 Vap::start(void)
{
  openInterface();
  setIpAddr(intfName(), ipAddress(), subnetMask());

  ACE_Reactor::instance()->register_handler(this, ACE_Event_Handler::READ_MASK);

  /*! Time Out Value of 1sec.*/
  ACE_Time_Value to(1,0);
  while(1)
  {
    if(ACE_Reactor::instance()->handle_events(to) < 0)
    {
      ACE_ERROR((LM_ERROR, "handle_events failed\n"));
      break;
    }
  }

  return(0);
}/*start*/


/*
 * @brief  This is the hook method of ACE Event Handler and is called by ACE Framework to retrieve the
 *         handle. The handle is nothing but it's fd - file descriptor.
 * @param  none
 * @return handle of type ACE_HANDLE
 */
ACE_HANDLE Vap::get_handle(void) const
{
  return(const_cast<Vap *>(this)->handle());
}/*get_handle*/

/*
 * @brief  This member function is a hook method for ACE which will be called
 *         when a packet is received on bind socket without reading from socket.It's
 *         the implementer function which needs to receive from the socket.
 * @param  ACE_HANDLE it's nothing but a socket file descriptor.
 * @return upon sucess 0 else < 0.
 * */
ACE_INT32 Vap::handle_input(ACE_HANDLE handle)
{
  ACE_INT32 ret = -1;
  struct sockaddr_ll sa;
  ACE_UINT8 packet[1500];
  ACE_INT32 max_len = sizeof(packet);
  socklen_t addr_len = sizeof(sa);

  memset((void *)packet, 0, sizeof(packet));
  do
  {
    ret = recvfrom(handle,
                   packet,
                   max_len,
                   0,
                   (struct sockaddr *)&sa,
                    &addr_len);

  }while((ret == -1) && (errno == EINTR));

  /*broadcast MAC*/
  uint8_t bmac[ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  /*Pointer to Ethernet Packet*/
  struct ethhdr *eth_hdr_ptr = (struct ethhdr *)packet;
  /*Pointer to IP Packet*/
  struct iphdr *iphdr_ptr = (struct iphdr *)&packet[sizeof(struct ethhdr)];
  /*protocol could have any of value - 1 = ICMP; 2= IGMP; 6 = TCP; 17= UDP*/
  struct udphdr *udphdr_ptr = (struct udphdr *)&packet[sizeof(struct ethhdr) +
                                                       sizeof(struct iphdr)];

  if(0x0800/*ETH_P_IP*/ == ntohs(eth_hdr_ptr->h_proto))
  {
    /*Ethernet packet is followed by IP Packet*/
    if(!memcmp(eth_hdr_ptr->h_dest, bmac, ETH_ALEN))
    {
      /*It's a broadcast Packet*/
      if(17/*IP_UDP*/ == iphdr_ptr->protocol)
      {
        /*Check whether it's DHCP packet or not based on destination port*/
        if((67/*DHCP_SERVER_PORT*/ == ntohs(udphdr_ptr->dest)) &&
           (68/*DHCP_CLIENT_PORT*/ == ntohs(udphdr_ptr->source)))
        {
          /*! Process DHCP Messages from client */
        }
      }
    }
    else if(!memcmp(eth_hdr_ptr->h_dest, macAddress().c_str(), ETH_ALEN))
    {
      /*! Unicast Frame*/
      if(53 /*DNS PORT*/ == ntohs(udphdr_ptr->dest))
      {
        /*DNS Request*/
      }
      else if(17/*IP_UDP*/ == iphdr_ptr->protocol)
      {
        /*! Process UDP Frame */
      }
      else if(6/*IP_TCP*/ == iphdr_ptr->protocol)
      {
        /*! Process TCP Frame*/
      }
      else if(1/*IP_ICMP*/ == iphdr_ptr->protocol)
      {
        /*PING Request*/
      }
    }
    else
    {
      /*! Multical Packets.*/
    }
  }
  else if(0x0806/*ETH_P_ARP*/ == ntohs(eth_hdr_ptr->h_proto))
  {
    /*! Process ARP Request.*/
  }
  else if(0x888e/*ETH_P_EAPOL*/ == ntohs(eth_hdr_ptr->h_proto))
  {
    /*eapol - eap over LAN*/
  }
  else if(ntohs(eth_hdr_ptr->h_proto) <= 1500)
  {
    /*802.3 llc Frame*/
  }

  /*! Keep ACE Framework Happy.*/
  return(0);

}/*handle_input*/


int Vap::handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask)
{

  return(0);
}

int main(int argc, char *argv[])
{
  //Vap pVap = new Vap(ACE_Thread_Manager::instance())
  return(0);
}
#endif /* __VAP_CC__ */
