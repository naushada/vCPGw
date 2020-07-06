#ifndef __DHCP_SERVER_USER_CC__
#define __DHCP_SERVER_USER_CC__

#include "ace/Basic_Types.h"
#include "ace/Log_Msg.h"
#include "ace/Reactor.h"

#include "CPGateway.h"
#include "DhcpServerUser.h"
#include "CommonIF.h"
#include "DhcpServer.h"
#include "DhcpCommon.h"

#include "DhcpServerStateDecline.h"
#include "DhcpServerStateDiscover.h"
#include "DhcpServerStateInit.h"
#include "DhcpServerStateIPAllocated.h"
#include "DhcpServerStateLeaseExpire.h"
#include "DhcpServerStateRelease.h"
#include "DhcpServerStateRequest.h"
#include "DhcpServerStateRequestAck.h"

DhcpServerUser::DhcpServerUser(CPGateway *parent)
{
  m_cpGw = parent;
  m_instMap.unbind_all();
  m_sessMap.unbind_all();

  populateIPPool(ntohl(cpGw().DHCPConfInst().subnetMask()),
                 ntohl(cpGw().DHCPConfInst().startIP()),
                 ntohl(cpGw().DHCPConfInst().endIP()));

  /*Point to the first element of the list.*/
  m_selectedIP = m_ipPoolList.begin();
}

ACE_UINT32 DhcpServerUser::getIPFromPool(void)
{
  ACE_UINT32 ip = 0;
  if(m_selectedIP != m_ipPoolList.end())
  {
    ip = *m_selectedIP;
    ++m_selectedIP;
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l The selected IP is 0x%X\n"), ip));
  return(ip);
}

void DhcpServerUser::populateIPPool(ACE_UINT32 mask, ACE_UINT32 sIP, ACE_UINT32 eIP)
{
  ACE_UINT32 sOffset = (~mask) & sIP;
  ACE_UINT32 eOffset = (~mask) & eIP;
  ACE_UINT8 octet4 = (sOffset >>  0) & 0b11111111;
  ACE_UINT8 octet3 = (sOffset >>  8) & 0b11111111;
  ACE_UINT8 octet2 = (sOffset >> 16) & 0b11111111;
  ACE_UINT8 octet1 = (sOffset >> 24) & 0b11111111;

  ACE_UINT8 cnt1;
  ACE_UINT8 cnt2;
  ACE_UINT8 cnt3;
  ACE_UINT8 cnt4;

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l mask 0x%X sIP 0x%X eIP 0x%X\n"), mask, sIP, eIP));

  if(!octet1 && !octet2 && !octet3)
  {
    for(cnt1 = octet4; cnt1 <= (eOffset & 0b11111111); cnt1++)
    {
      m_ipPoolList.push_back(sIP);
      sIP++;
    }
  }
  else if(!octet1 && !octet2)
  {
    ACE_UINT32 tmpSIP = sIP;
    for(cnt2 = octet3; cnt2 <= ((eOffset >> 8) & 0b11111111); cnt2++)
    {
      tmpSIP = sIP & 0xFFFF0000;

      tmpSIP += ((cnt2 & 0b11111111) << 8);
      for(cnt1 = octet4; cnt1 <= (eOffset & 0b11111111); cnt1++)
      {
        tmpSIP = tmpSIP + cnt1;
        m_ipPoolList.push_back(tmpSIP);
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l ipAddr 0x%X\n"), tmpSIP));
      }
    }
  }
  else if(!octet1)
  {
    ACE_UINT32 tmpSIP = sIP;
    for(cnt3 = octet2; cnt3 <= ((eOffset >> 16) & 0b11111111); cnt3++)
    {
      tmpSIP = sIP & 0xFF000000;

      tmpSIP += ((cnt3 & 0b11111111) << 16);
      for(cnt2 = octet3; cnt2 <= ((eOffset >> 8) & 0b11111111); cnt2++)
      {
        tmpSIP += ((cnt2 & 0b11111111) << 8);
        for(cnt1 = octet4; cnt1 <= eOffset & 0b11111111; cnt1++)
        {
          tmpSIP += cnt1;
          m_ipPoolList.push_back(tmpSIP);
        }
      }
    }
  }
  else
  {
    ACE_UINT32 tmpSIP = sIP;
    for(cnt4 = octet1; cnt4 <= ((eOffset >> 24) & 0b11111111); cnt4++)
    {
      tmpSIP = ((cnt4 & 0b11111111) << 24);
      for(cnt3 = octet2; cnt3 <= ((eOffset >> 16) & 0b11111111); cnt3++)
      {
        tmpSIP += ((cnt3 & 0b11111111) << 16);
        for(cnt2 = octet3; cnt2 <= ((eOffset >> 8) & 0b11111111); cnt2++)
        {
          tmpSIP += ((cnt2 & 0b11111111) << 8);
          for(cnt1 = octet4; cnt1 <= (eOffset & 0b11111111); cnt1++)
          {
            tmpSIP += (cnt1 & 0b11111111);
            m_ipPoolList.push_back(tmpSIP);
          }
        }
      }
    }
  }
}

DhcpServerUser::~DhcpServerUser()
{
  DHCP::Server *inst = NULL;
  DhcpServerInstMap_iter iter = m_instMap.begin();

  for(; iter != m_instMap.end(); iter++)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (DHCP::Server *)((*iter).int_id_);
    ACE_CString cha = (ACE_CString)((*iter).ext_id_);
    m_instMap.unbind(cha);
    delete inst;
  }

  IPPoolList_Iter_t it = m_ipPoolList.begin();
  for(; it != m_ipPoolList.end();)
  {
    it = m_ipPoolList.erase(it);
  }

  m_ipPoolList.clear();

  /*re-claim the heap memory now. of singelton instance.*/
  delete DhcpServerStateDiscover::get_instance();
  delete DhcpServerStateInit::get_instance();
  delete DhcpServerStateLeaseExpire::get_instance();
  delete DhcpServerStateRelease::get_instance();
  delete DhcpServerStateRequest::get_instance();
}

