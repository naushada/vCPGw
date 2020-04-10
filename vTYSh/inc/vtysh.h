#ifndef __VTYSH_H__
#define __VTYSH_H__

/*
This file interfaces with readline library and provides the implementation
for command/response to/from hostapd.
Copyright (C) <2019>  <Mohammed Naushad Ahmed>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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

class VtyshCtrlIF : public ACE_Task<ACE_MT_SYNCH>
{

  public:
    void handle(ACE_HANDLE handle);
    ACE_HANDLE handle(void);
    int main(int argc, char *argv[]);
    int transmit(char *command);

    VtyshCtrlIF(ACE_Thread_Manager *thrMgr);
    virtual ~VtyshCtrlIF();

    virtual int handle_input(ACE_HANDLE handle = ACE_INVALID_HANDLE);
    virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask);
    virtual ACE_HANDLE get_handle(void) const;
    virtual int handle_timeout(const ACE_Time_Value &tv,const void *arg = 0);

  private:
    ACE_HANDLE       m_handle;
    ACE_UNIX_Addr    m_unixAddr;
    ACE_LSOCK_Dgram  m_unixDgram;
    long             m_rspTimerId;
};


class VtyshTask : public VtyshCtrlIF
{
  public:
    VtyshTask(ACE_Thread_Manager *thrMgr);
    virtual ~VtyshTask();

    virtual int open(void *args=0);
    virtual int svc(void);

  private:
};

#endif
