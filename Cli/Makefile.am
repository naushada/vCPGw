
bin_PROGRAMS=Cli

Cli_SOURCES=Src/cli.cc \
            Src/climain.cc \
            Src/tclient.cc

Cli_CPPFLAGS=-I$(srcdir)/Inc \
             -I/usr/local \
             -I/usr/local/ACE-6.5.0/include

Cli_CXXFLAGS=-g -Wall -std=gnu++11

Cli_LDFLAGS=-L/usr/local/ACE-6.5.0/lib

Cli_LDADD=-lreadline -lACE -ltinfo -lpthread
