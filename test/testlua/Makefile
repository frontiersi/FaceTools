CPP    = cl
LOCAL_LIBS = C:\Users\riche\local_libs
INCDIR = /I $(LOCAL_LIBS)\lua5\include
LINK   = $(LOCAL_LIBS)\lua5\lib\lua53.lib
CFLAGS = $(INCDIR) /std:c++14
TARGET = test.exe
SOURCES = main.cpp

.cpp.obj:
	$(CPP) $(CFLAGS) /c $< /out:$@

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CPP) $(CFLAGS) /out:$@ $** $(LINK)
