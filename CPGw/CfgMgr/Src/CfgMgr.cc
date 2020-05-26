#ifndef __CFG_MGR_CC__
#define __CFG_MGR_CC__

#include "CfgMgr.h"
#include "CfgMgrMain.h"
#include "CommonIF.h"
#include "UniIPC.h"

#include "ace/Message_Block.h"

CfgMgr::~CfgMgr()
{
  stop();
}

CfgMgr::CfgMgr(ACE_CString &schema)
{
  m_cpGwCfg = nullptr;
  aaa().unbind_all();
  http().unbind_all();
  ap().unbind_all();
  dhcp().unbind_all();
  agent().unbind_all();
  cpgw().unbind_all();
  m_schema = schema;
}

CfgMgr::CfgMgr(ACE_Thread_Manager *thrMgr, ACE_CString ip, ACE_UINT8 entId, ACE_UINT8 instId, ACE_CString nodeTag) :
  UniIPC(thrMgr, ip, entId, instId, nodeTag)
{
  ACE_Reactor::instance()->register_handler(this,
                                            ACE_Event_Handler::READ_MASK);
}

AAAInstMap_t &CfgMgr::aaa(void)
{
  return(m_aaa);
}

HTTPInstMap_t &CfgMgr::http(void)
{
  return(m_http);
}

APInstMap_t &CfgMgr::ap(void)
{
  return(m_ap);
}

DHCPInstMap_t &CfgMgr::dhcp(void)
{
  return(m_dhcp);
}

DHCPAgentInstMap_t &CfgMgr::agent(void)
{
  return(m_agent);
}

CPGWInstMap_t &CfgMgr::cpgw(void)
{
  return(m_cpgw);
}

ACE_INT32 CfgMgr::processDHCPAgentCfg(void)
{
  ACE_INT32 ret = -1;
  do
  {
    /*Global Object.*/
    JSON glob(m_cpGwCfg->value());

    /*For root object.*/
    JSON inst(m_cpGwCfg->value());
    /*Value of root node of JSON.*/
    JSON::JSONValue *root = inst["cp-gateway"];
    if(!root)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid schema with root\n")));
      break;
    }
    JSON objR(root);

    /*First Child of root node - root.*/
    JSON::JSONValue *rootInstC1 = objR["instances"];
    if(!rootInstC1)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid instance child\n")));
      break;
    }

    JSON objInstRC1(rootInstC1);

    /*Start Accessing individual element of instance now and put them in ACE HASH MAP.*/
    JSON::JSONValue *rootInstC11 = objInstRC1["dhcp-agent"];
    if(!rootInstC11)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid rootInstC11 child under root\n")));
      break;
    }

    /*populating HASH MAP now with instance configuration.*/
    int idx = 0;
    JSON::JSONValue *val = nullptr;
    JSON objAgent(rootInstC11);

    /*All instances of HTTP.*/
    for(val = objAgent[idx]; val; val = objAgent[++idx])
    {
      char *key = ACE_OS::strdup(val->m_svalue);
      ACE_CString keyStr(key, ACE_OS::strlen((const char *)key));

      _CpGwDHCPAgentInstance_t *pInst = nullptr;
      ACE_NEW_RETURN(pInst, _CpGwDHCPAgentInstance_t(), 0);

      /*Insert into MAP now.*/
      agent().bind(keyStr, pInst);

      /*http/uam instance name .*/
      JSON::JSONValue *agentVal = glob[key];
      JSON objAgent(agentVal);

      /*Individual element of dhcp instance start.*/
      JSON::JSONValue *virtNw = objAgent["virtual-nw"];
      if(!virtNw)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr key %s\n"), keyStr.c_str()));
        /*reclaim the heap memory.*/
        agent().unbind(keyStr);
        delete pInst;
        ACE_OS::free(key);
        key = nullptr;
        break;
      }

      ACE_CString VirtNwName(virtNw->m_svalue);

      /*extracting virtual-networks to scan the network name mentioned in dhcp instance.*/
      virtNw = glob["virtual-networks"];
      if(!virtNw)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr\n")));
        /*reclaim the heap memory.*/
        agent().unbind(keyStr);
        delete pInst;
        ACE_OS::free(key);
        key = nullptr;
        break;
      }

      JSON objRVirtNw(virtNw);

      int idxx = 0;
      JSON::JSONValue *vval = nullptr;
      for(vval = objRVirtNw[idxx]; vval; vval = objRVirtNw[++idxx])
      {
        JSON objVirtVal(vval);
        JSON::JSONValue *oval = objVirtVal["name"];
        if(!oval)
        {
          ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr\n")));
          /*reclaim the heap memory.*/
          agent().unbind(keyStr);
          delete pInst;
          break;
        }

        ACE_CString vNwVal(oval->m_svalue, ACE_OS::strlen((const char *)oval->m_svalue));

        if(VirtNwName == vNwVal)
        {
          /*virtual network name matched.*/
          ACE_OS::strncpy((ACE_TCHAR *)pInst->m_nw.m_name, vNwVal.c_str(),
                          (sizeof(pInst->m_nw.m_name) - 1));

          JSON objVirtInterface(objVirtVal["interface"]);
          oval = objVirtInterface["type"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[type] is nullptr\n")));
            /*reclaim the heap memory.*/
            agent().unbind(keyStr);
            delete pInst;
            break;
          }

          ACE_OS::strncpy((ACE_TCHAR *)pInst->m_nw.m_type, oval->m_svalue,
                          (sizeof(pInst->m_nw.m_type) - 1));

          oval = objVirtInterface["address"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[address] is nullptr\n")));
            /*reclaim the heap memory.*/
            agent().unbind(keyStr);
            delete pInst;
            break;
          }

          sscanf(oval->m_svalue, "%d.%d.%d.%d",
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[0],
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[1],
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[2],
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[3]);

          oval = objVirtInterface["net-mask"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[net-mask] is nullptr\n")));
            /*reclaim the heap memory.*/
            agent().unbind(keyStr);
            delete pInst;
            break;
          }

          sscanf((const ACE_TCHAR *)oval->m_svalue, "%d.%d.%d.%d",
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[0],
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[1],
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[2],
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[3]);

          oval = objVirtInterface["port"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[port] is nullptr\n")));
            /*reclaim the heap memory.*/
            agent().unbind(keyStr);
            delete pInst;
            break;
          }

          ACE_OS::strncpy((ACE_TCHAR *)pInst->m_nw.m_port, oval->m_svalue,
                          (sizeof(pInst->m_nw.m_port) - 1));

          /*break the for loop of virtual NW now.*/
          break;
        }
      }

      ACE_OS::free(key);
      key = nullptr;

      /*Now populate remaining dhcp element.*/
      vval = objAgent["subnet-mask"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[subnet-mask] is nullptr\n")));
        /*reclaim the heap memory.*/
        agent().unbind(keyStr);
        delete pInst;
        break;
      }

      sscanf((const ACE_TCHAR *)vval->m_svalue, "%d.%d.%d.%d",
             (ACE_INT32 *)&pInst->m_mask.m_ips[0],
             (ACE_INT32 *)&pInst->m_mask.m_ips[1],
             (ACE_INT32 *)&pInst->m_mask.m_ips[2],
             (ACE_INT32 *)&pInst->m_mask.m_ips[3]);

      vval = objAgent["ip"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[ip] is nullptr\n")));
        /*reclaim the heap memory.*/
        agent().unbind(keyStr);
        delete pInst;
        break;
      }

      sscanf((const ACE_TCHAR *)vval->m_svalue, "%d.%d.%d.%d",
             (ACE_INT32 *)&pInst->m_ip.m_ips[0],
             (ACE_INT32 *)&pInst->m_ip.m_ips[1],
             (ACE_INT32 *)&pInst->m_ip.m_ips[2],
             (ACE_INT32 *)&pInst->m_ip.m_ips[3]);

      vval = objAgent["host-name"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[host-name] is nullptr\n")));
        /*reclaim the heap memory.*/
        agent().unbind(keyStr);
        delete pInst;
        break;
      }

      ACE_OS::strncpy((ACE_TCHAR *)pInst->m_host_name, vval->m_svalue,
                      (sizeof(pInst->m_host_name) - 1));

      vval = objAgent["server-ip"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[server-ip] is nullptr\n")));
        /*reclaim the heap memory.*/
        agent().unbind(keyStr);
        delete pInst;
        break;
      }

      sscanf((const ACE_TCHAR *)vval->m_svalue, "%d.%d.%d.%d",
             (ACE_INT32 *)&pInst->m_server_ip.m_ips[0],
             (ACE_INT32 *)&pInst->m_server_ip.m_ips[1],
             (ACE_INT32 *)&pInst->m_server_ip.m_ips[2],
             (ACE_INT32 *)&pInst->m_server_ip.m_ips[3]);
    }

    ret = 0;
  }while(0);

  return(ret);
}

