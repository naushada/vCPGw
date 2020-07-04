#ifndef __TCLIENT_CC__
#define __TCLIENT_CC__

#include "tclient.h"

using namespace std;

/********************************************************************
 *  Tclient - TCP Client Section Starts.
 * ******************************************************************/
Tclient::Tclient(ACE_UINT16 port_)
{
    m_remote_addr.set_port_number(port_);
}

Tclient::~Tclient()
{
}

int Tclient::handle_input(ACE_HANDLE fd)
{
    int len = -1;
    ACE_Message_Block *mb;
    int maxLen = 1024;

    ACE_NEW_NORETURN(mb, ACE_Message_Block(1024));

    len = m_stream.recv_n(mb->wr_ptr(), maxLen, 0);

    if(len < 0)
    {
        ACE_ERROR((LM_ERROR, "%p\n", "sent-failed"));
    }

    return(0);
}

int Tclient::handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask)
{
    return(0);
}

ACE_HANDLE Tclient::get_handle(void) const
{
    return(const_cast<Tclient *>(this)->handle());
}

void Tclient::handle(ACE_HANDLE fd)
{
    m_handle = fd;
}

ACE_HANDLE Tclient::handle(void)
{
    return(m_handle);
}

int Tclient::transmit(const char *command_, int len_)
{
    int len = 0;

    len = m_stream.send_n(command_, len_, 0);

    if(len < 0)
    {
        ACE_ERROR((LM_ERROR, "%p\n", "sent-failed"));
    }

    return(len);
}

void Tclient::attemp_connect()
{
    ACE_Time_Value timeout (2);

    do
    {
        if (m_conn.connect(m_stream, m_remote_addr, &timeout) == -1)
        {
            if (ACE_OS::last_error() == ETIME)
            {
                ACE_DEBUG ((LM_DEBUG, "(%P|%t) Timeout while "
                                      "connecting to log server\n"));
            }
        }
        else
        {
            handle(m_stream.get_handle());
            break;
        }

    }while(1);

}

/********************************************************************
 * TclientTask - TCP Client Task - Active Object Starts.
 * ******************************************************************/
TclientTask::TclientTask(ACE_Thread_Manager *thrMgr,
                         ACE_UINT16 port,
                         Cli *cli) : ACE_Task<ACE_MT_SYNCH>(thrMgr)
{
    m_cli = cli;
    ACE_NEW_NORETURN(m_tclient, Tclient(port));
    tclient(m_tclient);
}

TclientTask::~TclientTask()
{
    delete m_tclient;
    m_tclient = nullptr;
}

int TclientTask::open(void *args)
{
    /*Make object as an active object.*/
    activate(THR_NEW_LWP);
    return(0);
}


int TclientTask::svc(void)
{
    ACE_Message_Block *mb = nullptr;

    tclient().attemp_connect();

    /*Register discrete Handler to ACE Reactor now.*/
    ACE_Reactor::instance()->register_handler(m_tclient,
                                              ACE_Event_Handler::READ_MASK);

    for(;getq(mb);)
    {
        if(mb->msg_type() == ACE_Message_Block::MB_HANGUP)
        {
             mb->release();
             break;
        }

        tclient().transmit(mb->rd_ptr(), mb->length());

        /*! Time Out Value of 1sec.*/
        ACE_Time_Value to(2,0);
        if(-1 == ACE_Reactor::instance()->handle_events(to))
        {
            ACE_ERROR((LM_ERROR, "handle_events failed\n"));
            break;
        }
    }
    return(0);
}

Tclient &TclientTask::tclient(void)
{
    return(*m_tclient);
}

void TclientTask::tclient(Tclient *t)
{
    m_tclient = t;
}
















#endif /*__TCLIENT_CC__*/
