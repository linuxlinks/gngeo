EXTRA_DIST = romrc.d.tgz

install-data-local:
	tar -xzf $(srcdir)/romrc.d.tgz
	mkdir -p $(DESTDIR)$(pkgdatadir)/romrc.d
	$(INSTALL_DATA) $(srcdir)/*.rc $(DESTDIR)$(pkgdatadir)/romrc.d

