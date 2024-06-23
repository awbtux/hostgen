# build root dir
buildroot = .

# statically link binaries?
static_bin = no

# strip binaries of their debug information?
strip_bin = yes

# root dir
sysroot =

# architecture-independent and architecture-dependent install prefixes
prefix = /usr/local
exec_prefix = $(prefix)

# install dirs
bindir = $(sysroot)$(exec_prefix)/bin
libdir = $(sysroot)$(prefix)/lib
includedir = $(sysroot)$(prefix)/include
sharedir = $(sysroot)$(prefix)/share
etcdir = $(sysroot)/etc
mandir = $(sharedir)/man

# cross-compiler target
CROSS =

# binaries
CC = $(CROSS)gcc
AR = $(CROSS)ar
AS = $(CROSS)as
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump
STRINGS = $(CROSS)strings
STRIP = $(CROSS)strip
RANLIB = $(CROSS)ranlib
NM = $(CROSS)nm
READELF = $(CROSS)readelf

# gnu options
CFLAGS = -pipe -ffunction-sections -fdata-sections -fmerge-all-constants -Wall -Wno-unused-variable
CPPFLAGS = -D_DEFAULT_SOURCE
CXXFLAGS = -pipe -ffunction-sections -fdata-sections -fmerge-all-constants -Wall -Wno-unused-variable
LDFLAGS = -Wl,--gc-sections,-s,-z,norelro,-z,now,--hash-style=sysv,--sort-section,alignment
STRIPFLAGS = -S --strip-unneeded -R .note.gnu.gold-version -R .comment -R .note -R .note.gnu.build-id -R .jcr -R .note.gnu.property -R .note.ABI-tag -R .gnu.version -R .gcc_except_table

# enable static linking, if defined
ifeq ($(static_bin),yes)
LDFLAGS += -static -Wl,-a,archive
else
LDFLAGS += -Wl,-a,shared
endif

# strip binaries, if defined
ifeq ($(strip_bin),yes)
LDFLAGS += -s -Wl,--build-id=none
CFLAGS += -s -Os -g0
CXXFLAGS += -s -Os -g0
stripcmd = $(STRIP) $@ $(STRIPFLAGS)
else
LDFLAGS += -g
CFLAGS += -g -Og
CXXFLAGS += -g -Og
endif

# stuff
SRC = main.c
OBJ = $(SRC:.c=.o)
BIN = hostgen
MAN = $(BIN).1

all: hostgen

clean:
	rm -rf *.o hostgen hostgen-*.tar.*

install-hostgen: all
	mkdir -p -m 755 $(bindir)
	cp hostgen $(bindir)

uninstall-hostgen:
	rm $(bindir)/hostgen

install-doc: $(MAN)
	mkdir -p -m 755 $(mandir)/man1
	cp $(MAN) $(mandir)/man1

uninstall-doc:
	rm $(mandir)/man1/$(MAN)

dist: clean
	-tar -cC ../ ./$(shell basename $$PWD) -Jf hostgen-$(version).tar.xz

install: install-hostgen install-doc
uninstall: uninstall-hostgen uninstall-doc

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

hostgen: $(SRC) $(OBJ)
	$(CC) $(LIBS) $(LDFLAGS) $(OBJ) -o $@
	$(stripcmd)

.PHONY: all hostgen install-hostgen uninstall-hostgen install-doc uninstall-doc install uninstall dist clean
