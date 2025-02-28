.PHONY: all check clean dist distclean doc install install-headers regressions uninstall

BUILD_DIR=/Volumes/dev/StitchMarker/ck
SRC_DIR=/Volumes/dev/StitchMarker/ck
CFLAGS=-D_XOPEN_SOURCE=600 -D_BSD_SOURCE -D_DEFAULT_SOURCE -std=gnu99 -pedantic -Wall -W -Wundef -Wendif-labels -Wshadow -Wpointer-arith -Wcast-align -Wcast-qual -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Winline -Wdisabled-optimization -fstrict-aliasing -O2 -pipe -Wno-parentheses  -fPIC
VERSION=1.0.0
VERSION_MAJOR=0
PREFIX=/usr/local
LIBRARY=/usr/local/lib
HEADERS=/usr/local/include
PKGCONFIG_DATA=/usr/local/lib/pkgconfig
LDNAME=libck.dylib
LDNAME_VERSION=libck.1.0.0.dylib
LDNAME_MAJOR=libck.0.dylib

all: doc
	$(MAKE) -C src all || exit
	@echo
	@echo
	@echo ---[ Concurrency Kit has built successfully. You may now \"make install\".

doc:
	$(MAKE) -C doc all || exit
	@echo ---[ Manual pages are ready for installation.

regressions:
	$(MAKE) -C regressions all || exit
	@echo ---[ Regressions have built successfully.

check: regressions
	@echo ---[ Executing unit tests...
	$(MAKE) -C regressions check
	@echo ---[ Unit tests have completed successfully.

install-headers:
	mkdir -p $(DESTDIR)/$(HEADERS) || exit
	cp $(SRC_DIR)/include/*.h $(DESTDIR)/$(HEADERS) || exit
	chmod 644 $(DESTDIR)/$(HEADERS)/ck_*.h || exit
	mkdir -p $(DESTDIR)$(HEADERS)/gcc || exit
	cp -r $(SRC_DIR)/include/gcc/* $(DESTDIR)/$(HEADERS)/gcc || exit
	cp include/ck_md.h $(DESTDIR)/$(HEADERS)/ck_md.h || exit
	chmod 755 $(DESTDIR)/$(HEADERS)/gcc
	chmod 644 $(DESTDIR)/$(HEADERS)/gcc/ck_*.h $(DESTDIR)/$(HEADERS)/gcc/*/ck_*.h || exit
	mkdir -p $(DESTDIR)$(HEADERS)/spinlock || exit
	cp -r $(SRC_DIR)/include/spinlock/* $(DESTDIR)/$(HEADERS)/spinlock || exit
	chmod 755 $(DESTDIR)/$(HEADERS)/spinlock
	chmod 644 $(DESTDIR)/$(HEADERS)/spinlock/*.h || exit

install-so:
	mkdir -p $(DESTDIR)/$(LIBRARY)
	cp src/libck.so $(DESTDIR)/$(LIBRARY)/$(LDNAME_VERSION)
	ln -sf $(LDNAME_VERSION) $(DESTDIR)/$(LIBRARY)/$(LDNAME)
	ln -sf $(LDNAME_VERSION) $(DESTDIR)/$(LIBRARY)/$(LDNAME_MAJOR)
	chmod 744 $(DESTDIR)/$(LIBRARY)/$(LDNAME_VERSION)	\
		  $(DESTDIR)/$(LIBRARY)/$(LDNAME)		\
		  $(DESTDIR)/$(LIBRARY)/$(LDNAME_MAJOR)

install-lib:
	mkdir -p $(DESTDIR)/$(LIBRARY)
	cp src/libck.a $(DESTDIR)/$(LIBRARY)/libck.a
	chmod 644 $(DESTDIR)/$(LIBRARY)/libck.a

install: all install-headers install-so install-lib
	$(MAKE) -C doc install
	mkdir -p $(DESTDIR)/$(LIBRARY) || exit
	mkdir -p $(DESTDIR)/$(PKGCONFIG_DATA) || exit
	chmod 755 $(DESTDIR)/$(PKGCONFIG_DATA)
	cp build/ck.pc $(DESTDIR)/$(PKGCONFIG_DATA)/ck.pc || exit
	@echo
	@echo
	@echo ---[ Concurrency Kit has installed successfully.

uninstall:
	$(MAKE) -C doc uninstall
	rm -f $(DESTDIR)/$(LIBRARY)/$(LDNAME_VERSION)	\
	      $(DESTDIR)/$(LIBRARY)/$(LDNAME)		\
	      $(DESTDIR)/$(LIBRARY)/$(LDNAME_MAJOR)
	rm -f $(DESTDIR)/$(LIBRARY)/libck.so*
	rm -f $(DESTDIR)/$(LIBRARY)/libck.a
	rm -f $(DESTDIR)/$(HEADERS)/ck_*.h
	rm -f $(DESTDIR)/$(HEADERS)/spinlock/*.h
	rm -f $(DESTDIR)/$(HEADERS)/gcc/ck_*.h
	rm -f $(DESTDIR)/$(HEADERS)/gcc/*/ck_*.h
	rm -f $(DESTDIR)/$(PKGCONFIG_DATA)/ck.pc

clean:
	$(MAKE) -C doc clean
	$(MAKE) -C src clean
	$(MAKE) -C regressions clean
	rm -f $(BUILD_DIR)/*~ $(BUILD_DIR)/*.o $(BUILD_DIR)/*.tar.gz

dist:
	git archive --remote=$(SRC_DIR) --format=tar --prefix=ck-$(VERSION)/ HEAD \
		| gzip > $(BUILD_DIR)/ck-$(VERSION).tar.gz

distclean: clean
	rm -f $(BUILD_DIR)/include/ck_md.h
	rm -f $(BUILD_DIR)/build/regressions.build
	rm -f $(BUILD_DIR)/build/ck.build
	rm -f $(BUILD_DIR)/build/ck.pc
	rm -f $(BUILD_DIR)/Makefile
	rm -f $(BUILD_DIR)/doc/Makefile

