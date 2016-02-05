AUTOMAKE_OPTIONS = foreign

SUBDIRS = debian.upstream src

# Extra clean files so that maintainer-clean removes *everything*
MAINTAINERCLEANFILES = \
	aclocal.m4 compile config.guess config.sub \
	configure depcomp install-sh ltmain.sh     \
	Makefile.in missing

DEB_BUILDDIR = debian.build

deb:
	@[ -d debian ] || ln -s debian.upstream debian
	dpkg-buildpackage -rfakeroot -uc -us

deb.upstream: dist
	-mkdir -p $(DEB_BUILDDIR)
	cd $(DEB_BUILDDIR)				&& \
	rm -rf $(PACKAGE)-$(VERSION)			&& \
	tar zxvf ../$(PACKAGE)-$(VERSION).tar.gz	&& \
	cd $(PACKAGE)-$(VERSION)			&& \
	$(MAKE) deb -f Makefile.am

EXTRA_DIST = Android.mk
