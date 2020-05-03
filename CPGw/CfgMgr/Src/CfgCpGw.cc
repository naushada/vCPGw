#ifndef __CFG_CPGW_CC__
#define __CFG_CPGW_CC__

#include "CfgCpGw.h"

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

CfgMgr::~CfgMgr()
{
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
  DHCPInstMap_Iter_t iter = dhcp().begin();

  for(; iter != dhcp().end(); iter++)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwDHCPInstance_t *)((*iter).int_id_);
    ACE_CString instName = (ACE_CString)((*iter).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));

    dhcp().unbind(instName);
    delete inst;
  }

  return(0);
}

ACE_Byte CfgMgr::purgeDHCPAgent(void)
{
  _CpGwDHCPAgentInstance_t *inst = NULL;
  DHCPAgentInstMap_Iter_t iter = agent().begin();

  for(; iter != agent().end(); iter++)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwDHCPAgentInstance_t *)((*iter).int_id_);
    ACE_CString instName = (ACE_CString)((*iter).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));

    agent().unbind(instName);
    delete inst;
  }

  return(0);
}

ACE_Byte CfgMgr::purgeAPInst(void)
{
  _CpGwAPInstance_t *inst = NULL;
  APInstMap_Iter_t iter = ap().begin();

  for(; iter != ap().end(); iter++)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwAPInstance_t *)((*iter).int_id_);
    ACE_CString instName = (ACE_CString)((*iter).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));

    ap().unbind(instName);
    delete inst;
  }

  return(0);
}

ACE_Byte CfgMgr::purgeHTTP(void)
{
  _CpGwHTTPInstance_t *inst = NULL;
  HTTPInstMap_Iter_t iter = http().begin();

  for(; iter != http().end(); iter++)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwHTTPInstance_t *)((*iter).int_id_);
    ACE_CString instName = (ACE_CString)((*iter).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));

    http().unbind(instName);
    delete inst;
  }

  return(0);
}

ACE_Byte CfgMgr::purgeAAA(void)
{
  _CpGwAAAInstance_t *inst = NULL;
  AAAInstMap_Iter_t iter = aaa().begin();

  for(; iter != aaa().end(); iter++)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwAAAInstance_t *)((*iter).int_id_);
    ACE_CString instName = (ACE_CString)((*iter).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));

    aaa().unbind(instName);
    delete inst;
  }

  return(0);
}

ACE_Byte CfgMgr::purgeCPGW(void)
{
  _CpGwCPGWInstance_t *inst = NULL;
  CPGWInstMap_Iter_t iter = cpgw().begin();

  for(; iter != cpgw().end(); iter++)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwCPGWInstance_t *)((*iter).int_id_);
    ACE_CString instName = (ACE_CString)((*iter).ext_id_);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %N:%l instance Name is %s\n"), instName.c_str()));

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
  _CpGwDHCPInstance_t *inst = NULL;
  DHCPInstMap_Iter_t iter = dhcp().begin();

  for(; iter != dhcp().end(); iter++)
  {
    /*int_id_ is the Value, ext_id_ is the key of ACE_Hash_Map_Manager.*/
    inst = (_CpGwDHCPInstance_t *)((*iter).int_id_);
    ACE_CString instName = (ACE_CString)((*iter).ext_id_);
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

#endif /*__CFG_CPGW_CC__*/
