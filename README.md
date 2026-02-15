# Star Lanes

This is a C version of the classic space trading game [_Star
Lanes_](https://github.com/beejjorgensen/starlanes-info) that I wrote
back in the mid 1990s based on the Osborne FOG disk source.

I was in college and my skills were poor, so the source is just one
gigantic file. Maybe I'll break that up since it's so offensive.

## Building and Installation

Type "make" to build the `starlanes` binary.

You can override the following macros:

* `CURSESLIB=ncurses`: this is added to the command line as `-lncurses`.
* `CC`: which compiler to use.
* `CCOPTS=-Wall -Wextra`: compiler options.
* `prefix=/usr`: where things are installed relative to root.
* `DESTDIR`: if you want to set another "root" for the install.

## Version Info

David Barnsdale wrote an AI for this thing and released it as
[1.3.0](https://web.archive.org/web/20190701060458/https://www.barnsdle.demon.co.uk/game/starlanes.html).
But I'm having trouble getting that to run without crashing. So I've
forged ahead with modernizing the working one, and we'll see about
backporting the AI into it later.

This one is based on 1.2.2.

---

beej@beej.us

