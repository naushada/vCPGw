#ifndef __ARP_H__
#define __ARP_H__

#include "ace/SString.h"
#include "ace/Basic_Types.h"

class CPGateway;

namespace ARP
{
  class CPGwArp
  {
  private:
    ACE_CString m_maccAddress;
    CPGateway *m_parent;
    ACE_CString m_peerMac;
    ACE_UINT32 m_peerIp;

  public:
    CPGwArp(CPGateway *parent, ACE_CString mac);
    ~CPGwArp();

    ACE_UINT32 processRequest(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen);
    ACE_Message_Block *buildResponse(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen);

    ACE_CString &getMac(void);

    ACE_UINT32 peerIp(void);
    void peerIp(ACE_UINT32 ip);

    ACE_CString &peerMac(void);
    void peerMac(ACE_CString mac);

  };
}


#endif /*__ARP_H__*/
