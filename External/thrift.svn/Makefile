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

PATCHED != awk '/^Index/ {print $$2}' patch/*diff
repatch:
	svn revert $(PATCHED)
	sh -x apply-patches

apply-edits: $(PATCH)

THRIFT.DIR ?= /usr/local/include/thrift

$(PATCH)::
	./apply-edits $(:!find trunk $(THRIFT.DIR) -name $(@:T:R)!)
	svn diff $(:!find trunk -name $(@:T:R)!) > $@~ 
	@mv $@~ $@

env: 
	apt-get install flex bison libssl-dev libssl-doc