include conf.make

DESTDIR=
CCOPTS=-Wall -Wextra
TARGET=starlanes

SRCS=$(wildcard *.c)
HEADERS=$(wildcard *.h)

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

.PHONY: all clean pristine

BINDIR=$(DESTDIR)$(prefix)/bin
MANDIR=$(DESTDIR)$(mandir)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CCOPTS) -o $@ $(OBJS) -l$(CURSES_LIBRARY)

%.o: %.c
	$(CC) $(CCOPTS) -MMD -MF $(@:.o=.d) -c $< -o $@

install:
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/$(TARGET)
	install -d $(MANDIR)/man6
	install -m 644 $(TARGET).6 $(MANDIR)/man6/$(TARGET).6

clean:
	rm -f *.o *.d

pristine: clean
	rm -f conf.make conf.h
	rm -f $(TARGET)

-include $(DEPS)

