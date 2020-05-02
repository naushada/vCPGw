#ifndef __CFG_CPGW_CC__
#define __CFG_CPGW_CC__

#include "CfgCpGw.h"

CfgMgr::CfgMgr(ACE_CString &schema)
{
  m_cpGwCfg = nullptr;
  m_aaa.unbind_all();
  m_http.unbind_all();
  m_ap.unbind_all();
  m_dhcp.unbind_all();
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

CfgMgr::~CfgMgr()
{
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

    JSON::JSONValue *rootInstC12 = objInstRC1["dhcp-agent"];
    if(!rootInstC12)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid rootInstC12 child under root\n")));
      break;
    }

    JSON::JSONValue *rootInstC13 = objInstRC1["http-server"];
    if(!rootInstC13)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid rootInstC13 child under root\n")));
      break;
    }

    JSON::JSONValue *rootInstC14 = objInstRC1["cp-gw"];
    if(!rootInstC14)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid rootInstC14 child under root\n")));
      break;
    }

    JSON::JSONValue *rootInstC15 = objInstRC1["access-points"];
    if(!rootInstC15)
    {
      ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %N:%l Invalid rootInstC15 child under root\n")));
      break;
    }


    /*Peer children of root.*/
    JSON::JSONValue *rootPeerC11 = objPeerRC1["aaa-server"];
    /*Populate HASH MAP now with peers configuration.*/

    ret = 0;
  }while(0);

  return(ret);
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

  processDHCPServerCfg();

  /*release the memory.*/
  m_cpGwCfg->stop();
  delete m_cpGwCfg;
  m_cpGwCfg = nullptr;

  return(0);
}

ACE_Byte CfgMgr::stop(void)
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

void CfgMgr::display(void)
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
#endif /*__CFG_CPGW_CC__*/
