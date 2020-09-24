#ifndef __DHCP_SERVER_STATE_LEASE_EXPIRE__H__
#define __DHCP_SERVER_STATE_LEASE_EXPIRE__H__

#include "DhcpServerState.h"

/*
 * @brief class with final keyword is to make leaf class meaning this can't be used to inherit.
 *        This leaf class will override the virtual function of base class. to make it explicit
 *        an override is suffixed to member's name for readability purpose only.
 * */
class DhcpServerStateLeaseExpire final : public DhcpServerState
{
private:
  static DhcpServerStateLeaseExpire *m_instance;
  DhcpServerStateLeaseExpire();

  virtual void do_onEntry(DHCP::Server &parent) override;
  virtual void do_onExit(DHCP::Server &parent) override;
  virtual ACE_UINT32 do_offer(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) override;
  virtual ACE_UINT32 do_discover(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) override;
  virtual ACE_UINT32 do_request(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen) override;
  virtual ACE_UINT32 do_requestAck(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen) override;
  /*Release  */
  virtual ACE_UINT32 do_release(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) override;
  /*NACK  */
  virtual ACE_UINT32 do_nack(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) override;
  /*decline  */
  virtual ACE_UINT32 do_decline(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) override;
  virtual ACE_UINT32 do_leaseTO(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 inLen) override;
  /*Guard Timer is stared if next request is expected to complete the Flow.*/
  virtual ACE_UINT32 do_guardTimerExpiry(DHCP::Server &parent, const void *act) override;
  /*lease Timer is started when IP address assignment is done successfully.*/
  virtual ACE_UINT32 do_leaseTimerExpiry(DHCP::Server &parent, const void *act) override;

public:
  virtual ~DhcpServerStateLeaseExpire();
  static DhcpServerStateLeaseExpire *instance();
  static DhcpServerStateLeaseExpire *get_instance(void);

};


#endif /*__DHCP_SERVER_STATE_LEASE_EXPIRE__H__*/
