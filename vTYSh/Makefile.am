
bin_PROGRAMS=vTYSh

vTYSh_SOURCES=Src/ReadlineIF.cc \
              Src/Vtysh.cc \
              Src/Main.cc

vTYSh_CPPFLAGS=-I$(srcdir)/Inc \
               -I/usr/local \
               -I/usr/local/ACE-6.5.0/include

vTYSh_CXXFLAGS=-g -Wall -std=gnu++11

vTYSh_LDFLAGS=-L/usr/local/ACE-6.5.0/lib

vTYSh_LDADD=-lreadline -lACE -ltinfo -lpthread
