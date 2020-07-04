#ifndef __RESTCLI_CC__
#define __RESTCLI_CC__

#include <ace/Message_Block.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "cli.h"
#include "tclient.h"

int Cli::m_offset = 0;
int Cli::m_len = 0;
char *Cli::m_cmdName = nullptr;
int Cli::m_argOffset = 0;

Cli::Command Cli::m_command[256] =
{
  /* command name */           /* command argument(s) */                 /* command description */
  { "ping",                    {/*argument list*/ nullptr},                 "pings hostapd" },
  { "mib",                     {nullptr},                 "get MIB variables (dot1x, dot11, radius)" },
  { "relog",                   {nullptr},                 "reload/truncate debug log output file" },
  { "status",                  {nullptr},                 "show interface status info" },
  { "sta",                     {nullptr},                 "<addr> = get MIB variables for one station" },
  { "all_sta",                 {nullptr},                 "get MIB variables for all stations" },
  { "list_sta",                {nullptr},                 "list all stations" },
  { "new_sta",                 {nullptr},                 "<addr> = add a new station" },
  { "deauthenticate",          {nullptr},                 "<addr> = deauthenticate a station" },
  { "disassociate",            {nullptr},                 "<addr> = disassociate a station" },
  { "signature",               {nullptr},                 "<addr> = get taxonomy signature for a station" },
  { "sa_query",                {nullptr},                 "<addr> = send SA Query to a station" },
  { "wps_pin",                 {nullptr},                 "<uuid> <pin> [timeout] [addr] = add WPS "
                                                       "Enrollee PIN" },
  { "wps_check_pin",           {nullptr},                 "<PIN> = verify PIN checksum" },
  { "wps_pbc",                 {nullptr},                 "indicate button pushed to initiate PBC" },
  { "wps_cancel",              {nullptr},                 " cancel the pending WPS operation" },
  { "wps_nfc_tag_read",        {nullptr},                 "<hexdump> = report read NFC tag with WPS data" },
  { "wps_nfc_config_token",    {nullptr},                 "<WPS/NDEF> = build NFC configuration token" },
  { "wps_nfc_token",           {nullptr},                 "<WPS/NDEF/enable/disable> = manager "
                                                       "NFC password token" },
  { "nfc_get_handover_sel",    {nullptr},                 nullptr },
  /* command name */           /* command argument(s) */                 /* command description */
  { "wps_ap_pin", {"disable",
                   "random",
                   "get",
                   "set",
                   nullptr},                 "<cmd> [params..] = enable/disable AP PIN" },
  { "wps_config",              {nullptr},                 "<SSID> <auth> <encr> <key> = configure AP" },
  { "wps_get_status",          {nullptr},                 "show current WPS status" },
  { "disassoc_imminent",       {nullptr},                 "send Disassociation Imminent notification" },
  { "ess_disassoc",            {nullptr},                 "send ESS Dissassociation Imminent notification" },
  { "bss_tm_req", {"disassoc_timer=",
                   "valid_int=",
                   "bss_term=",
                   "url=",
                   "pref=1",
                   "abridged=1",
                   nullptr},                 "send BSS Transition Management Request, First Argument is the mac address in hex contiguous bytes" },
  { "get_config",              {nullptr},                 "show current configuration" },
  { "help",                    {nullptr},                 "= show this usage help" },
  { "interface",               {nullptr},                 "[ifname] = show interfaces/select interface" },
  { "fst",                     {nullptr},                 "<params...> = send FST-MANAGER control "
                                                       "interface command" },
  { "raw",                     {nullptr},                 "<params..> = send unprocessed command" },
  { "level",                   {nullptr},                 "<debug level> = change debug level" },
  { "license",                 {nullptr},                 "show full hostapd_cli license" },
  { "quit",                    {nullptr},                 "exit hostapd_cli" },
  { "set",  {"wps_version_number=",
             "wps_testing_dummy_cred=",
             "wps_corrupt_pkhash=",
             "ext_mgmt_frame_handling=",
             "ext_eapol_frame_io=",
             "dpp_config_obj_override=",
             "dpp_discovery_override=",
             "dpp_groups_override=",
             "dpp_ignore_netaccesskey_mismatch=",
             "dpp_test=",
             "mbo_assoc_disallow=",
             "dpp_configurator_params=",
             "deny_mac_file=",
             "accept_mac_file=",
             nullptr},
                                                        "<name> <value> = set runtime variables" },
  /*get with arguments*/
  { "get",                     {"version", "tls_library", nullptr}, "<name> = get runtime info" },
  { "set_qos_map_set",         {nullptr},                 "<arg,arg,...> = set QoS Map set element" },
  { "send_qos_map_conf",       {nullptr},                 "<addr> = send QoS Map Configure frame" },
  { "chan_switch",             {nullptr},                 "<cs_count> <freq> [sec_channel_offset=] "
  /* command name */           /* command argument(s) */                 /* command description */
                                                       "[center_freq1=]\n"
                                                       "[center_freq2=] [bandwidth=] [blocktx] "
                                                       "[ht|vht]\n"
                                                       "initiate channel switch announcement" },
  { "hs20_wnm_notif",          {nullptr},                 "<addr> <url>\n"
                                                       "send WNM-Notification Subscription "
                                                       "Remediation Request" },
  { "hs20_deauth_req",         {nullptr},                 "<addr> <code (0/1)> <Re-auth-Delay(sec)> [url]\n"
                                                       "send WNM-Notification imminent deauthentication "
                                                       "indication" },
  { "vendor",                  {nullptr},                 "<vendor id> <sub command id> "
                                                       "[<hex formatted data>]\n"
                                                       "send vendor driver command" },
  { "enable",                  {nullptr},                 "enable hostapd on current interface" },
  { "reload",                  {nullptr},                 "reload configuration for current interface" },
  { "disable",                 {nullptr},                 "disable hostapd on current interface" },
  { "update_beacon",           {nullptr},                 "update Beacon frame contents\n"},
  { "erp_flush",               {nullptr},                 "drop all ERP keys"},
  { "log_level",               {nullptr},                 "[level] = show/change log verbosity level" },
  { "pmksa",                   {nullptr},                 "show PMKSA cache entries" },
  { "pmksa_flush",             {nullptr},                 "flush PMKSA cache" },
  { "set_neighbor",            {nullptr},                 "<addr> <ssid=> <nr=> [lci=] [civic=] [stat]\n"
                                                       "  = add AP to neighbor database" },
  { "remove_neighbor",         {nullptr},                 "<addr> <ssid=> = remove AP from neighbor "
                                                       "database" },
  { "req_lci",                 {nullptr},                 "<addr> = send LCI request to a station"},
  /* command name */           /* command argument(s) */                 /* command description */
  { "req_range",               {nullptr},                 "send FTM range request"},
  { "driver_flags",            {nullptr},                 " = show supported driver flags"},
  { "dpp_qr_code",             {nullptr},                 "report a scanned DPP URI from a QR Code" },
  { "dpp_bootstrap_gen",       {nullptr},                 "type=<qrcode> [chan=..] [mac=..] [info=..] "
                                                       "[curve=..] [key=..] = generate DPP bootstrap "
                                                       "information" },
  { "dpp_bootstrap_remove",    {nullptr},                 "*|<id> = remove DPP bootstrap information" },
  { "dpp_bootstrap_get_uri",   {nullptr},                 "<id> = get DPP bootstrap URI" },
  { "dpp_bootstrap_info",      {nullptr},                 "<id> = show DPP bootstrap information" },
  { "dpp_auth_init",           {nullptr},                 "peer=<id> [own=<id>] = initiate DPP "
                                                       "bootstrapping" },
  { "dpp_listen",              {nullptr},                 "<freq in MHz> = start DPP listen" },
  { "dpp_stop_listen",         {nullptr},                 "= stop DPP listen" },
  { "dpp_configurator_add",    {nullptr},                 "[curve=..] [key=..] = add DPP configurator" },
  { "dpp_configurator_remove", {nullptr},                 "*|<id> = remove DPP configurator" },
  { "dpp_configurator_get_key",{nullptr},                 "<id> = Get DPP configurator's private key" },
  { "dpp_configurator_sign",   {nullptr},                 "conf=<role> configurator=<id> = generate "
                                                       "self DPP configuration" },
  { "dpp_pkex_add",            {nullptr},                 "add PKEX code" },
  { "dpp_pkex_remove",         {nullptr},                 "*|<id> = remove DPP pkex information" },
  { "accept_acl",              {nullptr},                 "Add/Delete/Show/Clear accept MAC ACL" },
  /* command name */           /* command argument(s) */                 /* command description */
  { "deny_acl",                {nullptr},                 "Add/Delete/Show/Clear deny MAC ACL" },
  { "poll_sta",                {nullptr},                 "<addr> = poll a STA to check connectivity "
                                                       "with a QoS nullptr frame" },
  { "req_beacon",              {nullptr},                 "<addr> [req_mode=] <measurement request "
                                                       "hexdump>  = "
                                                       "send a Beacon report request to a station" },
  { "reload_wpa_psk",          {nullptr},                 "reload wpa_psk_file only" },
  { nullptr,                      {nullptr},                  nullptr }
};

