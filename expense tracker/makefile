CC = gcc
CFLAGS = -Wall -Wextra
UNAME := $(shell uname)

ifeq ($(UNAME),Darwin)
SHARED = -dynamiclib -install_name @rpath/libexpense.so
LINK = ./libexpense.so
RPATH = -Wl,-rpath,@executable_path
else
SHARED = -shared
LINK = -L. -lexpense
RPATH = -Wl,-rpath,'$$ORIGIN'
endif

all: tracker

libexpense.so: expense.c expense.h
	$(CC) $(CFLAGS) -fPIC $(SHARED) -o libexpense.so expense.c

tracker: main.c libexpense.so
	$(CC) $(CFLAGS) -o tracker main.c $(LINK) $(RPATH)

clean:
	rm -f libexpense.so tracker report.txt