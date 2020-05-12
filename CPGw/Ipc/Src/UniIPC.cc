#ifndef __UNIIPC_CC__
#define __UNIIPC_CC__

#include <ace/Reactor.h>
#include <ace/Basic_Types.h>
#include <ace/Event_Handler.h>
#include <ace/Task.h>
#include <ace/INET_Addr.h>
#include <ace/UNIX_Addr.h>
#include <ace/SOCK_Dgram.h>
#include <ace/LSOCK_CODgram.h>
#include <ace/LSOCK_Dgram.h>
#include <ace/Task_T.h>
#include <ace/UNIX_Addr.h>
#include <ace/Timer_Queue_T.h>

#include "CommonIF.h"
#include "UniIPC.h"

UniIPC::~UniIPC()
{
	m_magic = 0x00000000;
  m_sockDgram.close();
}

UniIPC::UniIPC()
{
  m_magic = 0x00000000;
  m_handle = -1;
  m_ipcAddr.set("");
  m_facility = 0;
  m_instance = 0;
  m_ipcPort = 0;
  m_ipAddr = 0;
  m_taskIdToPeerUMap.clear();

}

UniIPC::UniIPC(ACE_Thread_Manager *thrMgr, ACE_CString ipAddr,
               ACE_UINT8 fac, ACE_UINT8 ins,
               ACE_CString node_tag) : ACE_Task<ACE_MT_SYNCH>(thrMgr)
{
  do
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l UniIPC ")));

    selfProcId(CommonIF::get_hash32(reinterpret_cast<const ACE_UINT8 *>(node_tag.c_str())));
    m_nodeTag = node_tag;


    selfTaskId(CommonIF::get_task_id(fac, ins));
    m_ipAddr = ipAddr;

    ipcPort(CommonIF::get_ipc_port(fac, ins));
    facility(fac);
    instance(ins);

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l IPC Port %u\n"), m_ipcPort));
    m_ipcAddr.set_port_number(ipcPort());
    //m_ipcAddr.set_address(m_ipAddr.rep(), m_ipAddr.length());
    m_ipcAddr.set_address(m_ipAddr.c_str(), m_ipAddr.length());

    if(m_sockDgram.open(m_ipcAddr, 1) < 0)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l IPC Socket Creation Failed for 0x%X"), m_ipcPort));
      break;
    }

    handle(m_sockDgram.get_handle());

    /*Spwaning the Thread now.*/
    open();
  }while(0);

}/*UniIPC*/

void UniIPC::ipcPort(ACE_UINT16 port)
{
  m_ipcPort = port;
}

ACE_UINT16 UniIPC::ipcPort(void)
{
  return(m_ipcPort);
}

void UniIPC::facility(ACE_UINT8 fac)
{
  m_facility = fac;
}

ACE_UINT8 UniIPC::facility(void)
{
  return(m_facility);
}

void UniIPC::instance(ACE_UINT8 instance)
{
  m_instance = instance;
}

ACE_UINT8 UniIPC::instance(void)
{
  return(m_instance);
}

void UniIPC::handle(ACE_HANDLE handle)
{
  m_handle = handle;
}

ACE_HANDLE UniIPC::handle(void)
{
  return(m_handle);
}

ACE_UINT32 UniIPC::get_self_taskId(void)
{
  return(m_selfTaskId);
}

void UniIPC::selfTaskId(ACE_UINT32 tId)
{
  m_selfTaskId = tId;
}

void UniIPC::nodeTag(ACE_CString node_tag)
{
  m_nodeTag = node_tag;
}

ACE_CString UniIPC::nodeTag(void)
{
  return(m_nodeTag);
}

int UniIPC::open(void *args)
{
  activate(THR_NEW_LWP | THR_SUSPENDED);
  return(0);
}

/*
 * @brief  This is the hook method of ACE Event Handler and is called by ACE Framework to retrieve the
 *         handle. The handle is nothing but it's fd - file descriptor.
 * @param  none
 * @return handle of type ACE_HANDLE
 */
ACE_HANDLE UniIPC::get_handle(void) const
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l UniIPC handle %u\n"), m_handle));
  return(const_cast<UniIPC *>(this)->handle());
}

ACE_INT32 UniIPC::handle_signal(int signum)
{
  process_signal(signum);
}

/*
 * @brief  This is the hook method for application to define this member function and is invoked by
 *         ACE Framework.
 * @param  handle in which read/recv/recvfrom to be called.
 * @return 0 for success else for failure.
 */
