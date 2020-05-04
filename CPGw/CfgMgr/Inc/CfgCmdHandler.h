#ifndef __CFG_CMD_HANDLER_H__
#define __CFG_CMD_HANDLER_H__

#include "ace/LSOCK_Stream.h"
#include "ace/LSOCK_Acceptor.h"
#include "ace/Event_Handler.h"
#include "ace/Task.h"
#include <ace/Event_Handler.h>
#include <ace/INET_Addr.h>
#include <ace/UNIX_Addr.h>
#include <ace/SOCK_Dgram.h>
#include <ace/LSOCK_CODgram.h>
#include <ace/LSOCK_Dgram.h>
#include <ace/UNIX_Addr.h>

class CfgCmdHandler : public ACE_Task<ACE_MT_SYNCH>
{
public:
  CfgCmdHandler(ACE_Thread_Manager *thrMgr);
  virtual ~CfgCmdHandler();
  int processCommand(ACE_TCHAR *in, int inLen);

  void handle(ACE_HANDLE hd);
  ACE_HANDLE handle(void);

  /*These are base class virtual function to be overriden.*/
  ACE_HANDLE get_handle() const;
  int handle_input(ACE_HANDLE handle);
  int handle_close(ACE_HANDLE handle,
                   ACE_Reactor_Mask mask);
  int handle_output(ACE_HANDLE fd);

  int open(void *args=0);
  int svc(void);

private:
  ACE_UNIX_Addr       m_myAddr;
  ACE_UNIX_Addr       m_peerAddr;
  ACE_LSOCK_Stream    m_unixStream;
  ACE_LSOCK_Acceptor  m_unixAcceptor;

  ACE_LSOCK_Dgram  m_lSockDgram;
  ACE_HANDLE       m_handle;
  ACE_Unbounded_Queue<ACE_Message_Block *> m_rspQ;

};



#endif /*__CFG_CMD_HANDLER_H__*/
