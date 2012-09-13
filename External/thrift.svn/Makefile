# $Id$

.PHONY: install build install env patches repatch 

all: build

build: trunk/build/Makefile
	cd trunk/build && make

OPTIONS != grep -v '\#' .build-options
trunk/build/Makefile: trunk/configure .build-options
	test -d trunk/build || mkdir trunk/build
	cd trunk/build && ../configure ${OPTIONS}

install: trunk/build/Makefile
	cd trunk/build && make install

configure trunk/configure: trunk/bootstrap.sh
	cd trunk && ./bootstrap.sh

THRIFT.DIR ?= /usr/local/include/thrift
SVN.DIR = trunk/lib/cpp/src/thrift
PATCHED.SRC != awk '/^Index/ {print $$2}' patch/*/*.diff | dos2unix 
PATCHED.TGT = $(PATCHED.SRC:S!$(SVN.DIR)!$(THRIFT.DIR)!g)

repatch:
	svn revert $(PATCHED.SRC)
	sh -x apply-patches

# Generate a makefile to update the working directory 
# in case updates were made to the installed files. 
Makefile.update::
	./update-from-installed $(SVN.DIR)  $(THRIFT.DIR) \
	> $@~
	@mv $@~ $@

# N.B. Update this by hand as needed.
# See also dependencies below. 
PATCHES = 	patch/transport/TBufferTransports.h.diff \
		patch/transport/TSocket.h.diff 
PATCHES.BASE = $(PATCHES:S!patch/!!g:.diff=)


patches: $(PATCHES)
$(PATCHES):
	svn diff $(@:S!^patch!$(SVN.DIR)!g:S!.diff!!g) > $@~ 
	@mv $@~ $@

AWK = awk '/---/ {print $$2}' patch/$@.diff

compare-patched: $(PATCHES.BASE)
$(PATCHES.BASE): 
	diff -u $(AWK:sh) $(AWK:sh:S!$(SVN.DIR)!$(THRIFT.DIR)!) || \
	diff    $(AWK:sh) $(AWK:sh:S!$(SVN.DIR)!$(THRIFT.DIR)!) \
		| cmp patch/$@.diff /dev/stdin

install-patched: $(PATCHED.TGT)
$(THRIFT.DIR)/protocol/TBinaryProtocol.tcc: \
	$(SVN.DIR)/protocol/TBinaryProtocol.tcc
	install $(.ALLSRC) $@
$(THRIFT.DIR)/transport/TBufferTransports.h: \
	$(SVN.DIR)/transport/TBufferTransports.h
	install $(.ALLSRC) $@
$(THRIFT.DIR)/transport/TSocket.h: \
	$(SVN.DIR)/transport/TSocket.h
	install $(.ALLSRC) $@



env: 
	apt-get install flex bison libssl-dev libssl-doc dos2unix

patch/transport/TBufferTransports.h.diff: \
	$(SVN.DIR)/transport/TBufferTransports.h
patch/transport/TSocket.h.diff: \
	$(SVN.DIR)/transport/TSocket.h