ACE_INT32 UniIPC::handle_input(ACE_HANDLE handle)
{
  ACE_Message_Block *mb;
  size_t recv_len = -1;

  ACE_NEW_RETURN(mb, ACE_Message_Block(CommonIF::SIZE_64MB), -1);

  /*UDP socket for IPC.*/
  ACE_INET_Addr peer;
  do
  {
    ACE_OS::memset((void *)mb->wr_ptr(), 0, CommonIF::SIZE_64MB);
    if((recv_len = m_sockDgram.recv(mb->wr_ptr(), CommonIF::SIZE_64MB, peer)) < 0)
    {
      ACE_ERROR((LM_ERROR, "Receive from peer 0x%X Failed\n", peer.get_port_number()));
      break;
    }

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l recv_le is %u\n"), recv_len));
    /*Update the length now.*/
    mb->wr_ptr(recv_len);
    /*dispatch the hook method now.*/
    handle_ipc(mb);

  }while(0);

  return(0);
}

ACE_UINT32 UniIPC::get_self_procId(void)
{
	return(m_selfProcId);
}

void UniIPC::selfProcId(ACE_UINT32 selfProcId)
{
	m_selfProcId = selfProcId;
}

/*
 * @brief
 * @param dstProcId is the destination ProcId which is hash32 of processor ID/Node ID
 * @param
 * @param
 * @param
 * @return
 * */
ACE_UINT32 UniIPC::send_ipc(ACE_Byte *rsp, ACE_UINT32 rspLen)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l sendign message via IPC len %u\n"), rspLen));

  int bytesToSend = 0;
  int len = 0;
  int offset = 0;

  bytesToSend = (rspLen - offset);
	do
	{
    CommonIF::_cmMessage_t *cMsg = (CommonIF::_cmMessage_t *)rsp;

    /*UDP socket for IPC.*/
    ACE_INET_Addr to;

    ACE_DEBUG((LM_DEBUG, "dstProcId %u srcProcId %u\n", cMsg->m_dst.m_procId, get_self_procId()));

	  if(cMsg->m_dst.m_procId == get_self_procId())
	  {
      /*data to be sent to same Processor.*/
      to.set_port_number(CommonIF::get_ipc_port(cMsg->m_dst.m_entId, cMsg->m_dst.m_instId));
      to.set_address(m_ipAddr.c_str(), m_ipAddr.length());

      do
      {
        len = m_sockDgram.send(&rsp[offset], bytesToSend, to);
        if(len > 0)
        {
          offset += len;
          bytesToSend -= len;
        }
        else
        {
          ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l %m send Failed\n")));
          break;
        }

      }while(bytesToSend);
	  }

	}while(0);

  return(len);
}

ACE_UINT32 UniIPC::handle_ipc(ACE_Message_Block *mb)
{
  ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l not defined in sub-class\n")));
  return(0);
}

ACE_UINT32 UniIPC::process_signal(int signum)
{
  ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l proces_signal to be  re-defined in sub-class\n")));
  return(0);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Timer API definition
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
 * @brief  This is the hook method for application to process the timer expiry. This is invoked by
 *         ACE Framework upon expiry of timer.
 * @param  tv in sec and usec.
 * @param  argument which was passed while starting the timer.
 * @return 0 for success else for failure.
 */
ACE_INT32 UniTimer::handle_timeout(ACE_Time_Value &tv, const void *arg)
{
  ACE_TRACE(("UniTimer::handle_timeout"));
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
long UniTimer::start_timer(ACE_UINT32 to,
                         const void *act,
                         ACE_Time_Value interval = ACE_Time_Value::zero)
{
  ACE_TRACE(("UniTimer::start_timer"));
  ACE_Time_Value delay(to);
  long tid = 0;

  tid = ACE_Reactor::instance()->schedule_timer(this,
                                                act,
                                                delay,
                                                interval/*After this interval, timer will be started automatically.*/);

  /*Timer Id*/
  return(tid);
}

void UniTimer::stop_timer(long tId)
{
  ACE_TRACE((UniTimer::stop_timer));
  ACE_Reactor::instance()->cancel_timer(tId);
}

ACE_INT32 UniTimer::process_timeout(const void *act)
{
  /*! Derived class function should have been called.*/
  return(0);
}

UniTimer::UniTimer(): ACE_Event_Handler()
{
  ACE_TRACE("UniTimer::UniTimer\n");
  ACE_Reactor::instance()->register_handler(this,
                                            ACE_Event_Handler::TIMER_MASK);
}

UniTimer::~UniTimer()
{
}























#endif /*__UNIIPC_CC__*/
