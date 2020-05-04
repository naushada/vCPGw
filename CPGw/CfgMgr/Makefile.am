
bin_PROGRAMS=CFGMGR

CFGMGR_SOURCES=Src/CfgMgr.cc \
							 Src/CfgMgrMain.cc \
							 Src/CfgCmdHandler.cc

CFGMGR_CPPFLAGS=-I$(srcdir)/Inc \
								-I$(srcdir)/../Ipc/Inc \
								-I$(srcdir)/../Common/Inc \
								-I$(srcdir)/../Parser/Json/Inc \
								-I/usr/local \
								-I/usr/local/ACE-6.5.0/include


CFGMGR_LDFLAGS=-L/usr/local/ACE-6.5.0/lib \
							 -L$(CURDIR)/../Ipc \
							 -L$(CURDIR)/../Common \
							 -L$(CURDIR)/../Parser/Json

CFGMGR_CXXFLAGS=-g -Wall -std=gnu++11

CFGMGR_LDADD=-lACE -lreadline -lpthread -ltinfo -lJson -lIpc -lCommon
