noinst_LIBRARIES=libJson.a

AM_YFLAGS=-d
AM_LFLAGS=-d

libJson_a_SOURCES=Src/JsonParser.yy \
							    Src/JsonLexer.ll \
							    Src/Json.cc \
							    Src/JsonMain.cc

libJson_a_CPPFLAGS=-I$(srcdir)/Inc \
								   -I$(CURDIR)/Src \
								   -I$(CURDIR) \
								   -I/usr/local \
								   -I/usr/local/ACE-6.5.0/include


#PARSER_LDFLAGS=-L/usr/local/ACE-6.5.0/lib

libJson_a_CXXFLAGS=-g -Wall -std=gnu++11
libJson_a_AR=ar rvs

#PARSER_LDADD=-lACE -lreadline -lpthread -ltinfo