ACE_INT32 CfgMgr::processCPGWCfg(void)
{
  ACE_INT32 ret = -1;
  do
  {
    /*Global Object.*/
    JSON glob(m_cpGwCfg->value());

    /*For root object.*/
    JSON inst(m_cpGwCfg->value());
    /*Value of root node of JSON.*/
    JSON::JSONValue *root = inst["cp-gateway"];
    if(!root)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid schema with root\n")));
      break;
    }
    JSON objR(root);

    /*First Child of root node - root.*/
    JSON::JSONValue *rootInstC1 = objR["instances"];
    if(!rootInstC1)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid instance child\n")));
      break;
    }

    JSON objInstRC1(rootInstC1);

    /*Start Accessing individual element of instance now and put them in ACE HASH MAP.*/
    JSON::JSONValue *rootInstC11 = objInstRC1["cp-gw"];
    if(!rootInstC11)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid rootInstC11 child under root\n")));
      break;
    }

    /*populating HASH MAP now with instance configuration.*/
    int idx = 0;
    JSON::JSONValue *val = nullptr;
    JSON cpGw(rootInstC11);

    /*All instances of cpGw.*/
    for(val = cpGw[idx]; val; val = cpGw[++idx])
    {
      char *key = ACE_OS::strdup(val->m_svalue);
      ACE_CString keyStr(key, ACE_OS::strlen((const char *)key));

      _CpGwCPGWInstance_t *pInst = nullptr;
      ACE_NEW_RETURN(pInst, _CpGwCPGWInstance_t(), 0);

      /*Insert into MAP now.*/
      cpgw().bind(keyStr, pInst);

      /*CPGW instance name .*/
      JSON::JSONValue *Val = glob[key];
      JSON objCpGw(Val);

      /*Individual element of dhcp instance start.*/
      JSON::JSONValue *virtNw = objCpGw["virtual-nw"];
      if(!virtNw)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr key %s\n"), keyStr.c_str()));
        /*reclaim the heap memory.*/
        cpgw().unbind(keyStr);
        delete pInst;
        ACE_OS::free(key);
        key = nullptr;
        break;
      }

      ACE_CString VirtNwName(virtNw->m_svalue);

      /*extracting virtual-networks to scan the network name mentioned in dhcp instance.*/
      virtNw = glob["virtual-networks"];
      if(!virtNw)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr\n")));
        /*reclaim the heap memory.*/
        cpgw().unbind(keyStr);
        delete pInst;
        ACE_OS::free(key);
        key = nullptr;
        break;
      }

      JSON objRVirtNw(virtNw);

      int idxx = 0;
      JSON::JSONValue *vval = nullptr;
      for(vval = objRVirtNw[idxx]; vval; vval = objRVirtNw[++idxx])
      {
        JSON objVirtVal(vval);
        JSON::JSONValue *oval = objVirtVal["name"];
        if(!oval)
        {
          ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr\n")));
          /*reclaim the heap memory.*/
          cpgw().unbind(keyStr);
          delete pInst;
          break;
        }

        ACE_CString vNwVal(oval->m_svalue, ACE_OS::strlen((const char *)oval->m_svalue));

        if(VirtNwName == vNwVal)
        {
          /*virtual network name matched.*/
          ACE_OS::strncpy((ACE_TCHAR *)pInst->m_nw.m_name, vNwVal.c_str(),
                          (sizeof(pInst->m_nw.m_name) - 1));

          JSON objVirtInterface(objVirtVal["interface"]);
          oval = objVirtInterface["type"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[type] is nullptr\n")));
            /*reclaim the heap memory.*/
            cpgw().unbind(keyStr);
            delete pInst;
            break;
          }

          ACE_OS::strncpy((ACE_TCHAR *)pInst->m_nw.m_type, oval->m_svalue,
                          (sizeof(pInst->m_nw.m_type) - 1));

          oval = objVirtInterface["address"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[address] is nullptr\n")));
            /*reclaim the heap memory.*/
            cpgw().unbind(keyStr);
            delete pInst;
            break;
          }

          sscanf(oval->m_svalue, "%d.%d.%d.%d",
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[0],
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[1],
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[2],
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[3]);

          oval = objVirtInterface["net-mask"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[net-mask] is nullptr\n")));
            /*reclaim the heap memory.*/
            cpgw().unbind(keyStr);
            delete pInst;
            break;
          }

          sscanf((const ACE_TCHAR *)oval->m_svalue, "%d.%d.%d.%d",
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[0],
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[1],
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[2],
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[3]);

          oval = objVirtInterface["port"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[port] is nullptr\n")));
            /*reclaim the heap memory.*/
            cpgw().unbind(keyStr);
            delete pInst;
            break;
          }

          ACE_OS::strncpy((ACE_TCHAR *)pInst->m_nw.m_port, oval->m_svalue,
                          (sizeof(pInst->m_nw.m_port) - 1));

          /*break the for loop of virtual NW now.*/
          break;
        }
      }

      ACE_OS::free(key);
      key = nullptr;

      /*Now populate remaining dhcp element.*/
      vval = objCpGw["subnet-mask"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[subnet-mask] is nullptr\n")));
        /*reclaim the heap memory.*/
        cpgw().unbind(keyStr);
        delete pInst;
        break;
      }

      sscanf((const ACE_TCHAR *)vval->m_svalue, "%d.%d.%d.%d",
             (ACE_INT32 *)&pInst->m_mask.m_ips[0],
             (ACE_INT32 *)&pInst->m_mask.m_ips[1],
             (ACE_INT32 *)&pInst->m_mask.m_ips[2],
             (ACE_INT32 *)&pInst->m_mask.m_ips[3]);

      vval = objCpGw["ip"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[ip] is nullptr\n")));
        /*reclaim the heap memory.*/
        cpgw().unbind(keyStr);
        delete pInst;
        break;
      }

      sscanf((const ACE_TCHAR *)vval->m_svalue, "%d.%d.%d.%d",
             (ACE_INT32 *)&pInst->m_ip.m_ips[0],
             (ACE_INT32 *)&pInst->m_ip.m_ips[1],
             (ACE_INT32 *)&pInst->m_ip.m_ips[2],
             (ACE_INT32 *)&pInst->m_ip.m_ips[3]);

      vval = objCpGw["host-name"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[host-name] is nullptr\n")));
        /*reclaim the heap memory.*/
        cpgw().unbind(keyStr);
        delete pInst;
        break;
      }

      ACE_OS::strncpy((ACE_TCHAR *)pInst->m_host_name, vval->m_svalue,
                      (sizeof(pInst->m_host_name) - 1));
    }

    ret = 0;
  }while(0);

  return(ret);
}

