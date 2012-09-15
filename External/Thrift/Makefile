# $Id$

.PHONY: install build install env patches repatch 

all: build

build: build/Makefile
	cd build && make

OPTIONS != grep -v '\#' .build-options
build/Makefile: configure .build-options
	test -d build || mkdir build
	cd build && ../configure $(OPTIONS)

install: build/Makefile
	cd build && make install

configure: bootstrap.sh
	./bootstrap.sh

THRIFT.DIR ?= /usr/local/include/thrift
SVN.DIR = lib/cpp/src/thrift
PATCHED.SRC != awk '/^Index/ {print $$2}' \
			patch/*/*.diff patch/*.diff | dos2unix 
PATCHED.TGT = $(PATCHED.SRC:S!$(SVN.DIR)!$(THRIFT.DIR)!g)

repatch:
	svn revert $(PATCHED.SRC)
	./apply-patches

env: 
	apt-get install flex bison libssl-dev libssl-doc dos2unix

