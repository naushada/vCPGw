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
    if(-1 == m_myAddr.set("/opt/mna/.cliif"))
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l %P|%t local Unix domain socket creation failed\n")));
      break;
    }

    ACE_OS::unlink(m_myAddr.get_path_name());
    if(-1 == m_unixDgram.open(m_myAddr))
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l %P|%t opening of unixSocket Failed\n")));
      break;
    }

    /*learn the handle now.*/
    handle(m_unixDgram.get_handle());

  }while(0);
}

CfgCmdHandler::~CfgCmdHandler()
{
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

    m_rspQ.dequeue_head(mb);
    /*starts sending the bytes now.*/

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
  int ret = -1;
  ACE_Message_Block *mb = nullptr;
  ACE_NEW_RETURN(mb, ACE_Message_Block(CommonIF::SIZE_64MB), ret);


  /*Is this from CLI? pass it on to Active-Object to handle it.*/
  putq(mb);
  ret = 0;
  return(ret);
}

ACE_HANDLE CfgCmdHandler::get_handle() const
{
  ACE_TRACE("CfgCmdHandler::get_handle");
  return(const_cast<CfgCmdHandler *>(this)->handle());
}

int CfgCmdHandler::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
{
}

int CfgCmdHandler::open(void *args)
{
  activate(THR_NEW_LWP);
  return(0);
}

int CfgCmdHandler::processCommand(ACE_TCHAR *in, int inLen)
{
  ACE_Message_Block *mb = nullptr;

  /*Prepare response now and put it into rspQ.*/
  m_rspQ.enqueue_tail(mb);
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

      /*Process the Command Now.*/
      processCommand(mb->rd_ptr(), mb->length());
      /*Set the handle in out-put mode so that it can be sent to peer.*/
      ACE_Reactor::instance()->register_handler(this,
                                                ACE_Event_Handler::WRITE_MASK);
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