ACE_INT32 CfgMgr::processAAACfg(void)
{
  ACE_INT32 ret = -1;
  do
  {
    /*Global Object.*/
    JSON glob(m_cpGwCfg->value());

    /*For root object.*/
    JSON inst(m_cpGwCfg->value());
    /*Value of root node of JSON.*/
    JSON::JSONValue *root = inst["cp-gateway"];
    if(!root)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid schema with root\n")));
      break;
    }
    JSON objR(root);

    /*First Child of root node - root.*/
    JSON::JSONValue *rootInstC1 = objR["instances"];
    if(!rootInstC1)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid instance child\n")));
      break;
    }

    JSON objInstRC1(rootInstC1);

    /*Start Accessing individual element of instance now and put them in ACE HASH MAP.*/
    JSON::JSONValue *rootInstC11 = objInstRC1["aaa-server"];
    if(!rootInstC11)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid rootInstC11 child under root\n")));
      break;
    }

    /*populating HASH MAP now with instance configuration.*/
    int idx = 0;
    JSON::JSONValue *val = nullptr;
    JSON aaaServer(rootInstC11);

    /*All instances of aaa.*/
    for(val = aaaServer[idx]; val; val = aaaServer[++idx])
    {
      char *key = ACE_OS::strdup(val->m_svalue);
      ACE_CString keyStr(key, ACE_OS::strlen((const char *)key));

      _CpGwAAAInstance_t *pInst = nullptr;
      ACE_NEW_RETURN(pInst, _CpGwAAAInstance_t(), 0);

      /*Insert into MAP now.*/
      aaa().bind(keyStr, pInst);

      /*http/uam instance name .*/
      JSON::JSONValue *Val = glob[key];
      JSON objAAA(Val);

      /*Individual element of dhcp instance start.*/
      JSON::JSONValue *virtNw = objAAA["virtual-nw"];
      if(!virtNw)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr key %s\n"), keyStr.c_str()));
        /*reclaim the heap memory.*/
        aaa().unbind(keyStr);
        delete pInst;
        ACE_OS::free(key);
        key = nullptr;
        break;
      }

      ACE_CString VirtNwName(virtNw->m_svalue);

      /*extracting virtual-networks to scan the network name mentioned in dhcp instance.*/
      virtNw = glob["virtual-networks"];
      if(!virtNw)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr\n")));
        /*reclaim the heap memory.*/
        aaa().unbind(keyStr);
        delete pInst;
        ACE_OS::free(key);
        key = nullptr;
        break;
      }

      JSON objRVirtNw(virtNw);

      int idxx = 0;
      JSON::JSONValue *vval = nullptr;
      for(vval = objRVirtNw[idxx]; vval; vval = objRVirtNw[++idxx])
      {
        JSON objVirtVal(vval);
        JSON::JSONValue *oval = objVirtVal["name"];
        if(!oval)
        {
          ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr\n")));
          /*reclaim the heap memory.*/
          aaa().unbind(keyStr);
          delete pInst;
          break;
        }

        ACE_CString vNwVal(oval->m_svalue, ACE_OS::strlen((const char *)oval->m_svalue));

        if(VirtNwName == vNwVal)
        {
          /*virtual network name matched.*/
          ACE_OS::strncpy((ACE_TCHAR *)pInst->m_nw.m_name, vNwVal.c_str(),
                          (sizeof(pInst->m_nw.m_name) - 1));

          JSON objVirtInterface(objVirtVal["interface"]);
          oval = objVirtInterface["type"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[type] is nullptr\n")));
            /*reclaim the heap memory.*/
            aaa().unbind(keyStr);
            delete pInst;
            break;
          }

          ACE_OS::strncpy((ACE_TCHAR *)pInst->m_nw.m_type, oval->m_svalue,
                          (sizeof(pInst->m_nw.m_type) - 1));

          oval = objVirtInterface["address"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[address] is nullptr\n")));
            /*reclaim the heap memory.*/
            aaa().unbind(keyStr);
            delete pInst;
            break;
          }

          sscanf(oval->m_svalue, "%d.%d.%d.%d",
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[0],
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[1],
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[2],
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[3]);

          oval = objVirtInterface["net-mask"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[net-mask] is nullptr\n")));
            /*reclaim the heap memory.*/
            aaa().unbind(keyStr);
            delete pInst;
            break;
          }

          sscanf((const ACE_TCHAR *)oval->m_svalue, "%d.%d.%d.%d",
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[0],
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[1],
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[2],
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[3]);

          oval = objVirtInterface["port"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[port] is nullptr\n")));
            /*reclaim the heap memory.*/
            aaa().unbind(keyStr);
            delete pInst;
            break;
          }

          ACE_OS::strncpy((ACE_TCHAR *)pInst->m_nw.m_port, oval->m_svalue,
                          (sizeof(pInst->m_nw.m_port) - 1));

          /*break the for loop of virtual NW now.*/
          break;
        }
      }

      ACE_OS::free(key);
      key = nullptr;

      /*Now populate remaining AAA element.*/
      vval = objAAA["ip"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[ip] is nullptr\n")));
        /*reclaim the heap memory.*/
        aaa().unbind(keyStr);
        delete pInst;
        break;
      }

      sscanf((const ACE_TCHAR *)vval->m_svalue, "%d.%d.%d.%d",
             (ACE_INT32 *)&pInst->m_ip.m_ips[0],
             (ACE_INT32 *)&pInst->m_ip.m_ips[1],
             (ACE_INT32 *)&pInst->m_ip.m_ips[2],
             (ACE_INT32 *)&pInst->m_ip.m_ips[3]);

      vval = objAAA["protocol"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[protocol] is nullptr\n")));
        /*reclaim the heap memory.*/
        aaa().unbind(keyStr);
        delete pInst;
        break;
      }

      ACE_OS::strncpy((ACE_TCHAR *)pInst->m_protocol, vval->m_svalue,
                      (sizeof(pInst->m_protocol) - 1));

      vval = objAAA["auth-port"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[auth-port] is nullptr\n")));
        /*reclaim the heap memory.*/
        aaa().unbind(keyStr);
        delete pInst;
        break;
      }

      pInst->m_auth_port = vval->m_ivalue;

      vval = objAAA["acc-port"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[acc-port] is nullptr\n")));
        /*reclaim the heap memory.*/
        aaa().unbind(keyStr);
        delete pInst;
        break;
      }

      pInst->m_acc_port = vval->m_ivalue;

      vval = objAAA["admin-user"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[admin-user] is nullptr\n")));
        /*reclaim the heap memory.*/
        aaa().unbind(keyStr);
        delete pInst;
        break;
      }

      ACE_OS::strncpy((ACE_TCHAR *)pInst->m_admin_user, vval->m_svalue,
                      (sizeof(pInst->m_admin_user) - 1));

      vval = objAAA["admin-pwd"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[admin-pwd] is nullptr\n")));
        /*reclaim the heap memory.*/
        aaa().unbind(keyStr);
        delete pInst;
        break;
      }

      ACE_OS::strncpy((ACE_TCHAR *)pInst->m_admin_pwd, vval->m_svalue,
                      (sizeof(pInst->m_admin_pwd) - 1));

      vval = objAAA["peer-ip"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[peer-ip] is nullptr\n")));
        /*reclaim the heap memory.*/
        aaa().unbind(keyStr);
        delete pInst;
        break;
      }

      sscanf((const ACE_TCHAR *)vval->m_svalue, "%d.%d.%d.%d",
             (ACE_INT32 *)&pInst->m_peer_ip.m_ips[0],
             (ACE_INT32 *)&pInst->m_peer_ip.m_ips[1],
             (ACE_INT32 *)&pInst->m_peer_ip.m_ips[2],
             (ACE_INT32 *)&pInst->m_peer_ip.m_ips[3]);

      vval = objAAA["peer-port"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[peer-port] is nullptr\n")));
        /*reclaim the heap memory.*/
        aaa().unbind(keyStr);
        delete pInst;
        break;
      }

      pInst->m_peer_port = vval->m_ivalue;
    }

    ret = 0;
  }while(0);

  return(ret);
}

