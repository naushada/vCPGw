#ifndef __CFG_CMD_HANDLER_CC__
#define __CFG_CMD_HANDLER_CC__

#include "CfgCmdHandler.h"
#include "ace/Event_Handler.h"
#include "ace/Null_Mutex.h"
#include "ace/Task.h"
#include "ace/Reactor.h"

#include "CommonIF.h"

CfgCmdHandler::CfgCmdHandler(ACE_Thread_Manager *mgr) :
  ACE_Task<ACE_MT_SYNCH>(mgr)
{
  do
  {
    if(-1 == m_myAddr.set("/var/run/.cliif"))
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l %P|%t local Unix domain socket creation failed\n")));
      break;
    }

    ACE_OS::unlink(m_myAddr.get_path_name());
    if(-1 == m_lSockDgram.open(m_myAddr))
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l %m opening of unixSocket Failed\n")));
      break;
    }

    /*learn the handle now.*/
    handle(m_lSockDgram.get_handle());

    /*Set the handle in out-put mode so that it can be sent to peer.*/
    ACE_Reactor::instance()->register_handler(this,
                                              ACE_Event_Handler::READ_MASK);

  }while(0);
}

CfgCmdHandler::~CfgCmdHandler()
{
  ACE_OS::unlink(m_myAddr.get_path_name());
  ACE_OS::close(handle());
}

int CfgCmdHandler::handle_output(ACE_HANDLE fd)
{
  int ret = 0;
  int bytesToSend = -1;
  int bytesSent = -1;
  ACE_TCHAR *data = nullptr;

  ACE_Message_Block *mb = nullptr;

  do
  {
    /*Response Queue is empty.*/
    if(m_rspQ.is_empty())
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%P|%t Response Queue is empty\n")));
      break;
    }

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l handle_output\n")));

    m_rspQ.dequeue_head(mb);
    /*starts sending the bytes now.*/
    if((ret = m_lSockDgram.send(mb->wr_ptr(), mb->length(), m_peerAddr)) < 0)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l %m send failed\n")));
    }

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l rsp %s length %u\n"), mb->rd_ptr(), mb->length()));
    /*reclaim the memory now.*/
    mb->release();

  }while(0);

  ACE_Reactor::instance()->remove_handler(this,
                                          ACE_Event_Handler::WRITE_MASK |
                                          /*DONT_CALL - don't call handle_close hook method.*/
                                          ACE_Event_Handler::DONT_CALL);

  return(ret);
}

int CfgCmdHandler::handle_input(ACE_HANDLE fd)
{
  int ret = 0;
  ACE_Message_Block *mb = nullptr;
  ACE_NEW_RETURN(mb, ACE_Message_Block(CommonIF::SIZE_64MB), ret);

  if((ret = m_lSockDgram.recv(mb->wr_ptr(), CommonIF::SIZE_64MB, m_peerAddr)) < 0)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:l %m")));
    return(ret);
  }

  /*Is this from CLI? pass it on to Active-Object to handle it.*/
  mb->wr_ptr(ret);
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l got the request %s\n"), mb->rd_ptr()));

  /*post to thread now to process it.*/
  putq(mb);
  return(ret);
}

ACE_HANDLE CfgCmdHandler::get_handle() const
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l CfgCmdHandler::get_handle %d\n"), m_handle));
  return(const_cast<CfgCmdHandler *>(this)->handle());
}

int CfgCmdHandler::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l CfgCmdHandler::handle_close called\n")));
  return(0);
}

int CfgCmdHandler::open(void *args)
{
  activate(THR_NEW_LWP);
  return(0);
}

int CfgCmdHandler::processCommand(ACE_TCHAR *in, int inLen)
{
  int ret = -1;
  ACE_Message_Block *mb = nullptr;

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l process (%s) command length %u\n"), in, inLen));

  ACE_NEW_RETURN(mb, ACE_Message_Block(CommonIF::SIZE_64MB), -1);

  ACE_OS::memcpy(mb->wr_ptr(), "Here I am", 9);
  mb->wr_ptr(9);

  if((ret = m_lSockDgram.send(mb->rd_ptr(), mb->length(), m_peerAddr)) < 0)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l %m send failed\n")));
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l length %u\n"), ret));
  /*reclaim the memory now.*/
  mb->release();
  /*Prepare response now and put it into rspQ.*/
  //m_rspQ.enqueue_tail(mb);
  //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l Registering Write Mask\n")));

  //ACE_Reactor::instance()->register_handler(this,
  //                                          ACE_Event_Handler::WRITE_MASK |
  //                                          ACE_Event_Handler::READ_MASK);
  return(0);
}

int CfgCmdHandler::svc(void)
{
  ACE_Message_Block *mb = nullptr;

  while(1)
  {
    if(-1 != getq(mb))
    {
      /*Process the Command.*/
      if(mb->msg_type() == ACE_Message_Block::MB_HANGUP)
      {
        mb->release();
        break;
      }

      ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l dequeue equest\n")));
      /*Process the Command Now.*/
      processCommand(mb->rd_ptr(), mb->length());
      /*reclaim the heap memory now. allocated by the sender*/
      mb->release();
    }
  }
  return(-1);
}
void CfgCmdHandler::handle(ACE_HANDLE hd)
{
  m_handle = hd;
}

ACE_HANDLE CfgCmdHandler::handle(void)
{
  return(m_handle);
}
#endif /*__CFG_CMD_HANDLER_CC__*/
