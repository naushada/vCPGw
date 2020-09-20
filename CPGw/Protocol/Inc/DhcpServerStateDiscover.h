#ifndef __DHCP_SERVER_STATE_DISCOVER_H__
#define __DHCP_SERVER_STATE_DISCOVER_H__

#include "DhcpServerState.h"

/*
 * @brief class with final keyword is to make leaf class meaning this can't be used to inherit.
 *        This leaf class will override the virtual function of base class. to make it explicit
 *        an override is suffixed to member's name for readability purpose only.
 * */
class DhcpServerStateDiscover final : public DhcpServerState
{
private:
  static DhcpServerStateDiscover *m_instance;
  DhcpServerStateDiscover();

public:
  virtual ~DhcpServerStateDiscover();
  static DhcpServerStateDiscover *instance();
  static DhcpServerStateDiscover *get_instance(void);

private:
  virtual void onEntryImpl(DHCP::Server &parent) override;
  virtual void onExitImpl(DHCP::Server &parent) override;
  virtual ACE_UINT32 offerImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) override;
  virtual ACE_UINT32 discoverImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) override;
  virtual ACE_UINT32 requestImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen) override;
  virtual ACE_UINT32 requestAckImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen) override;
  /*Release  */
  virtual ACE_UINT32 releaseImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) override;
  /*NACK  */
  virtual ACE_UINT32 nackImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) override;
  /*decline  */
  virtual ACE_UINT32 declineImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) override;
  virtual ACE_UINT32 leaseTOImpl(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen) override;
  /*Guard Timer is stared if next request is expected to complete the Flow.*/
  virtual ACE_UINT32 guardTimerExpiryImpl(DHCP::Server &parent, const void *act) override;
  /*lease Timer is started when IP address assignment is done successfully.*/
  virtual ACE_UINT32 leaseTimerExpiryImpl(DHCP::Server &parent, const void *act) override;
};


#endif /*__DHCP_SERVER_STATE_DISCOVER_H__*/
