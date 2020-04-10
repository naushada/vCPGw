
bin_PROGRAMS =vtysh

vtysh_SOURCES=src/readlineIF.cc \
              src/vtysh.cc \
              src/main.cc

vtysh_CPPFLAGS=-I$(srcdir)/inc \
               -I/usr/local \
               -I/usr/local/ACE-6.5.0/include

vtysh_CXXFLAGS=-g -Wall -std=gnu++11

vtysh_LDFLAGS=-L/usr/local/ACE-6.5.0/lib

vtysh_LDADD=-lreadline -lACE -ltinfo -lpthread