CPGateway &DhcpServerUser::cpGw(void)
{
  return(*m_cpGw);
}

ACE_UINT8 DhcpServerUser::isSubscriberFound(ACE_CString macAddress)
{
  if(m_instMap.find(macAddress) == -1)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l This client %X:"),  macAddress.c_str()[0] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "%X:",  macAddress.c_str()[1] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "%X:",  macAddress.c_str()[2] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "%X:",  macAddress.c_str()[3] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "%X:",  macAddress.c_str()[4] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "%X not found\n", macAddress.c_str()[5] & 0xFF));
    return(0);
  }

  return(1);
}

ACE_UINT8 DhcpServerUser::createSubscriber(ACE_CString macAddress)
{

  DHCP::Server *sess = NULL;
  ACE_NEW_NORETURN(sess, DHCP::Server());

  m_instMap.bind(macAddress, sess);

  return(0);
}

ACE_UINT8 DhcpServerUser::addSubscriber(DHCP::Server *sess, ACE_CString macAddress)
{
  /*let STL do the memory management for stack object.*/
  m_instMap.bind(macAddress, sess);

  return(0);
}

DHCP::Server *DhcpServerUser::getSubscriber(ACE_CString macAddress)
{
  DHCP::Server *sess = NULL;
  if(m_instMap.find(macAddress, sess) == -1)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l No session for client %X:"), macAddress.c_str()[0] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "%X:",  macAddress.c_str()[1] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "%X:",  macAddress.c_str()[2] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "%X:",  macAddress.c_str()[3] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "%X:",  macAddress.c_str()[4] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "%X\n", macAddress.c_str()[5] & 0xFF));
  }

  return(sess);
}

ACE_UINT8 DhcpServerUser::deleteSubscriber(ACE_CString cha)
{
  DHCP::Server *sess = NULL;
  if(m_instMap.find(cha, sess) != -1)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l subscriber is found for client %X:"), cha.c_str()[0] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "%X:", cha.c_str()[1] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "%X:", cha.c_str()[2] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "%X:", cha.c_str()[3] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "%X:", cha.c_str()[4] & 0xFF));
    ACE_DEBUG((LM_DEBUG, "%X\n",cha.c_str()[5] & 0xFF));
    m_instMap.unbind(cha);
    return(1);
  }

  return(0);
}

int DhcpServerUser::sendResponse(ACE_CString cha, ACE_Byte *in, ACE_UINT32 inLen)
{
  return(cpGw().sendResponse(cha, in, inLen));
}

/*
 * @brief This is the main method of processing the DHCP Packet
 *        This receives the Ethernet Packet.
 * @param in pointer to received byte buffer
 * @param total length of the received packet.
 * @return */
