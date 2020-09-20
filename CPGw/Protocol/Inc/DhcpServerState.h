#ifndef __DHCP_SERVER_STATE_H__
#define __DHCP_SERVER_STATE_H__

#include "CommonIF.h"
#include "ace/SString.h"
#include "ace/Message_Block.h"

/*Forward declaration of a class belong to a namespace. */
namespace DHCP { class Server;}

/*
 * @brief This class has two parts in it, First part of it is interface and second part of it
 *        is implementation which is done be derived class. The interface will invoke/call the
 *        implementation.
 * */
class DhcpServerState
{
protected:
  ACE_CString m_description;

private:

  /*
   * @brief These are the method to be implemented by derived class
   *        which will be invoked by interface method. Interface method
   *        calls implementation of respective derived class. These implementation
   *        method is made pure virtual to make sure then base class is not instantiated.
   * */
  /*DHCP Offer */
  virtual ACE_UINT32 offerImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) = 0;
  /*DHCP Discover */
  virtual ACE_UINT32 discoverImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) = 0;
  /*DHCP Request */
  virtual ACE_UINT32 requestImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) = 0;
  /*DHCP Request Ack */
  virtual ACE_UINT32 requestAckImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) = 0;
  /*Lease Time Out */
  virtual ACE_UINT32 leaseTOImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) = 0;
  /*Release  */
  virtual ACE_UINT32 releaseImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) = 0;
  /*NACK  */
  virtual ACE_UINT32 nackImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) = 0;
  /*decline  */
  virtual ACE_UINT32 declineImpl(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen) = 0;
  virtual void onEntryImpl(DHCP::Server &parent) = 0;
  virtual void onExitImpl(DHCP::Server &parent) = 0;
  /*Guard Timer is stared if next request is expected to complete the Flow.*/
  virtual ACE_UINT32 guardTimerExpiryImpl(DHCP::Server &parent, const void *act) = 0;
  /*lease Timer is started when IP address assignment is done successfully.*/
  virtual ACE_UINT32 leaseTimerExpiryImpl(DHCP::Server &parent, const void *act) = 0;

public:
  DhcpServerState();
  virtual ~DhcpServerState();

  /*
   * @brief These are the interface method which invokes respective implementation
   *
   * */
  void onEntry(DHCP::Server &parent);
  void onExit(DHCP::Server &parent);
  /*Guard Timer is stared if next request is expected to complete the Flow.*/
  ACE_UINT32 guardTimerExpiry(DHCP::Server &parent, const void *act);
  /*lease Timer is started when IP address assignment is done successfully.*/
  ACE_UINT32 leaseTimerExpiry(DHCP::Server &parent, const void *act);
  /*DHCP Offer */
  ACE_UINT32 offer(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen);
  /*DHCP Discover */
  ACE_UINT32 discover(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen);
  /*DHCP Request */
  ACE_UINT32 request(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen);
  /*DHCP Request Ack */
  ACE_UINT32 requestAck(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen);
  /*Lease Time Out */
  ACE_UINT32 leaseTO(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen);
  /*Release  */
  ACE_UINT32 release(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen);
  /*NACK  */
  ACE_UINT32 nack(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen);
  /*decline  */
  ACE_UINT32 decline(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen);

  /*RX  */
  ACE_UINT32 rx(DHCP::Server &parent, ACE_Byte *inPtr, ACE_UINT32 inLen);
  /*Populate DHCP Header.*/
  ACE_UINT32 populateDhcpHeader(DHCP::Server &parent, TransportIF::DHCP *dhcpHeader);
  /*Populate DHCP Options.*/
  ACE_UINT32 populateDhcpOption(DHCP::Server &parent, ACE_Byte *dhcpOption, ACE_UINT32 len);
  /*build Response.*/
  ACE_Message_Block &buildResponse(DHCP::Server &parent, ACE_Byte *in, ACE_UINT32 len);
  ACE_UINT16 chksumUDP(TransportIF::IP *in);
  ACE_UINT16 chksumIP(void *in, ACE_UINT32 inLen);
};

#endif /*__DHCP_SERVER_STATE_H__*/
