# $Id$

.PHONY: install build install env patch

MODIFIED != svn status trunk | awk '/^M/ {print $$2}'
PATCH = $(MODIFIED:T:S/^/patch\//g:S/$/.diff/)

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
PATCHED.SRC != awk '/^Index/ {print $$2}' patch/*diff
PATCHED.TGT = $(PATCHED.SRC:S!$(SVN.DIR)!$(THRIFT.DIR)!g)

repatch:
	svn revert $(PATCHED.SRC)
	sh -x apply-patches

apply-edits: $(PATCH)
$(PATCH)::
	./apply-edits $(:!find trunk $(THRIFT.DIR) -name $(@:T:R)!)
	svn diff $(:!find trunk -name $(@:T:R)!) > $@~ 
	@mv $@~ $@

install-patched: $(PATCHED.TGT)
$(THRIFT.DIR)/protocol/TBinaryProtocol.tcc: \
	$(SVN.DIR)/TBinaryProtocol.tcc
	install $(.ALLSRC) $@
$(THRIFT.DIR)/transport/TBufferTransports.h: \
	$(SVN.DIR)/transport/TBufferTransports.h
	install $(.ALLSRC) $@
$(THRIFT.DIR)/transport/TSocket.h: \
	$(SVN.DIR)/transport/TSocket.h
	install $(.ALLSRC) $@

env: 
	apt-get install flex bison libssl-dev libssl-doc