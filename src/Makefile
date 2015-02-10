CC=gcc
CFLAGS=-g -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include
LDFLAGS=-lusb -lglib-2.0 -ldbus-glib-1
SOURCES=rtx.c phoneusb.c common.c rtx_dbus.c 
HEADERS=rtx.h phoneusb.h common.h rtx_dbus.h dbus-stub.h 
OBJS=$(SOURCES:.c=.o)
EXECUTABLE=rtxdriver

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f *.o rtxdriver

