#ifndef __IPC_CC__
#define __IPC_CC__

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
#include "Ipc.h"

UniIPC::~UniIPC()
{
	m_magic = 0x00000000;
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

UniIPC::UniIPC(ACE_CString ipAddr, ACE_UINT8 fac,
           ACE_UINT8 ins, ACE_CString node_tag)
{
  do
  {
    ACE_TRACE("UniIPC::UniIPC");

    selfProcId(CommonIF::get_hash32(reinterpret_cast<const ACE_UINT8 *>(node_tag.c_str())));
    m_nodeTag = node_tag;


    selfTaskId(CommonIF::get_task_id(fac, ins));
    m_ipAddr = ipAddr;

    ipcPort(CommonIF::get_ipc_port(fac, ins));
    facility(fac);
    instance(ins);

    m_ipcAddr.set_port_number(ipcPort());
    m_ipcAddr.set_address(m_ipAddr.rep(), m_ipAddr.length());

    if(m_dgram.open(m_ipcAddr, 1) < 0)
    {
      ACE_ERROR((LM_ERROR, "IPC Socket Creation Failed for 0x%X", m_ipcPort));
      break;
    }

    handle(m_dgram.get_handle());
    ACE_Reactor::instance()->register_handler(this,
                                              ACE_Event_Handler::READ_MASK);

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

void UniIPC::nodeTag(ACE_CString node_tag)
{
  m_nodeTag = node_tag;
}

ACE_CString UniIPC::nodeTag(void)
{
  return(m_nodeTag);
}


/*
 * @brief  This is the hook method of ACE Event Handler and is called by ACE Framework to retrieve the
 *         handle. The handle is nothing but it's fd - file descriptor.
 * @param  none
 * @return handle of type ACE_HANDLE
 */
ACE_HANDLE UniIPC::get_handle(void) const
{
  return(const_cast<UniIPC *>(this)->handle());
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
	  memset((void *)mb->wr_ptr(), 0, (CommonIF::SIZE_64MB * sizeof(char)));
    if((recv_len = m_dgram.recv(mb->wr_ptr(), (CommonIF::SIZE_64MB * sizeof(char)), peer)) < 0)
    {
      ACE_ERROR((LM_ERROR, "Receive from peer 0x%X Failed\n", peer.get_port_number()));
      break;
    }

    mb->wr_ptr(recv_len);
    /*! peer is remembered and shall be used while sending response to it.*/

    /* +--------------+--------------+---------------+--------------+------------+-----------+-------------------+
     * |dstProcId(4)  | dsttaskId(4) | srcProcId(4)  | srcTaskId(4) | version(4) | length(4) | payload of length |
     * +--------------+--------------+---------------+--------------+------------+-----------+-------------------+
     * */
    handle_ipc((ACE_UINT8 *)mb->wr_ptr(), mb->length());
    mb->release();

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
ACE_UINT32 UniIPC::send_ipc(ACE_UINT32 dstProcId, ACE_UINT8 dstEntity,
		                      ACE_UINT8 dstInst, ACE_Byte *req,
						              ACE_UINT32 reqLen)
{
  ACE_TRACE("UniIPC::send_ipc");

  ACE_UINT32 len = 0;
	do
	{
	  ACE_Message_Block *mb;
    /*UDP socket for IPC.*/
    ACE_INET_Addr to;

    /* +--------------+--------------+---------------+--------------+------------+-----------+-------------------+
     * |dstProcId(4)  | dsttaskId(4) | srcProcId(4)  | srcTaskId(4) | version(4) | length(4) | payload of length |
     * +--------------+--------------+---------------+--------------+------------+-----------+-------------------+
     * */
    ACE_DEBUG((LM_DEBUG, "dstProcId %u srcProcId %u", dstProcId, get_self_procId()));
	  if(dstProcId == get_self_procId())
	  {
      /*data to be sent to different Node/Processor.*/
      to.set_port_number(CommonIF::get_ipc_port(dstEntity, dstInst));
      to.set_address(m_ipAddr.rep(), m_ipAddr.length());
      ACE_NEW_NORETURN(mb, ACE_Message_Block(CommonIF::SIZE_64MB));

      *((ACE_UINT32 *)mb->wr_ptr()) = dstProcId;
      mb->wr_ptr(sizeof(ACE_UINT32));
      *((ACE_UINT32 *)mb->wr_ptr()) = CommonIF::get_task_id(dstEntity, dstInst);
      mb->wr_ptr(sizeof(ACE_UINT32));
      *((ACE_UINT32 *)mb->wr_ptr()) = get_self_procId();
      mb->wr_ptr(sizeof(ACE_UINT32));
      *((ACE_UINT32 *)mb->wr_ptr()) = get_self_taskId();
      mb->wr_ptr(sizeof(ACE_UINT32));
      *((ACE_UINT32 *)mb->wr_ptr()) = 0x00000000;
      mb->wr_ptr(sizeof(ACE_UINT32));
      *((ACE_UINT32 *)mb->wr_ptr()) = (reqLen + 20);
      mb->wr_ptr(sizeof(ACE_UINT32));

      mb->copy((const char *)req, reqLen);

      len = m_dgram.send(mb->wr_ptr(), mb->length(), to);
      break;
	  }

	}while(0);

  return(len);
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























#endif /*__IPC_CC__*/
