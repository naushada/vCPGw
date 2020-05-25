
bin_PROGRAMS=SYSMGR

SYSMGR_SOURCES=Src/SysMgr.cc \
							 Src/SysMgrMain.cc

SYSMGR_CPPFLAGS=-I$(srcdir)/Inc \
								-I$(srcdir)/../Ipc/Inc \
								-I$(srcdir)/../Common/Inc \
								-I$(srcdir)/../ProcMgr/Inc \
								-I$(srcdir)/../Parser/Json/Inc \
								-I/usr/local \
								-I/usr/local/ACE-6.5.0/include


SYSMGR_LDFLAGS=-L/usr/local/ACE-6.5.0/lib \
							 -L$(CURDIR)/../Ipc \
							 -L$(CURDIR)/../Common \
							 -L$(CURDIR)/../Parser/Json

SYSMGR_CXXFLAGS=-g -Wall -std=gnu++11

SYSMGR_LDADD=-lACE -lreadline -lpthread -ltinfo -lJson -lIpc -lCommon