ACE_INT32 CfgMgr::processAPCfg(void)
{
  ACE_INT32 ret = -1;

  do
  {
    /*Global Object.*/
    JSON glob(m_cpGwCfg->value());

    /*For root object.*/
    JSON inst(m_cpGwCfg->value());
    /*Value of root node of JSON.*/
    JSON::JSONValue *root = inst["cp-gateway"];
    if(!root)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid schema with root\n")));
      break;
    }
    JSON objR(root);

    /*First Child of root node - root.*/
    JSON::JSONValue *rootInstC1 = objR["instances"];
    if(!rootInstC1)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid instance child\n")));
      break;
    }

    JSON objInstRC1(rootInstC1);

    /*Start Accessing individual element of instance now and put them in ACE HASH MAP.*/
    JSON::JSONValue *rootInstC11 = objInstRC1["access-points"];
    if(!rootInstC11)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid rootInstC11 child under root\n")));
      break;
    }

    /*populating HASH MAP now with instance configuration.*/
    int idx = 0;
    JSON::JSONValue *val = nullptr;
    JSON objAp(rootInstC11);

    /*All instances of HTTP.*/
    for(val = objAp[idx]; val; val = objAp[++idx])
    {
      char *key = ACE_OS::strdup(val->m_svalue);
      ACE_CString keyStr(key, ACE_OS::strlen((const char *)key));

      _CpGwAPInstance_t *pInst = nullptr;
      ACE_NEW_RETURN(pInst, _CpGwAPInstance_t(), 0);

      /*Insert into MAP now.*/
      ap().bind(keyStr, pInst);

      /*http/uam instance name .*/
      JSON::JSONValue *Val = glob[key];
      JSON objAP(Val);

      /*Individual element of dhcp instance start.*/
      JSON::JSONValue *virtNw = objAP["virtual-nw"];
      if(!virtNw)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr key %s\n"), keyStr.c_str()));
        /*reclaim the heap memory.*/
        ap().unbind(keyStr);
        delete pInst;
        ACE_OS::free(key);
        key = nullptr;
        break;
      }

      ACE_CString VirtNwName(virtNw->m_svalue);

      /*extracting virtual-networks to scan the network name mentioned in dhcp instance.*/
      virtNw = glob["virtual-networks"];
      if(!virtNw)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr\n")));
        /*reclaim the heap memory.*/
        ap().unbind(keyStr);
        delete pInst;
        ACE_OS::free(key);
        key = nullptr;
        break;
      }

      JSON objRVirtNw(virtNw);

      int idxx = 0;
      JSON::JSONValue *vval = nullptr;
      for(vval = objRVirtNw[idxx]; vval; vval = objRVirtNw[++idxx])
      {
        JSON objVirtVal(vval);
        JSON::JSONValue *oval = objVirtVal["name"];
        if(!oval)
        {
          ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr\n")));
          /*reclaim the heap memory.*/
          ap().unbind(keyStr);
          delete pInst;
          break;
        }

        ACE_CString vNwVal(oval->m_svalue, ACE_OS::strlen((const char *)oval->m_svalue));

        if(VirtNwName == vNwVal)
        {
          /*virtual network name matched.*/
          ACE_OS::strncpy((ACE_TCHAR *)pInst->m_nw.m_name, vNwVal.c_str(),
                          (sizeof(pInst->m_nw.m_name) - 1));

          JSON objVirtInterface(objVirtVal["interface"]);
          oval = objVirtInterface["type"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[type] is nullptr\n")));
            /*reclaim the heap memory.*/
            ap().unbind(keyStr);
            delete pInst;
            break;
          }

          ACE_OS::strncpy((ACE_TCHAR *)pInst->m_nw.m_type, oval->m_svalue,
                          (sizeof(pInst->m_nw.m_type) - 1));

          oval = objVirtInterface["address"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[address] is nullptr\n")));
            /*reclaim the heap memory.*/
            ap().unbind(keyStr);
            delete pInst;
            break;
          }

          sscanf(oval->m_svalue, "%d.%d.%d.%d",
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[0],
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[1],
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[2],
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[3]);

          oval = objVirtInterface["net-mask"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[net-mask] is nullptr\n")));
            /*reclaim the heap memory.*/
            ap().unbind(keyStr);
            delete pInst;
            break;
          }

          sscanf((const ACE_TCHAR *)oval->m_svalue, "%d.%d.%d.%d",
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[0],
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[1],
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[2],
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[3]);

          oval = objVirtInterface["port"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[port] is nullptr\n")));
            /*reclaim the heap memory.*/
            ap().unbind(keyStr);
            delete pInst;
            break;
          }

          ACE_OS::strncpy((ACE_TCHAR *)pInst->m_nw.m_port, oval->m_svalue,
                          (sizeof(pInst->m_nw.m_port) - 1));

          /*break the for loop of virtual NW now.*/
          break;
        }
      }

      ACE_OS::free(key);
      key = nullptr;

      /*Now populate remaining AP element.*/
      vval = objAP["ap-name"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[ap-name] is nullptr\n")));
        /*reclaim the heap memory.*/
        ap().unbind(keyStr);
        delete pInst;
        break;
      }

      ACE_OS::strncpy((ACE_TCHAR *)pInst->m_ap_name, vval->m_svalue,
                      (sizeof(pInst->m_ap_name) - 1));

      vval = objAP["latitude"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[latitude] is nullptr\n")));
        /*reclaim the heap memory.*/
        ap().unbind(keyStr);
        delete pInst;
        break;
      }

      ACE_OS::strncpy((ACE_TCHAR *)pInst->m_latitude, vval->m_svalue,
                      (sizeof(pInst->m_latitude) - 1));

      vval = objAP["longitude"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[longitude] is nullptr\n")));
        /*reclaim the heap memory.*/
        ap().unbind(keyStr);
        delete pInst;
        break;
      }

      ACE_OS::strncpy((ACE_TCHAR *)pInst->m_longitude, vval->m_svalue,
                      (sizeof(pInst->m_longitude) - 1));

      vval = objAP["elevation"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[elevation] is nullptr\n")));
        /*reclaim the heap memory.*/
        ap().unbind(keyStr);
        delete pInst;
        break;
      }

      ACE_OS::strncpy((ACE_TCHAR *)pInst->m_elevation, vval->m_svalue,
                      (sizeof(pInst->m_elevation) - 1));
    }

    ret = 0;
  }while(0);

  return(ret);
}

ACE_INT32 CfgMgr::processHTTPServerCfg(void)
{
  ACE_INT32 ret = -1;
  do
  {
    /*Global Object.*/
    JSON glob(m_cpGwCfg->value());

    /*For root object.*/
    JSON inst(m_cpGwCfg->value());
    /*Value of root node of JSON.*/
    JSON::JSONValue *root = inst["cp-gateway"];
    if(!root)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid schema with root\n")));
      break;
    }
    JSON objR(root);

    /*First Child of root node - root.*/
    JSON::JSONValue *rootInstC1 = objR["instances"];
    if(!rootInstC1)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid instance child\n")));
      break;
    }

    JSON objInstRC1(rootInstC1);

    /*Start Accessing individual element of instance now and put them in ACE HASH MAP.*/
    JSON::JSONValue *rootInstC11 = objInstRC1["http-server"];
    if(!rootInstC11)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid rootInstC11 child under root\n")));
      break;
    }

    /*populating HASH MAP now with instance configuration.*/
    int idx = 0;
    JSON::JSONValue *val = nullptr;
    JSON uamServer(rootInstC11);

    /*All instances of HTTP.*/
    for(val = uamServer[idx]; val; val = uamServer[++idx])
    {
      char *key = ACE_OS::strdup(val->m_svalue);
      ACE_CString keyStr(key, ACE_OS::strlen((const char *)key));

      _CpGwHTTPInstance_t *pInst = nullptr;
      ACE_NEW_RETURN(pInst, _CpGwHTTPInstance_t(), 0);

      /*Insert into MAP now.*/
      http().bind(keyStr, pInst);

      /*http/uam instance name .*/
      JSON::JSONValue *uamVal = glob[key];
      JSON objHttp(uamVal);

      /*Individual element of dhcp instance start.*/
      JSON::JSONValue *virtNw = objHttp["virtual-nw"];
      if(!virtNw)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr key %s\n"), keyStr.c_str()));
        /*reclaim the heap memory.*/
        http().unbind(keyStr);
        delete pInst;
        ACE_OS::free(key);
        key = nullptr;
        break;
      }

      ACE_CString VirtNwName(virtNw->m_svalue);

      /*extracting virtual-networks to scan the network name mentioned in dhcp instance.*/
      virtNw = glob["virtual-networks"];
      if(!virtNw)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr\n")));
        /*reclaim the heap memory.*/
        http().unbind(keyStr);
        delete pInst;
        ACE_OS::free(key);
        key = nullptr;
        break;
      }

      JSON objRVirtNw(virtNw);

      int idxx = 0;
      JSON::JSONValue *vval = nullptr;
      for(vval = objRVirtNw[idxx]; vval; vval = objRVirtNw[++idxx])
      {
        JSON objVirtVal(vval);
        JSON::JSONValue *oval = objVirtVal["name"];
        if(!oval)
        {
          ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr\n")));
          /*reclaim the heap memory.*/
          http().unbind(keyStr);
          delete pInst;
          break;
        }

        ACE_CString vNwVal(oval->m_svalue, ACE_OS::strlen((const char *)oval->m_svalue));

        if(VirtNwName == vNwVal)
        {
          /*virtual network name matched.*/
          ACE_OS::strncpy((ACE_TCHAR *)pInst->m_nw.m_name, vNwVal.c_str(),
                          (sizeof(pInst->m_nw.m_name) - 1));

          JSON objVirtInterface(objVirtVal["interface"]);
          oval = objVirtInterface["type"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[type] is nullptr\n")));
            /*reclaim the heap memory.*/
            http().unbind(keyStr);
            delete pInst;
            break;
          }

          ACE_OS::strncpy((ACE_TCHAR *)pInst->m_nw.m_type, oval->m_svalue,
                          (sizeof(pInst->m_nw.m_type) - 1));

          oval = objVirtInterface["address"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[address] is nullptr\n")));
            /*reclaim the heap memory.*/
            http().unbind(keyStr);
            delete pInst;
            break;
          }

          sscanf(oval->m_svalue, "%d.%d.%d.%d",
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[0],
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[1],
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[2],
                (ACE_INT32 *)&pInst->m_nw.m_ip.m_ips[3]);

          oval = objVirtInterface["net-mask"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[net-mask] is nullptr\n")));
            /*reclaim the heap memory.*/
            http().unbind(keyStr);
            delete pInst;
            break;
          }

          sscanf((const ACE_TCHAR *)oval->m_svalue, "%d.%d.%d.%d",
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[0],
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[1],
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[2],
                 (ACE_INT32 *)&pInst->m_nw.m_mask.m_ips[3]);

          oval = objVirtInterface["port"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[port] is nullptr\n")));
            /*reclaim the heap memory.*/
            http().unbind(keyStr);
            delete pInst;
            break;
          }

          ACE_OS::strncpy((ACE_TCHAR *)pInst->m_nw.m_port, oval->m_svalue,
                          (sizeof(pInst->m_nw.m_port) - 1));

          /*break the for loop of virtual NW now.*/
          break;
        }
      }

      ACE_OS::free(key);
      key = nullptr;

      /*Now populate remaining dhcp element.*/
      vval = objHttp["ip"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[ip] is nullptr\n")));
        /*reclaim the heap memory.*/
        http().unbind(keyStr);
        delete pInst;
        break;
      }

      sscanf((const ACE_TCHAR *)vval->m_svalue, "%d.%d.%d.%d",
             (ACE_INT32 *)&pInst->m_ip.m_ips[0],
             (ACE_INT32 *)&pInst->m_ip.m_ips[1],
             (ACE_INT32 *)&pInst->m_ip.m_ips[2],
             (ACE_INT32 *)&pInst->m_ip.m_ips[3]);

      vval = objHttp["port"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[port] is nullptr\n")));
        /*reclaim the heap memory.*/
        http().unbind(keyStr);
        delete pInst;
        break;
      }

      pInst->m_port = vval->m_ivalue;
    }

    ret = 0;
  }while(0);


  return(ret);
}

