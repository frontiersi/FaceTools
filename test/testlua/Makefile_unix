INCDIR = -I /home/rich/ll/lua5/include
CPP    = g++
CFLAGS = $(INCDIR) -std=c++14
LINK   = -L/home/rich/ll/lua5/lib -llua53 \
		 -ldl \
		 -Wl,-rpath=/home/rich/ll/lua5/lib

.cpp.o:
	$(CPP) $(CFLAGS) -c $< -o $@

all: test

test: main.cpp
	$(CPP) $(CFLAGS) -o test main.cpp $(LINK)
