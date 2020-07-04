#ifndef __CLIMAIN_CC__
#define __CLIMAIN_CC__

#include "cli.h"
#include "tclient.h"






int main(int argc, char *argv[])
{
    TclientTask *tTask = nullptr;
    Cli *cli = nullptr;
    ACE_NEW_NORETURN(cli, Cli());

    ACE_NEW_NORETURN(tTask, TclientTask(ACE_Thread_Manager::instance(),
                                        PORT,
                                        cli));

    /*Active Object. tTask - meaning TCP Task (Active Object).*/
    tTask->open();
    cli->tclientTask(tTask);
    cli->init();
    cli->prompt("(Peace)->");
    cli->main();
}









#endif /*__CLIMAIN_CC__*/
