#/*!Name of the binary*/

bin_PROGRAMS=vAPSh

vAPSh_SOURCES=src/vap.cc

vAPSh_CPPFLAGS=-I$(srcdir)/Inc \
               -I$(top_srcdir)/Common/Inc \
               -I$(top_srcdir)/Ipc/Inc \
               -I/usr/local \
               -I/usr/local/ACE-6.5.0/include

vAPSh_CXXFLAGS=-g -Wall -std=gnu++11

vAPSh_LDFLAGS=-L/usr/local/ACE-6.5.0/lib \
              -L$(CURDIR)/../Common \
              -L$(CURDIR)/../Ipc

vAPSh_LDADD=-lACE -lreadline -lpthread -ltinfo -lipc -lcommon
