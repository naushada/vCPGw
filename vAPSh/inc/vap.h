#ifndef __VAP_H__
#define __VAP_H__

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
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/tcp.h>
#include <linux/ip.h>
#include <linux/if.h>
#include <linux/udp.h>

#include <unordered_map>
#include <iostream>
#include <string>


#include "ipc.h"
typedef std::unordered_map<std::string, std::string> ctrlParamUMap_t;
typedef ctrlParamUMap_t::iterator ctrlParamIter_t;

class CtrlIF : public ACE_Event_Handler
{
  private:
    ACE_HANDLE m_handle;
    ACE_UNIX_Addr m_unixAddr;
    ACE_LSOCK_Dgram m_unixDgram;
    ctrlParamUMap_t m_ctrlIFParam;
    bool m_isConfigDone;
    static CtrlIF *m_instance;

  public:
    CtrlIF();
    virtual ~CtrlIF();
    static CtrlIF *instance();
    void handle(ACE_HANDLE fd);
    ACE_HANDLE handle(void);

    virtual int handle_input(ACE_HANDLE handle = ACE_INVALID_HANDLE);
    virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask);
    virtual ACE_HANDLE get_handle(void) const;
    void processCtrlIFReq(char *req, size_t reqLen);
    void parseSetParam(char *req);
};


class Vap : public ACE_Task<ACE_MT_SYNCH>
{
  private:
    ACE_CString m_intfName;
    ACE_CString m_ipAddress;
    ACE_CString m_subnetMask;
    ACE_CString m_macAddress;
    ACE_HANDLE m_handle;
    ACE_INT32 m_index;
    ACE_UINT32 m_magic;

    CtrlIF *m_ctrlIF;

  public:
    Vap();
    Vap(ACE_Thread_Manager *thrMgr, ACE_CString &intfName, ACE_CString &ipAddr, ACE_CString &subnetMask,
        ACE_UINT8 container,
        ACE_UINT8 component,
        ACE_UINT8 facility,
        ACE_UINT8 instance);
    virtual ~Vap();

    ACE_INT32 openInterface(void);

    ACE_CString &intfName(void);
    void intfName(ACE_CString &intf);

    void macAddress(ACE_CString &macAddr);
    ACE_CString &macAddress(void);

    ACE_HANDLE handle(void);
    void handle(ACE_HANDLE handle);

    ACE_INT32 index(void);
    void index(ACE_INT32 idx);

    ACE_CString &ipAddress(void);
    void ipAddress(ACE_CString &ipAddr);

    ACE_CString &subnetMask(void);
    void subnetMask(ACE_CString &subnetMask);

    ACE_INT32 setIpAddr(ACE_CString &intfName, ACE_CString &ipAddr, ACE_CString &netMask);
    ACE_INT32 init(void);
    ACE_INT32 start(void);
    ACE_INT32 stop(void);
    virtual int handle_input(ACE_HANDLE handle = ACE_INVALID_HANDLE);
    virtual ACE_HANDLE get_handle(void) const;
    virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask);
};


#endif /* __VAP_H__ */
