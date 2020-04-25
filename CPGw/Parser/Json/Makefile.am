bin_PROGRAMS=PARSER

AM_YFLAGS=-d
AM_LFLAGS=-d

PARSER_SOURCES=Src/Json.cc \
							 Src/JsonParser.yy \
							 Src/JsonLexer.ll \
							 Src/JsonMain.cc

PARSER_CPPFLAGS=-I$(srcdir)/Inc \
								-I$(CURDIR)/Src \
								-I$(CURDIR) \
								-I/usr/local \
								-I/usr/local/ACE-6.5.0/include


PARSER_LDFLAGS=-L/usr/local/ACE-6.5.0/lib

PARSER_CXXFLAGS=-g -Wall -std=gnu++11

PARSER_LDADD=-lACE -lreadline -lpthread -ltinfo
