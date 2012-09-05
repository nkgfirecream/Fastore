# $Id$

.PHONY: install build install env

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

.patched:
	sh -x apply-patch

env: 
	apt-get install flex bison libssl-dev libssl-doc