char *Cli::cmdName(void)
{
  return(m_cmdName);
}

void Cli::cmdName(char *cmdName)
{
  if(cmdName)
  {
    m_cmdName = strdup(cmdName);
  }
}

/*
 * @brief
 * @param
 * @return
 */
void Cli::prompt(char *prompt)
{
  m_prompt = strdup(prompt);
}

/*
 * @brief
 * @param
 * @return
 */
char *Cli::prompt()
{
  return(m_prompt);
}

char *commandArgListGenerator(const char *text, int state)
{
  /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
  if (!state)
  {
      Cli::m_argOffset = 0;
  }

  int &idx = Cli::m_offset;
  int &inner = Cli::m_argOffset;

  if(nullptr == Cli::m_command[idx].argv[inner])
    return (char *)nullptr;

  return(strdup(Cli::m_command[idx].argv[inner++]));
}

char *commandArgGenerator(const char *text, int state)
{
  const char *name;
  /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
  if (!state)
  {
      Cli::m_argOffset = 0;
  }

  int &idx = Cli::m_offset;
  int &inner = Cli::m_argOffset;
  /* Return the next name which partially matches from the command list. */
  while(nullptr != (name = Cli::m_command[idx].argv[inner]))
  {
    inner += 1;
    if(strncmp (name, text, ::strlen(text)) == 0)
    {
      return(strdup(name));
    }
  }

  /*Reset to default either for next command or command argument(s).*/
  Cli::m_argOffset = 0;

  /* If no names matched, then return nullptr. */
  return(nullptr);
}

