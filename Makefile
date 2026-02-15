include conf.make

DESTDIR=
CCOPTS=-Wall -Wextra
TARGET=starlanes

.PHONY: all clean pristine

BINDIR=$(DESTDIR)$(prefix)/bin
MANDIR=$(DESTDIR)$(mandir)

all: $(TARGET)

$(TARGET): starlanes.c
	$(CC) $(CCOPTS) -o $@ $^ -l$(CURSES_LIBRARY)

install:
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/$(TARGET)
	install -d $(MANDIR)/man6
	install -m 644 $(TARGET).6 $(MANDIR)/man6/$(TARGET).6

clean:
	rm -f *.o

pristine: clean
	rm -f conf.make conf.h
	rm -f $(TARGET)
