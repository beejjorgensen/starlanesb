include conf.make

DESTDIR=
CCOPTS=-Wall -Wextra
TARGET=starlanes

SRCS=$(wildcard *.c)
HEADERS=$(wildcard *.h)

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
DEPSDIR = deps

.PHONY: all clean pristine

BINDIR=$(DESTDIR)$(prefix)/bin
MANDIR=$(DESTDIR)$(mandir)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CCOPTS) -o $@ $(OBJS) -l$(CURSES_LIBRARY)

%.o: %.c
	mkdir -p $(DEPSDIR)
	$(CC) $(CCOPTS) -MMD -MF $(DEPSDIR)/$(@:.o=.d) -c $< -o $@

install:
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/$(TARGET)
	install -d $(MANDIR)/man6
	install -m 644 $(TARGET).6 $(MANDIR)/man6/$(TARGET).6

clean:
	rm -f *.o $(DEPSDIR)/*.d

pristine: clean
	rm -f conf.make conf.h
	rm -f $(TARGET)
	rmdir $(DEPSDIR) || true

-include $(DEPSDIR)/$(DEPS)