/* Generator function for command completion.  STATE lets us know whether
 * to start from scratch; without any state (i.e. STATE == 0), then we
 * start at the top of the list.
 */
char *commandGenerator(const char *text, int state)
{
  const char *name;
  /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
  if (!state)
  {
      Cli::m_offset = 0;
      Cli::m_len = strlen(text);
  }

  int &idx = Cli::m_offset;
  /* Return the next name which partially matches from the command list. */
  while(nullptr != (name = Cli::m_command[idx].cmd))
  {
    idx += 1;
    if(strncmp (name, text, Cli::m_len) == 0)
    {
      return(strdup(name));
    }
  }

  /*Reset to default either for next command or command argument(s).*/
  Cli::m_offset = 0;

  /* If no names matched, then return nullptr. */
  return(nullptr);
}

/* Attempt to complete on the contents of TEXT.  START and END show the
 * region of TEXT that contains the word to complete.  We can use the
 * entire line in case we want to do some simple parsing.  Return the
 * array of matches, or nullptr if there aren't any.
 */
char **commandCompletion(const char *text, int start, int end)
{
  char **matches;
  matches = (char **)nullptr;

  /* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is the name of a file in the current
     directory. */
  if(start == 0)
  {
    rl_attempted_completion_over = 1;
    matches = rl_completion_matches(text, commandGenerator);
  }
  else
  {
    /*user has hit the space bar*/
    if(start == end)
    {
      int idx = 0;
      /*remember it into its context - this is the command whose argument(s) to be listed.*/
      Cli::cmdName(rl_line_buffer);

      /*Return the entire arguments - list must have last element as nullptr.*/
      for(idx = 0; Cli::m_command[idx].cmd; idx++)
      {
        if(!strncmp(rl_line_buffer,
                    Cli::m_command[idx].cmd,
                    strlen(Cli::m_command[idx].cmd)))
        {
          /*remember this offset and will be used while looping through command arguments.*/
            Cli::m_offset = idx;
          rl_attempted_completion_over = 1;
          matches = rl_completion_matches(text, commandArgListGenerator);
          /*break the while loop.*/
          break;
        }
      }
    }
    else
    {
      /*user has entered the initials of argument*/
      rl_attempted_completion_over = 1;
      matches = rl_completion_matches(text, commandArgGenerator);
    }
  }

  return(matches);
}

int Cli::init(void)
{

  rl_attempted_completion_over = 1;
  /* Tell the completer that we want a crack first. */
  rl_attempted_completion_function = commandCompletion;
  return(0);
}

Cli::~Cli()
{
  delete m_prompt;
  /*set to default - nullptr*/
  m_prompt = nullptr;
}

Cli::Cli()
{
  m_prompt         = nullptr;
  m_continueStatus = false;
}

/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a nullptr pointer if NAME isn't a command name. */

