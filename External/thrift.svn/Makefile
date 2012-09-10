# $Id$

.PHONY: install build install env patches repatch 

all: build

build: trunk/build/Makefile
	cd trunk/build && make

trunk/build/Makefile: .patched trunk/configure
	test -d trunk/build || mkdir trunk/build
	OPTIONS=$$(grep -v '#' .build-options) \
	&& cd trunk/build && ../configure $${OPTIONS}

install: trunk/build/Makefile
	cd trunk/build && make install

configure trunk/configure: trunk/bootstrap.sh
	cd trunk && ./bootstrap.sh

THRIFT.DIR ?= /usr/local/include/thrift
SVN.DIR = trunk/lib/cpp/src/thrift
PATCHED.SRC != awk '/^Index/ {print $$2}' patch/*/*.diff
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
PATCHES = 	patch/protocol/TBinaryProtocol.tcc.diff \
		patch/transport/TBufferTransports.h.diff \
		patch/transport/TSocket.h.diff 

patches: $(PATCHES)
$(PATCHES):
	svn diff $(@:S!^patch!$(SVN.DIR)!g:S!.diff!!g) > $@~ 
	@mv $@~ $@

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
	apt-get install flex bison libssl-dev libssl-doc

#patches
##patch/TBinaryProtocol.h.diff: $(SVN.DIR)/TBinaryProtocol.h
patch/protocol/TBinaryProtocol.tcc.diff: \
	$(SVN.DIR)/protocol/TBinaryProtocol.tcc
patch/transport/TBufferTransports.h.diff: \
	$(SVN.DIR)/transport/TBufferTransports.h
patch/transport/TSocket.h.diff: \
	$(SVN.DIR)/transport/TSocket.h
