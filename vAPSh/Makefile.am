#/*!Name of the binary*/

bin_PROGRAMS = vAPG

vAPG_SOURCES = src/vap.cc \
               inc/vap.h

vAPG_CPPFLAGS=-I$(srcdir)/inc \
              -I$(top_srcdir)/common/inc \
              -I$(top_srcdir)/ipc/inc \
              -I/usr/local \
              -I/usr/local/ACE-6.5.0/include

vAPG_CXXFLAGS=-g -Wall -std=gnu++11

vAPG_LDFLAGS=-L/usr/local/ACE-6.5.0/lib \
             -L$(CURDIR)/../common \
             -L$(CURDIR)/../ipc

vAPG_LDADD=-lACE -lreadline -lpthread -ltinfo -lipc -lcommon