bool Cli::isValid(char *cmd)
{
  int i;
  bool result = false;
  do
  {
    for(i = 0; Cli::m_command[i].cmd; i++)
    {
      if(cmd && strcmp(cmd, Cli::m_command[i].cmd) == 0)
      {
        result = true;
        break;
      }
    }
  }while(0);

  return(result);
}

/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
char *Cli::stripwhite (char *string)
{
  char *s, *t;

  for (s = string; whitespace (*s); s++)
    ;

  if (*s == 0)
    return (s);

  t = s + strlen (s) - 1;
  while (t > s && whitespace (*t))
    t--;
  *++t = '\0';

  return s;
}

int Cli::executeLine(char *req)
{
  int i;
  char *cmd = nullptr;
  char *cmdArg = nullptr;
  bool isFound = false;
  char *line = strdup(req);
  int status = 1;

  /* Isolate the command word. */
  i = 0;
  while(line[i] && whitespace(line[i]))
    i++;
  cmd = line + i;

  while(line[i] && !whitespace(line[i]))
    i++;

  if(line[i])
    cmd[i++] = '\0';

  cmdArg = line + i;
  (void)cmdArg;

  isFound = isValid(cmd);

  do
  {
    if(!isFound)
    {
      fprintf(stderr, "%s: No such command.\n", cmd);
      help(cmd);
      /*bypass the following statement.*/
      break;
    }

    if(!strncmp(cmd, "help", 4) || (!strncmp(cmd, "?", 1)))
    {
      help(cmd);
    }
    else if(!strncmp(cmd, "quit", 4))
    {
      quit();
    }
    else
    {
      /* Call the function. */
      status = 0;
    }

  }while(0);

  free(line);
  return(status);
}

/* Print out help for cmd, or for all of the commands if cmd is
 * not present.
 */
void Cli::help(char *cmd)
{
  int i;
  int printed = 0;

  for (i = 0; Cli::m_command[i].cmd; i++)
  {
    if(!*cmd || (!strcmp(cmd, Cli::m_command[i].cmd)))
    {
      printf("%s\t\t%s.\n",
             Cli::m_command[i].cmd,
             Cli::m_command[i].usage);
      printed++;
    }
  }

  if(!printed)
  {
    fprintf(stderr, "No commands match `%s'.  Possibilties are:\n", cmd);

    for (i = 0; Cli::m_command[i].cmd; i++)
    {
      /* Print in six columns. */
      if (printed == 10)
      {
        printed = 0;
        printf ("\n");
      }

      printf ("%s\t", Cli::m_command[i].cmd);
      printed++;
    }

    if(printed)
      printf ("\n");
  }

}

void Cli::quit(void)
{
  continueStatus(true);
}

void Cli::continueStatus(bool status)
{
  m_continueStatus = status;
}

bool Cli::continueStatus(void)
{
  return(m_continueStatus);
}

int Cli::processCommand(char *cmd, int len)
{
  ACE_Message_Block *mb = nullptr;

  ACE_NEW_RETURN(mb, ACE_Message_Block(1024), -1);

  fprintf(stderr, "The Command is %s\n", cmd);

  ACE_OS::memcpy(mb->wr_ptr(), cmd, len);
  mb->wr_ptr(len);

  tclientTask().put(mb);
  mb->release();
  return(0);
}

int Cli::processResponse(char *rsp, int len)
{
  return(0);
}

int Cli::main(void)
{
    char *line = nullptr;
    char *s = nullptr;

    /* Loop reading and executing lines until the user quits. */
    for(;;)
    {
        line = readline(prompt());

        if(!line)
            break;

        /* Remove leading and trailing whitespace from the line.
           Then, if there is anything left, add it to the history list
           and execute it. */
        s = stripwhite(line);

        if(*s)
        {
            add_history(s);
            if(!executeLine(s))
            {
                ACE_Message_Block *mb = nullptr;
                ACE_NEW_RETURN(mb, ACE_Message_Block(1024), -1);
                ACE_OS::memcpy(mb->wr_ptr(), s, ACE_OS::strlen(s));
                mb->wr_ptr(ACE_OS::strlen(s));

                /*Send this command to backend to process it.*/
                ACE_Time_Value to(2);
                if(-1 == tclientTask().putq(mb, &to))
                {
                    ACE_ERROR((LM_ERROR, "Send to Hostapd Failed\n"));
                }
            }
        }

        free (line);
    }
    return(0);
}

TclientTask &Cli::tclientTask(void)
{
    return(*m_tclientTask);
}

void Cli::tclientTask(TclientTask *tTask)
{
    m_tclientTask = tTask;
}

#endif /*__CLI_CC__*/
