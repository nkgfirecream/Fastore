# $Id$

BOOST.DIR = boost_1_51_0

boost-install: $(BOOST.DIR)/project-config.jam
	cd $(BOOST.DIR) && ./b2 install

boost-build: $(BOOST.DIR)/project-config.jam
$(BOOST.DIR)/project-config.jam: distfiles/$(BOOST.DIR).tar.gz
	cd $(@:H) && ./bootstrap.sh

boost-extract: $(BOOST.DIR)/.extracted
$(BOOST.DIR)/.extracted: distfiles/$(BOOST.DIR).tar.gz
	pax -rzf $(.ALLSRC)
	touch $@

distfiles/$(BOOST.DIR).tar.gz:
	test -d distfiles || mkdir distfiles
	wget -P $(@:H) http://sourceforge.net/projects/boost/files/boost/1.51.0/$(BOOST.DIR).tar.gz/download
	mv $(@:H)/download $@