ACE_UINT32 DhcpServerUser::processRequest(ACE_Byte *in, ACE_UINT32 inLen)
{
   TransportIF::DHCP *dhcpHdr = (TransportIF::DHCP *)&in[sizeof(TransportIF::ETH) +
                                                         sizeof(TransportIF::IP) +
                                                         sizeof(TransportIF::UDP)];

   ACE_CString haddr((const char *)dhcpHdr->chaddr, TransportIF::ETH_ALEN);

   ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l DhcpServerUser::processRequest for ")));
   ACE_DEBUG((LM_DEBUG, "chaddr %X:",  haddr.c_str()[0] & 0xFF));
   ACE_DEBUG((LM_DEBUG, "%X:",  haddr.c_str()[1] & 0xFF));
   ACE_DEBUG((LM_DEBUG, "%X:",  haddr.c_str()[2] & 0xFF));
   ACE_DEBUG((LM_DEBUG, "%X:",  haddr.c_str()[3] & 0xFF));
   ACE_DEBUG((LM_DEBUG, "%X:",  haddr.c_str()[4] & 0xFF));
   ACE_DEBUG((LM_DEBUG, "%X\n", haddr.c_str()[5] & 0xFF));

   DHCP::Server *sess = NULL;

   /*Is this a subsequent Request?*/
   if(isSubscriberFound(haddr))
   {
     ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l subscriber is found\n")));
     sess = getSubscriber(haddr);
     /*Feed request to state machine.*/
     sess->getState().rx(*sess, in, inLen);
   }
   else
   {
     /*This is a new DHCP Packet.*/
     struct in_addr addr;
     addr.s_addr = cpGw().DHCPConfInst().serverIP();
     ACE_TCHAR *ipStr = inet_ntoa(addr);
     ACE_CString ip(ipStr);

     /*create a dhcp instance to handle this request.*/
     ACE_NEW_NORETURN(sess, DHCP::Server(this,
                                         cpGw().getMacAddress(),
                                         ip,
                                         cpGw().DHCPConfInst().serverName(),
                                         cpGw().DHCPConfInst().dnsName(),
                                         cpGw().DHCPConfInst().mtu(),
                                         cpGw().DHCPConfInst().leaseTime(),
                                         cpGw().DHCPConfInst().dnsIP(),
                                         cpGw().DHCPConfInst().subnetMask(),
                                         getIPFromPool()));

     addSubscriber(sess, haddr);
     /*Feed request to FSM.*/
     sess->getState().rx(*sess, in, inLen);
   }

   RFC2131::DhcpOption *elm = NULL;
   if(sess->optionMap().find(RFC2131::OPTION_HOST_NAME, elm) != -1)
   {
     /*Found, create one.*/
     ACE_Byte val[255];
     ACE_Byte len = 0;
     len = elm->getValue(val);
     ACE_CString hName((const char *)val, len);

     struct in_addr addr;
     addr.s_addr = htonl(sess->ipAddr());
     ACE_TCHAR *ip = nullptr;

     ip = inet_ntoa(addr);
     ACE_CString ipStr(ip, ACE_OS::strlen(ip));
     updateResolver(hName, ip);
     ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l IP Address %s for host %s\n"),
                ipStr.c_str(), hName.c_str()));
   }

   return(0);
}

/*
 * @brief  This is the hook method for application to process the timer expiry. This is invoked by
 *         ACE Framework upon expiry of timer.
 * @param  tv in sec and usec.
 * @param  argument which was passed while starting the timer.
 * @return 0 for success else for failure.
 */
ACE_HANDLE DhcpServerUser::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
  ACE_DEBUG((LM_DEBUG,ACE_TEXT("%D %M %N:%l DhcpServerUser::handle_timeout\n")));
  process_timeout(arg);
  return(0);
}

/*
 * @brief this member function is invoked to start the timer.
 * @param This is the duration for timer.
 * @param This is the argument passed by caller.
 * @param This is to denote the preodicity whether this timer is going to be periodic or not.
 * @return timer_id is return.
 * */
long DhcpServerUser::start_timer(ACE_UINT32 to,
                                 const void *act,
                                 ACE_Time_Value interval)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l DhcpServerUser::start_timer\n")));
  ACE_Time_Value delay(to,0);
  long tid = 0;

  tid = ACE_Reactor::instance()->schedule_timer(this,
                                                act,
                                                delay,
                                                interval/*After this interval, timer will be started automatically.*/);

  /*Timer Id*/
  return(tid);
}

void DhcpServerUser::stop_timer(long tId)
{
  ACE_TRACE(("DhcpServerUser::stop_timer\n"));
  ACE_Reactor::instance()->cancel_timer(tId);
}