ACE_INT32 CfgMgr::processDHCPServerCfg(void)
{
  ACE_INT32 ret = -1;
  do
  {
    /*Global Object.*/
    JSON glob(m_cpGwCfg->value());

    /*For root object.*/
    JSON inst(m_cpGwCfg->value());
    /*Value of root node of JSON.*/
    JSON::JSONValue *root = inst["cp-gateway"];
    if(!root)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid schema with root\n")));
      break;
    }
    JSON objR(root);

    /*First Child of root node - root.*/
    JSON::JSONValue *rootInstC1 = objR["instances"];
    if(!rootInstC1)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid instance child\n")));
      break;
    }
    JSON objInstRC1(rootInstC1);

    /*First Peer Child of root node.*/
    JSON::JSONValue *rootPeerC1 = objR["peers"];
    if(!rootPeerC1)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid peers child\n")));
      break;
    }
    JSON objPeerRC1(rootPeerC1);

    /*Direct child under root - cp-gateway.*/
    JSON::JSONValue *rootC1 = objR["tun-interface"];
    if(!rootC1)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid child under root\n")));
      break;
    }
    JSON objRootC1(rootC1);

    /*Start Accessing individual element of instance now and put them in ACE HASH MAP.*/
    JSON::JSONValue *rootInstC11 = objInstRC1["dhcp-server"];
    if(!rootInstC11)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid rootInstC11 child under root\n")));
      break;
    }
    /*populating HASH MAP now with instance configuration.*/
    int idx = 0;
    JSON::JSONValue *val = nullptr;
    JSON dhcpServer(rootInstC11);

    /*All instances of DHCP.*/
    for(val = dhcpServer[idx]; val; val = dhcpServer[++idx])
    {
      char *key = ACE_OS::strdup(val->m_svalue);
      ACE_CString keyStr(key, ACE_OS::strlen((const char *)key));

      _CpGwDHCPInstance_t *pDhcp = nullptr;
      ACE_NEW_RETURN(pDhcp, _CpGwDHCPInstance_t(), 0);

      /*Insert into MAP now.*/
      dhcp().bind(keyStr, pDhcp);

      /*dhcp instance name .*/
      JSON::JSONValue *dhcpVal = glob[key];
      JSON objDhcp(dhcpVal);

      /*Individual element of dhcp instance start.*/
      JSON::JSONValue *dhcpVirtNw = objDhcp["virtual-nw"];
      if(!dhcpVirtNw)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid dhcpVirtNw is nullptr key %s\n"), keyStr.c_str()));
        /*reclaim the heap memory.*/
        dhcp().unbind(keyStr);
        delete pDhcp;
        ACE_OS::free(key);
        key = nullptr;
        break;
      }

      ACE_CString dhcpVirtNwName(dhcpVirtNw->m_svalue);

      /*extracting virtual-networks to scan the network name mentioned in dhcp instance.*/
      JSON::JSONValue *virtNw = glob["virtual-networks"];
      if(!virtNw)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr\n")));
        /*reclaim the heap memory.*/
        dhcp().unbind(keyStr);
        delete pDhcp;
        ACE_OS::free(key);
        key = nullptr;
        break;
      }

      JSON objRVirtNw(virtNw);

      int idxx = 0;
      JSON::JSONValue *vval = nullptr;
      for(vval = objRVirtNw[idxx]; vval; vval = objRVirtNw[++idxx])
      {
        JSON objVirtVal(vval);
        JSON::JSONValue *oval = objVirtVal["name"];
        if(!oval)
        {
          ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid virtNw is nullptr\n")));
          /*reclaim the heap memory.*/
          dhcp().unbind(keyStr);
          delete pDhcp;
          break;
        }

        ACE_CString vNwVal(oval->m_svalue, ACE_OS::strlen((const char *)oval->m_svalue));

        if(dhcpVirtNwName == vNwVal)
        {
          /*virtual network name matched.*/
          ACE_OS::strncpy((ACE_TCHAR *)pDhcp->m_nw.m_name, vNwVal.c_str(),
                          (sizeof(pDhcp->m_nw.m_name) - 1));

          JSON objVirtInterface(objVirtVal["interface"]);
          oval = objVirtInterface["type"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[type] is nullptr\n")));
            /*reclaim the heap memory.*/
            dhcp().unbind(keyStr);
            delete pDhcp;
            break;
          }

          ACE_OS::strncpy((ACE_TCHAR *)pDhcp->m_nw.m_type, oval->m_svalue,
                          (sizeof(pDhcp->m_nw.m_type) - 1));

          oval = objVirtInterface["address"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[address] is nullptr\n")));
            /*reclaim the heap memory.*/
            dhcp().unbind(keyStr);
            delete pDhcp;
            break;
          }

          sscanf(oval->m_svalue, "%d.%d.%d.%d",
                (ACE_INT32 *)&pDhcp->m_nw.m_ip.m_ips[0],
                (ACE_INT32 *)&pDhcp->m_nw.m_ip.m_ips[1],
                (ACE_INT32 *)&pDhcp->m_nw.m_ip.m_ips[2],
                (ACE_INT32 *)&pDhcp->m_nw.m_ip.m_ips[3]);

          oval = objVirtInterface["net-mask"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[net-mask] is nullptr\n")));
            /*reclaim the heap memory.*/
            dhcp().unbind(keyStr);
            delete pDhcp;
            break;
          }

          sscanf((const ACE_TCHAR *)oval->m_svalue, "%d.%d.%d.%d",
                 (ACE_INT32 *)&pDhcp->m_nw.m_mask.m_ips[0],
                 (ACE_INT32 *)&pDhcp->m_nw.m_mask.m_ips[1],
                 (ACE_INT32 *)&pDhcp->m_nw.m_mask.m_ips[2],
                 (ACE_INT32 *)&pDhcp->m_nw.m_mask.m_ips[3]);

          oval = objVirtInterface["port"];
          if(!oval)
          {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[port] is nullptr\n")));
            /*reclaim the heap memory.*/
            dhcp().unbind(keyStr);
            delete pDhcp;
            break;
          }

          ACE_OS::strncpy((ACE_TCHAR *)pDhcp->m_nw.m_port, oval->m_svalue,
                          (sizeof(pDhcp->m_nw.m_port) - 1));

          /*break the for loop of virtual NW now.*/
          break;
        }
      }

      ACE_OS::free(key);
      key = nullptr;

      /*processing/extracting dhcp-profile now.*/
      JSON::JSONValue *dhcpProfile = objDhcp["profile-name"];
      if(!dhcpProfile)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid oval[port] is nullptr\n")));
        /*reclaim the heap memory.*/
        dhcp().unbind(keyStr);
        delete pDhcp;
        break;
      }

      JSON objProfile(glob[dhcpProfile->m_svalue]);

      vval = objProfile["mtu"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[mtu] is nullptr\n")));
        /*reclaim the heap memory.*/
        dhcp().unbind(keyStr);
        delete pDhcp;
        break;
      }

      pDhcp->m_profile.m_mtu = vval->m_ivalue;

      vval = objProfile["dns-ip"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[dns-ip] is nullptr\n")));
        /*reclaim the heap memory.*/
        dhcp().unbind(keyStr);
        delete pDhcp;
        break;
      }

      sscanf((const ACE_TCHAR *)vval->m_svalue, "%d.%d.%d.%d",
             (ACE_INT32 *)&pDhcp->m_profile.m_ip.m_ips[0],
             (ACE_INT32 *)&pDhcp->m_profile.m_ip.m_ips[1],
             (ACE_INT32 *)&pDhcp->m_profile.m_ip.m_ips[2],
             (ACE_INT32 *)&pDhcp->m_profile.m_ip.m_ips[3]);

      vval = objProfile["dns-name"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[dns-name] is nullptr\n")));
        /*reclaim the heap memory.*/
        dhcp().unbind(keyStr);
        delete pDhcp;
        break;
      }

      ACE_OS::strncpy((ACE_TCHAR *)pDhcp->m_profile.m_domain_name, vval->m_svalue,
                      (sizeof(pDhcp->m_profile.m_domain_name) - 1));

      vval = objProfile["lease-time"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[lease-time] is nullptr\n")));
        /*reclaim the heap memory.*/
        dhcp().unbind(keyStr);
        delete pDhcp;
        break;
      }

      pDhcp->m_profile.m_lease = vval->m_ivalue;

      /*Now populate remaining dhcp element.*/
      vval = objDhcp["subnet-mask"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[subnet-mask] is nullptr\n")));
        /*reclaim the heap memory.*/
        dhcp().unbind(keyStr);
        delete pDhcp;
        break;
      }

      sscanf((const ACE_TCHAR *)vval->m_svalue, "%d.%d.%d.%d",
             (ACE_INT32 *)&pDhcp->m_mask.m_ips[0],
             (ACE_INT32 *)&pDhcp->m_mask.m_ips[1],
             (ACE_INT32 *)&pDhcp->m_mask.m_ips[2],
             (ACE_INT32 *)&pDhcp->m_mask.m_ips[3]);

      vval = objDhcp["ip"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[ip] is nullptr\n")));
        /*reclaim the heap memory.*/
        dhcp().unbind(keyStr);
        delete pDhcp;
        break;
      }

      sscanf((const ACE_TCHAR *)vval->m_svalue, "%d.%d.%d.%d",
             (ACE_INT32 *)&pDhcp->m_ip.m_ips[0],
             (ACE_INT32 *)&pDhcp->m_ip.m_ips[1],
             (ACE_INT32 *)&pDhcp->m_ip.m_ips[2],
             (ACE_INT32 *)&pDhcp->m_ip.m_ips[3]);

      vval = objDhcp["host-name"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[host-name] is nullptr\n")));
        /*reclaim the heap memory.*/
        dhcp().unbind(keyStr);
        delete pDhcp;
        break;
      }

      ACE_OS::strncpy((ACE_TCHAR *)pDhcp->m_host_name, vval->m_svalue,
                      (sizeof(pDhcp->m_host_name) - 1));

      vval = objDhcp["start-ip"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[start-ip] is nullptr\n")));
        /*reclaim the heap memory.*/
        dhcp().unbind(keyStr);
        delete pDhcp;
        break;
      }

      sscanf((const ACE_TCHAR *)vval->m_svalue, "%d.%d.%d.%d",
             (ACE_INT32 *)&pDhcp->m_start_ip.m_ips[0],
             (ACE_INT32 *)&pDhcp->m_start_ip.m_ips[1],
             (ACE_INT32 *)&pDhcp->m_start_ip.m_ips[2],
             (ACE_INT32 *)&pDhcp->m_start_ip.m_ips[3]);

      vval = objDhcp["end-ip"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[end-ip] is nullptr\n")));
        /*reclaim the heap memory.*/
        dhcp().unbind(keyStr);
        delete pDhcp;
        break;
      }

      sscanf((const ACE_TCHAR *)vval->m_svalue, "%d.%d.%d.%d",
             (ACE_INT32 *)&pDhcp->m_end_ip.m_ips[0],
             (ACE_INT32 *)&pDhcp->m_end_ip.m_ips[1],
             (ACE_INT32 *)&pDhcp->m_end_ip.m_ips[2],
             (ACE_INT32 *)&pDhcp->m_end_ip.m_ips[3]);

      vval = objDhcp["exclude-ip"];
      if(!vval)
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid vval[exclude-ip] is nullptr\n")));
        /*reclaim the heap memory.*/
        dhcp().unbind(keyStr);
        delete pDhcp;
        break;
      }

      JSON objExIp(vval);
      /*for loop for exclude-ip list.*/
      int exIdx = 0;
      for(vval = objExIp[exIdx]; vval; vval = objExIp[++exIdx])
      {
        sscanf((const ACE_TCHAR *)vval->m_svalue, "%d.%d.%d.%d",
               (ACE_INT32 *)&pDhcp->m_exclude_ip[exIdx].m_ips[0],
               (ACE_INT32 *)&pDhcp->m_exclude_ip[exIdx].m_ips[1],
               (ACE_INT32 *)&pDhcp->m_exclude_ip[exIdx].m_ips[2],
               (ACE_INT32 *)&pDhcp->m_exclude_ip[exIdx].m_ips[3]);
      }

      pDhcp->m_ip_count = exIdx;
    }

    ret = 0;
  }while(0);

  return(ret);
}

