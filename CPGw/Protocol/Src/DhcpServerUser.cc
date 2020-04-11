#ifndef __DHCP_SERVER_USER_CC__
#define __DHCP_SERVER_USER_CC__

#include "ace/Basic_Types.h"
#include "ace/Log_Msg.h"
#include "ace/Reactor.h"

#include "CPGateway.h"
#include "DhcpServerUser.h"
#include "CommonIF.h"
#include "DhcpServer.h"

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
  guardTid(0);
  leaseTid(0);
  m_instMap.unbind_all();
  m_sessMap.unbind_all();
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
}

CPGateway &DhcpServerUser::cpGw(void)
{
  return(*m_cpGw);
}

ACE_UINT8 DhcpServerUser::isSubscriberFound(ACE_CString macAddress)
{
  if(m_instMap.find(macAddress) == -1)
  {
    ACE_DEBUG((LM_ERROR, "This client %X:",  macAddress.c_str()[0] & 0xFF));
    ACE_DEBUG((LM_ERROR, "%X:",  macAddress.c_str()[1] & 0xFF));
    ACE_DEBUG((LM_ERROR, "%X:",  macAddress.c_str()[2] & 0xFF));
    ACE_DEBUG((LM_ERROR, "%X:",  macAddress.c_str()[3] & 0xFF));
    ACE_DEBUG((LM_ERROR, "%X:",  macAddress.c_str()[4] & 0xFF));
    ACE_DEBUG((LM_ERROR, "%X not found\n", macAddress.c_str()[5] & 0xFF));
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
    ACE_DEBUG((LM_ERROR, "No session for client %s is found\n", macAddress.c_str()));
  }

  return(sess);
}

int DhcpServerUser::sendResponse(ACE_CString cha, ACE_Byte *in, ACE_UINT32 inLen)
{
  return(cpGw().sendResponse(cha, in, inLen));
}

ACE_UINT32 DhcpServerUser::processRequest(ACE_Byte *in, ACE_UINT32 inLen)
{
   TransportIF::DHCP *dhcpHdr = (TransportIF::DHCP *)&in[sizeof(TransportIF::ETH) +
                                                         sizeof(TransportIF::IP) +
                                                         sizeof(TransportIF::UDP)];

   ACE_CString haddr((const char *)dhcpHdr->chaddr, TransportIF::ETH_ALEN);

   ACE_DEBUG((LM_DEBUG, "chaddr %X:",  haddr.c_str()[0] & 0xFF));
   ACE_DEBUG((LM_DEBUG, "%X:",  haddr.c_str()[1] & 0xFF));
   ACE_DEBUG((LM_DEBUG, "%X:",  haddr.c_str()[2] & 0xFF));
   ACE_DEBUG((LM_DEBUG, "%X:",  haddr.c_str()[3] & 0xFF));
   ACE_DEBUG((LM_DEBUG, "%X:",  haddr.c_str()[4] & 0xFF));
   ACE_DEBUG((LM_DEBUG, "%X\n", haddr.c_str()[5] & 0xFF));

   if(isSubscriberFound(haddr))
   {
     ACE_DEBUG((LM_INFO, "%I subscriber is found\n"));
     DHCP::Server *sess = getSubscriber(haddr);
     sess->getState().rx(*sess, in, inLen);
   }
   else
   {
     DHCP::Server *sess = NULL;
     ACE_NEW_NORETURN(sess, DHCP::Server(this, cpGw().getMacAddress()));

     addSubscriber(sess, haddr);
     sess->getState().rx(*sess, in, inLen);
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
ACE_INT32 DhcpServerUser::handle_timeout(ACE_Time_Value &tv, const void *arg)
{
  ACE_TRACE(("DhcpServerUser::handle_timeout\n"));
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
  ACE_TRACE(("DhcpServerUser::start_timer\n"));
  ACE_Time_Value delay(to);
  long tid = 0;

  tid = ACE_Reactor::instance()->schedule_timer(this,
                                                act,
                                                delay,
                                                interval/*After this interval, timer will be started automatically.*/);

  /*Timer Id*/
  return(tid);
}

long DhcpServerUser::guardTid(void)
{
  return(m_guardTid);
}

void DhcpServerUser::guardTid(long gTid)
{
  m_guardTid = gTid;
}

long DhcpServerUser::leaseTid(void)
{
  return(m_leaseTid);
}

void DhcpServerUser::leaseTid(long lTid)
{
  m_leaseTid = lTid;
}

void DhcpServerUser::stop_timer(long tId)
{
  ACE_TRACE(("DhcpServerUser::stop_timer\n"));
  ACE_Reactor::instance()->cancel_timer(tId);
}

ACE_INT32 DhcpServerUser::process_timeout(const void *act)
{
  ACE_TRACE("DhcpServerUser::process_timeout\n");
  TIMER_ID *timerId = (TIMER_ID *)act;

  DHCP::Server *sess = NULL;
  ACE_CString cha((const char *)timerId->chaddr());
  if(m_instMap.find(cha, sess) != -1)
  {
    switch(timerId->timerType())
    {
    case DHCP::EXPECTED_REQUEST_GUARD_TIMER_ID:
      ACE_DEBUG((LM_DEBUG, "EXPECTED_REQUEST_GUARD_TIMER_ID is expired\n"));
      /*Kick the state machine.*/
      sess->getState().guardTimerExpiry(*sess, (const void *)act);
      m_instMap.unbind(cha);

      /*re-claim the heap memory now.*/
      delete DhcpServerStateDiscover::instance();
      delete DhcpServerStateInit::instance();
      delete DhcpServerStateLeaseExpire::instance();
      delete DhcpServerStateRelease::instance();
      delete DhcpServerStateRequest::instance();
      delete sess;
      break;

    case DHCP::LEASE_GUARD_TIMER_ID:
      ACE_DEBUG((LM_DEBUG, "LEASE_GUARD_TIMER_ID is expired\n"));
      /*Kick the state machine.*/
      sess->getState().leaseTimerExpiry(*sess, (const void *)act);
      m_instMap.unbind(cha);

      /*re-claim the heap memory now.*/
      delete DhcpServerStateDiscover::instance();
      delete DhcpServerStateInit::instance();
      delete DhcpServerStateLeaseExpire::instance();
      delete DhcpServerStateRelease::instance();
      delete DhcpServerStateRequest::instance();
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
    ACE_DEBUG((LM_DEBUG, "IP %u not found in the m_sessMap\n", ipAddr));
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
    ACE_DEBUG((LM_DEBUG, "session for IP %u is removed\n", ipAddr));
  }
}

#endif /*__DHCP_SERVER_USER_CC__*/
