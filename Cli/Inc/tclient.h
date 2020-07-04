#ifndef __TCLIENT_H__
#define __TCLIENT_H__

#include <ace/Message_Block.h>
#include <ace/Reactor.h>
#include <ace/Basic_Types.h>
#include <ace/Event_Handler.h>
#include <ace/Task.h>
#include <ace/INET_Addr.h>
#include <ace/UNIX_Addr.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Dgram.h>
#include <ace/LSOCK_CODgram.h>
#include <ace/LSOCK_Dgram.h>
#include <ace/UNIX_Addr.h>
#include <ace/Log_Msg.h>

#define PORT 28751
class Cli;

class Tclient : public ACE_Event_Handler
{
  public:
    int handle_input(ACE_HANDLE handle = ACE_INVALID_HANDLE);
    int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask);
    ACE_HANDLE get_handle(void) const;

    /*Default constructor.*/
    Tclient(ACE_UINT16 port_);
    virtual ~Tclient();

    void handle(ACE_HANDLE handle);
    ACE_HANDLE handle(void);
    int transmit(const char *command_, int len_);
    void attemp_connect(void);

  private:
    ACE_HANDLE  m_handle;
    /*Local Address*/
    ACE_INET_Addr  m_remote_addr;
    ACE_SOCK_Connector m_conn;
    ACE_SOCK_Stream m_stream;

};

class TclientTask : public ACE_Task<ACE_MT_SYNCH>
{
  public:
    int open(void *args=0);
    int svc(void);

    TclientTask(ACE_Thread_Manager *th, ACE_UINT16 port, Cli *cli);
    virtual ~TclientTask();

    Cli &cli(void);
    void cli(Cli *t);

    Tclient &tclient(void);
    void tclient(Tclient *t);

  private:
    Cli *m_cli;
    Tclient *m_tclient;
};

#endif /*__TCLIENT_H__*/