ACE_INT32 CfgMgr::processCfg(void)
{
  processDHCPServerCfg();
  processDHCPAgentCfg();
  processHTTPServerCfg();
  processCPGWCfg();
  processAPCfg();
  processAAACfg();

  return(0);
}

ACE_Byte CfgMgr::start(void)
{
  m_cpGwCfg = JSON::instance();
  if(nullptr != m_cpGwCfg)
  {
    /*start parsing CPGateway Config in JSON format.*/
    if(m_cpGwCfg->start(m_schema.c_str()))
    {
      delete m_cpGwCfg;
      m_cpGwCfg = nullptr;

      /*Parsing of JSON Schema Failed.*/
      return(1);
    }
  }

  processCfg();

  /*release the memory.*/
  m_cpGwCfg->stop();
  delete m_cpGwCfg;
  m_cpGwCfg = nullptr;

  return(0);
}

ACE_Byte CfgMgr::stop(void)
{
  purgeDHCP();
  purgeDHCPAgent();
  purgeAAA();
  purgeAPInst();
  purgeHTTP();
  purgeCPGW();

  return(0);
}

ACE_Byte CfgMgr::purgeDHCP(void)
{
  _CpGwDHCPInstance_t *inst = NULL;
  DHCPInstMap_Iter_t iter(dhcp().begin());

  for(DHCPInstMap_t::ENTRY *entry = nullptr; iter.next(entry);)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwDHCPInstance_t *)((*entry).int_id_);
    ACE_CString instName = (ACE_CString)((*entry).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));
    iter.advance();
    dhcp().unbind(instName);
    delete inst;
  }

  return(0);
}

ACE_Byte CfgMgr::purgeDHCPAgent(void)
{
  _CpGwDHCPAgentInstance_t *inst = NULL;
  DHCPAgentInstMap_Iter_t iter(agent().begin());

  for(DHCPAgentInstMap_t::ENTRY *entry = nullptr; iter.next(entry);)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwDHCPAgentInstance_t *)((*entry).int_id_);
    ACE_CString instName = (ACE_CString)((*entry).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));
    iter.advance();
    agent().unbind(instName);
    delete inst;
  }

  return(0);
}

ACE_Byte CfgMgr::purgeAPInst(void)
{
  _CpGwAPInstance_t *inst = NULL;
  APInstMap_Iter_t iter(ap().begin());

  for(APInstMap_t::ENTRY *entry = nullptr; iter.next(entry);)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwAPInstance_t *)((*entry).int_id_);
    ACE_CString instName = (ACE_CString)((*entry).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));
    iter.advance();
    ap().unbind(instName);
    delete inst;
  }

  return(0);
}