void DhcpServerUser::reset_timer(long tId, ACE_UINT32 timeOutInSec)
{
  ACE_TRACE(("DhcpServerUser::stop_timer\n"));
  ACE_Time_Value to(timeOutInSec, 0);

  ACE_Reactor::instance()->reset_timer_interval(tId, to);
}

ACE_INT32 DhcpServerUser::process_timeout(const void *act)
{
  TIMER_ID *timerId = (TIMER_ID *)act;
  DHCP::Server *sess = nullptr;

  ACE_CString cha((const char *)timerId->chaddr(), timerId->chaddrLen());

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l DhcpServerUser::process_timeout\n")));

  if(isSubscriberFound(cha) && (sess = getSubscriber(cha)))
  {
    switch(timerId->timerType())
    {
    case DHCP::PURGE_TIMER_MSG_ID:
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l PURGE_TIMER_MSG_ID is expired tid %d\n"), timerId->tid()));
      /*Kick the state machine.*/
      sess->getState().guardTimerExpiry(*sess, act);
      deleteSubscriber(cha);

       /*delete DHCP::Server instance now.*/
      delete sess;
      break;

    case DHCP::LEASE_TIMER_MSG_ID:
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l LEASE_TIMER_MSG_ID is expired tid %d\n"), timerId->tid()));
      /*Kick the state machine.*/
      sess->getState().leaseTimerExpiry(*sess, (const void *)act);
      deleteSubscriber(cha);

      /*delete DHCP::Server instance now.*/
      delete sess;
      break;

    default:
      break;
    }
  }

  return(0);
}

void DhcpServerUser::addSession(ACE_UINT32 ipAddr, ACE_CString macAddr)
{

  if(isSubscriberFound(macAddr))
  {
    DHCP::Server *sess = getSubscriber(macAddr);
    /*IP address is updated into dhcp User.*/
    sess->ipAddr(ipAddr);
  }

  if(m_sessMap.find(ipAddr) == -1)
  {
    /*Not Found in thme map.*/
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l IP 0x%X not found in the m_sessMap\n"), ipAddr));
    m_sessMap.bind(ipAddr, macAddr);
  }
}

void DhcpServerUser::deleteSession(ACE_UINT32 ipAddr)
{
  ACE_CString mac;
  if(m_sessMap.find(ipAddr, mac) != -1)
  {
    /*Session is removed now.*/
    m_sessMap.unbind(ipAddr);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l session for IP %u is removed\n"), ipAddr));
  }
}

void DhcpServerUser::addResolver(ACE_CString hName, ACE_CString ip)
{
  ACE_CString ipStr;
  if(m_name2IPMap.find(hName, ipStr) == -1)
  {
    /*Not Found. Add into it.*/
    m_name2IPMap.bind(hName, ip);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l IP %s hName %s not found in the m_name2IPMap\n"),
               ip.c_str(), hName.c_str()));
  }
}

void DhcpServerUser::deleteResolver(ACE_CString hName)
{
  ACE_CString ipStr;
  if(m_name2IPMap.find(hName, ipStr) != -1)
  {
    m_name2IPMap.unbind(hName);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l Resolver hostName %s IP %u are removed\n"),
               hName.c_str(), ipStr.c_str()));
  }
}

void DhcpServerUser::updateResolver(ACE_CString hName, ACE_CString ip)
{
  ACE_CString ipStr;
  if(m_name2IPMap.find(hName, ipStr) != -1)
  {
    /*Found. Update it now.*/
    /*rebind - newValue, newValue, oldValue, oldValue.*/
    if(m_name2IPMap.rebind(hName, ip, hName, ipStr) == -1)
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l old IP %s old hName %s new IP %s new hName failed to update in m_name2IPMap\n"), 
               ipStr.c_str(), hName.c_str(), ip.c_str(), hName.c_str()));
    }
  }
  else
  {
    addResolver(hName, ip);
  }
}

ACE_Byte *DhcpServerUser::getResolverIP(ACE_CString &hName)
{
  ACE_CString ipStr;
  ACE_Byte *IP = NULL;

  if(m_name2IPMap.find(hName, ipStr) != -1)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l The hName %s IP Address %s\n"), hName.c_str(),
               ipStr.c_str()));

    ACE_NEW_NORETURN(IP, ACE_Byte[4]);
    IP[0] = ipStr.c_str()[0] & 0xFF;
    IP[1] = ipStr.c_str()[1] & 0xFF;
    IP[2] = ipStr.c_str()[2] & 0xFF;
    IP[3] = ipStr.c_str()[3] & 0xFF;
  }

  return(IP);
}


#endif /*__DHCP_SERVER_USER_CC__*/
