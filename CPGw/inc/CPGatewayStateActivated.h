#ifndef __CPGATEWAY_STATE_ACTIVATED_H__
#define __CPGATEWAY_STATE_ACTIVATED_H__

#include "CPGatewayState.h"

class CPGatewayStateActivated : public CPGatewayState
{
private:
  static CPGatewayStateActivated *m_instance;
public:
  static CPGatewayStateActivated *instance();

  CPGatewayStateActivated();
  virtual ~CPGatewayStateActivated();

  /*Process CPGateway lock Request*/
  virtual ACE_UINT32 lock(CPGateway &parent);
  /*Process CPGateway unlock Request*/
  virtual ACE_UINT32 unlock(CPGateway &parent);

  /*invoked upon state Entry*/
  virtual void onEntry(CPGateway &parent);
  /*invoked upon state Exit*/
  virtual void onExit(CPGateway &parent);
};


#endif /*__CPGATEWAY_STATE_ACTIVATED_H__*/