ACE_Byte CfgMgr::purgeHTTP(void)
{
  _CpGwHTTPInstance_t *inst = NULL;
  HTTPInstMap_Iter_t iter(http().begin());

  for(HTTPInstMap_t::ENTRY *entry = nullptr; iter.next(entry);)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwHTTPInstance_t *)((*entry).int_id_);
    ACE_CString instName = (ACE_CString)((*entry).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));
    iter.advance();
    http().unbind(instName);
    delete inst;
  }

  return(0);
}

ACE_Byte CfgMgr::purgeAAA(void)
{
  _CpGwAAAInstance_t *inst = NULL;
  AAAInstMap_Iter_t iter = aaa().begin();

  for(AAAInstMap_t::ENTRY *entry = nullptr; iter.next(entry);)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwAAAInstance_t *)((*entry).int_id_);
    ACE_CString instName = (ACE_CString)((*entry).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));
    iter.advance();
    aaa().unbind(instName);
    delete inst;
  }

  return(0);
}

ACE_Byte CfgMgr::purgeCPGW(void)
{
  _CpGwCPGWInstance_t *inst = NULL;
  CPGWInstMap_Iter_t iter = cpgw().begin();

  for(CPGWInstMap_t::ENTRY *entry = nullptr; iter.next(entry);)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwCPGWInstance_t *)((*entry).int_id_);
    ACE_CString instName = (ACE_CString)((*entry).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));
    iter.advance();
    cpgw().unbind(instName);
    delete inst;
  }

  return(0);
}

void CfgMgr::display(void)
{
  displayDHCP();
  displayDHCPAgent();
  displayAAA();
  displayAPInst();
  displayHTTP();
  displayCPGW();
}

void CfgMgr::displayDHCP(void)
{
  _CpGwDHCPInstance_t *inst = nullptr;
  DHCPInstMap_Iter_t iter = dhcp().begin();

  //for(; iter != dhcp().end(); iter++)
  for(DHCPInstMap_t::ENTRY *entry = nullptr; iter.next(entry); iter.advance())
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwDHCPInstance_t *)((*entry).int_id_);
    ACE_CString instName = (ACE_CString)((*entry).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));

    /*virtual networks.*/
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_name is %s\n"), inst->m_nw.m_name));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_type is %s\n"), inst->m_nw.m_type));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_ip is 0x%X\n"), inst->m_nw.m_ip.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_mask is 0x%X\n"), inst->m_nw.m_mask.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_port is %s\n"), inst->m_nw.m_port));
    /*Profile .*/
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_mtu is %u\n"), inst->m_profile.m_mtu));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_ip is 0x%X\n"), inst->m_profile.m_ip.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_domain_name is %s\n"), inst->m_profile.m_domain_name));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_lease is %u\n"), inst->m_profile.m_lease));

    /*Individual elements.*/
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_mask is 0x%X\n"), inst->m_mask.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_ip is 0x%X\n"), inst->m_ip.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_host_name is %s\n"), inst->m_host_name));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_start_ip is 0x%X\n"), inst->m_start_ip));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_end_ip is 0x%X\n"), inst->m_end_ip));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_ip_count is %u\n"), inst->m_ip_count));
  }
}

void CfgMgr::displayDHCPAgent(void)
{
  _CpGwDHCPAgentInstance_t *inst = NULL;
  DHCPAgentInstMap_Iter_t iter = agent().begin();

  for(; iter != agent().end(); iter++)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwDHCPAgentInstance_t *)((*iter).int_id_);
    ACE_CString instName = (ACE_CString)((*iter).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));

    /*virtual networks.*/
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_name is %s\n"), inst->m_nw.m_name));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_type is %s\n"), inst->m_nw.m_type));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_ip is 0x%X\n"), inst->m_nw.m_ip.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_mask is 0x%X\n"), inst->m_nw.m_mask.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_port is %s\n"), inst->m_nw.m_port));

    /*Individual elements.*/
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_mask is 0x%X\n"), inst->m_mask.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_ip is 0x%X\n"), inst->m_ip.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_host_name is %s\n"), inst->m_host_name));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_server_ip is 0x%X\n"), inst->m_server_ip.m_ipn));
  }
}

void CfgMgr::displayAAA(void)
{
  _CpGwAAAInstance_t *inst = NULL;
  AAAInstMap_Iter_t iter = aaa().begin();

  for(; iter != aaa().end(); iter++)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwAAAInstance_t *)((*iter).int_id_);
    ACE_CString instName = (ACE_CString)((*iter).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));

    /*virtual networks.*/
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_name is %s\n"), inst->m_nw.m_name));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_type is %s\n"), inst->m_nw.m_type));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_ip is 0x%X\n"), inst->m_nw.m_ip.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_mask is 0x%X\n"), inst->m_nw.m_mask.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_port is %s\n"), inst->m_nw.m_port));

    /*Individual elements.*/
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_ip is 0x%X\n"), inst->m_ip.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_protocol is %s\n"), inst->m_protocol));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_auth_port is 0x%X\n"), inst->m_auth_port));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_acc_port is 0x%X\n"), inst->m_acc_port));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_admin_user is %s\n"), inst->m_admin_user));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_admin_pwd is %s\n"), inst->m_admin_pwd));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_peer_ip is 0x%X\n"), inst->m_peer_ip.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_peer_port is %u\n"), inst->m_peer_port));
  }
}

void CfgMgr::displayAPInst(void)
{
  _CpGwAPInstance_t *inst = NULL;
  APInstMap_Iter_t iter = ap().begin();

  for(; iter != ap().end(); iter++)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwAPInstance_t *)((*iter).int_id_);
    ACE_CString instName = (ACE_CString)((*iter).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));

    /*virtual networks.*/
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_name is %s\n"), inst->m_nw.m_name));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_type is %s\n"), inst->m_nw.m_type));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_ip is 0x%X\n"), inst->m_nw.m_ip.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_mask is 0x%X\n"), inst->m_nw.m_mask.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_port is %s\n"), inst->m_nw.m_port));

    /*Individual elements.*/
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_ap_name is %s\n"), inst->m_ap_name));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_latitude is %s\n"), inst->m_latitude));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_longitude is %s\n"), inst->m_longitude));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_elevation is %s\n"), inst->m_elevation));
  }
}

void CfgMgr::displayHTTP(void)
{
  _CpGwHTTPInstance_t *inst = NULL;
  HTTPInstMap_Iter_t iter = http().begin();

  for(; iter != http().end(); iter++)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwHTTPInstance_t *)((*iter).int_id_);
    ACE_CString instName = (ACE_CString)((*iter).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));

    /*virtual networks.*/
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_name is %s\n"), inst->m_nw.m_name));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_type is %s\n"), inst->m_nw.m_type));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_ip is 0x%X\n"), inst->m_nw.m_ip.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_mask is 0x%X\n"), inst->m_nw.m_mask.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_port is %s\n"), inst->m_nw.m_port));

    /*Individual elements.*/
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_ip is 0x%X\n"), inst->m_ip.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_port is %u\n"), inst->m_port));
  }
}

void CfgMgr::displayCPGW(void)
{
  _CpGwCPGWInstance_t *inst = NULL;
  CPGWInstMap_Iter_t iter = cpgw().begin();

  for(; iter != cpgw().end(); iter++)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwCPGWInstance_t *)((*iter).int_id_);
    ACE_CString instName = (ACE_CString)((*iter).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));

    /*virtual networks.*/
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_name is %s\n"), inst->m_nw.m_name));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_type is %s\n"), inst->m_nw.m_type));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_ip is 0x%X\n"), inst->m_nw.m_ip.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_mask is 0x%X\n"), inst->m_nw.m_mask.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_port is %s\n"), inst->m_nw.m_port));

    /*Individual elements.*/
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_mask is 0x%X\n"), inst->m_mask.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_ip is 0x%X\n"), inst->m_ip.m_ipn));
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l m_host_name is %s\n"), inst->m_host_name));
  }
}

/*
 * @brief This method does the actual message/request processing.
 * @param Reference to ACE_Message_Block.
 * @return Upon success 0 else -1.
 * */
int CfgMgr::processIPCMessage(ACE_Message_Block &mb)
{
  ACE_Byte *in = nullptr;
  ACE_UINT32 len = 0;
  int rspLen = -1;

  /*Process The Request.*/
  CommonIF::_cmMessage_t *cMsg = (CommonIF::_cmMessage_t *)mb.rd_ptr();

  ACE_UINT32 msgType = cMsg->m_msgType;

  in = (ACE_Byte *)mb.rd_ptr();
  len = mb.length();

  switch(msgType)
  {
  case CommonIF::MSG_CPGW_CFGMGR_CONFIG_REQ:
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l Config Request Received of len %u\n"), len));

    ACE_Message_Block *mbRsp = nullptr;
    ACE_NEW_NORETURN(mbRsp, ACE_Message_Block(CommonIF::SIZE_8MB));

    buildConfigResponse(in, len, *mbRsp);
    len = send_ipc((ACE_Byte *)mbRsp->rd_ptr(), (ACE_UINT32)mbRsp->length());
    mbRsp->release();

    break;
  }

  return(0);
}

