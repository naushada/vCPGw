#ifndef __CPGATEWAY_STATE_H__
#define __CPGATEWAY_STATE_H__

class CPGateway;

class CPGatewayState
{
protected:
  ACE_CString m_description;

public:
  CPGatewayState();
  ~CPGatewayState();

  void description(ACE_CString desc);
  ACE_CString description(void);

  /*Process Request */
  ACE_UINT32 processRequest(CPGateway &parent, ACE_Byte *in, ACE_UINT32 inLen);
  /*Process CPGateway lock Request*/
  virtual ACE_UINT32 lock(CPGateway &parent);
  /*Process CPGateway unlock Request*/
  virtual ACE_UINT32 unlock(CPGateway &parent);

  /*invoked upon state Entry*/
  virtual void onEntry(CPGateway &parent);
  /*invoked upon state Exit*/
  virtual void onExit(CPGateway &parent);
};




#endif /*__CPGATEWAY_STATE_H__*/