int CfgMgr::buildIPCHeader(ACE_Byte *in, ACE_Message_Block &mb)
{
  CommonIF::_cmMessage_t *req = (CommonIF::_cmMessage_t *)in;

  /*fill destination details.*/
  *((ACE_UINT32 *)mb.wr_ptr()) = req->m_src.m_procId;
  mb.wr_ptr(4);
  *((ACE_UINT8 *)mb.wr_ptr()) = req->m_src.m_entId;
  mb.wr_ptr(1);
  *((ACE_UINT8 *)mb.wr_ptr()) = req->m_src.m_instId;
  mb.wr_ptr(1);

  /*fill source details.*/
  *((ACE_UINT32 *)mb.wr_ptr()) = req->m_dst.m_entId;
  mb.wr_ptr(4);
  *((ACE_UINT8 *)mb.wr_ptr()) = req->m_dst.m_instId;
  mb.wr_ptr(1);
  *((ACE_UINT8 *)mb.wr_ptr()) = req->m_dst.m_procId;
  mb.wr_ptr(1);

  return(0);
}

/*
 * @brief This method is prepares the Config Response based on schema which is
 *        provided in JSON format.
 * @param Pointer to ACE_Byte which is received in config Request.
 * @param length of the input request.
 * @param This is the out buffer in form of ACE_Message_Block which is invoed call by reference.
 * @return 0 upon success , -1 else.
 * */
int CfgMgr::buildConfigResponse(ACE_Byte *in , ACE_UINT32 len, ACE_Message_Block &mb)
{
  int idx = 0;;

  if(!in)
  {
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l pointer to in is nullptr\n")));
    return(-1);
  }

  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l sizeof struct %u\n"), sizeof(_CpGwConfigs_t)));

  CommonIF::_cmMessage_t *rsp = (CommonIF::_cmMessage_t *)mb.wr_ptr();
  _CpGwConfigs_t *configRsp = (_CpGwConfigs_t *)rsp->m_message;

  CommonIF::_cmMessage_t *req = (CommonIF::_cmMessage_t *)in;

  ACE_OS::memset((void *)rsp, 0, sizeof(CommonIF::_cmMessage_t));
  ACE_OS::memset((void *)configRsp, 0, sizeof(_CpGwConfigs_t));

  rsp->m_dst.m_procId = get_self_procId(); //req->m_src.m_procId;
  rsp->m_dst.m_entId = req->m_src.m_entId;
  rsp->m_dst.m_instId = req->m_src.m_instId;

  rsp->m_src.m_procId = get_self_procId(); //req->m_dst.m_procId;
  rsp->m_src.m_entId = req->m_dst.m_entId;
  rsp->m_src.m_instId = req->m_dst.m_instId;

  rsp->m_msgType = CommonIF::MSG_CFGMGR_CPGW_CONFIG_RSP;

  /*block scoped declaration of identifier inst.*/
  {
    /*Preparing DHCPInstance Config.*/
    _CpGwDHCPInstance_t *inst = nullptr;
    DHCPInstMap_Iter_t iter = dhcp().begin();

    for(idx = 0; iter != dhcp().end(); iter++, idx++)
    {
      inst = (_CpGwDHCPInstance_t *)((*iter).int_id_);
      //mb.copy((const ACE_TCHAR *)inst, sizeof(_CpGwDHCPInstance_t));
      ACE_OS::memcpy((void *)&configRsp->m_instance.m_instDHCP[idx], (const void *)inst, sizeof(_CpGwDHCPInstance_t));
    }

    configRsp->m_instance.m_DHCPInstCount = idx;
    rsp->m_messageLen = (idx * sizeof(_CpGwDHCPInstance_t));
  }

  {
    /*Preparing for DHCPAgent */
    _CpGwDHCPAgentInstance_t *inst = nullptr;
    DHCPAgentInstMap_Iter_t iter = agent().begin();

    for(idx = 0; iter != agent().end(); iter++, idx++)
    {
      inst = (_CpGwDHCPAgentInstance_t *)((*iter).int_id_);
      ACE_OS::memcpy((void *)&configRsp->m_instance.m_instDHCPAgent[idx], (const void *)inst, sizeof(_CpGwDHCPAgentInstance_t));
    }

    configRsp->m_instance.m_DHCPAgentInstCount = idx;
    rsp->m_messageLen += (idx * sizeof(_CpGwDHCPAgentInstance_t));

  }

  {
    /*Preparing for HTTP Instance*/
    _CpGwHTTPInstance_t *inst = nullptr;
    HTTPInstMap_Iter_t iter = http().begin();

    for(idx = 0; iter != http().end(); iter++, idx++)
    {
      inst = (_CpGwHTTPInstance_t *)((*iter).int_id_);
      ACE_OS::memcpy((void *)&configRsp->m_instance.m_instHTTP[idx], (const void *)inst, sizeof(_CpGwHTTPInstance_t));
    }

    configRsp->m_instance.m_HTTPInstCount = idx;
    rsp->m_messageLen += (idx * sizeof(_CpGwHTTPInstance_t));

  }

  {
    /*Preparing for AAA Instance*/
    _CpGwAAAInstance_t *inst = nullptr;
    AAAInstMap_Iter_t iter = aaa().begin();

    for(idx = 0; iter != aaa().end(); iter++, idx++)
    {
      inst = (_CpGwAAAInstance_t *)((*iter).int_id_);
      ACE_OS::memcpy((void *)&configRsp->m_instance.m_instAAA[idx], (const void *)inst, sizeof(_CpGwAAAInstance_t));
    }

    configRsp->m_instance.m_AAAInstCount = idx;
    rsp->m_messageLen += (idx * sizeof(_CpGwAAAInstance_t));
  }

  {
    /*Preparing for CPGW Instance*/
    _CpGwCPGWInstance_t *inst = nullptr;
    CPGWInstMap_Iter_t iter = cpgw().begin();

    for(idx = 0; iter != cpgw().end(); iter++, idx++)
    {
      inst = (_CpGwCPGWInstance_t *)((*iter).int_id_);
      ACE_OS::memcpy((void *)&configRsp->m_instance.m_instCPGW[idx], (const void *)inst, sizeof(_CpGwCPGWInstance_t));
    }

    configRsp->m_instance.m_CPGWInstCount = idx;
    rsp->m_messageLen += (idx * sizeof(_CpGwCPGWInstance_t));
  }

  mb.wr_ptr(sizeof(_CpGwConfigs_t));
  return(0);
}

ACE_HANDLE CfgMgr::get_handle(void) const
{
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l CfgMgr::get_handle\n")));
  return(const_cast<CfgMgr *>(this)->handle());
}

/*
 * @brief this is the hook method and is invoked by IPC framework. IPC framework allocates
 *        memory and passes the request to process. The request is pass on to Active Object and
 *        control is returned to Reactor for next request.
 * @param Pointer to ACE_Message_Block which has it wr_ptr populated with request byte buffer.
 * @return Upon success 0 else -1. When -1 is returned the reactor closes the handle.
 **/
ACE_UINT32 CfgMgr::handle_ipc(ACE_Message_Block *mb)
{
  /*Posting to the active object now.*/
  putq(mb);
  return(0);
}

/*
 * @brief This method is the hook method of Active Object. Which is blocked on message Queue for its request.
 *        When it receives request from the Queue then it processes it and finally releases the memory.
 * @param none
 * @return upon success returns 0.
 * */
int CfgMgr::svc(void)
{
  ACE_Message_Block *mb = nullptr;

  for(;-1 != getq(mb);)
  {
    /*Process IPC Request Now.*/
    if(mb->msg_type() == ACE_Message_Block::MB_HANGUP)
    {
      mb->release();
      break;
    }

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l dequeue equest\n")));
    processIPCMessage(*mb);
    /*reclaim the heap memory now. allocated by the sender*/
    mb->release();
  }

  return(0);
}

void CfgMgr::schema(ACE_CString sc)
{
  m_schema = sc;
}


#endif /*__CFG_MGR_CC__*/
