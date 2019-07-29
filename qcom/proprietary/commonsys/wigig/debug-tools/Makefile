.DEFAULT_GOAL = all

.PHONY = all install uninstall clean

all: make_host_manager_11ad make_remoteserver make_logcollector make_wiburn make_wlctutils make_wlctpciacss make_flashacss

make_host_manager_11ad:
	$(MAKE) -C host_manager_11ad CROSS=$(CROSS)

make_remoteserver: make_wlctutils make_wlctpciacss
	$(MAKE) -C remoteserver CROSS=$(CROSS)

make_logcollector: make_wlctutils make_wlctpciacss
	$(MAKE) -C LogCollector CROSS=$(CROSS)

make_wiburn: make_wlctutils make_wlctpciacss make_flashacss
	$(MAKE) -C wiburn CROSS=$(CROSS)

make_wlctutils:
	$(MAKE) -C lib/utils CROSS=$(CROSS)

make_wlctpciacss:
	$(MAKE) -C lib/WlctPciAcss CROSS=$(CROSS)

make_flashacss:
	$(MAKE) -C lib/FlashAcss CROSS=$(CROSS)

clean:
	echo "Cleaning up wigig debug tools"
	$(MAKE) -C host_manager_11ad clean
	$(MAKE) -C remoteserver clean
	$(MAKE) -C LogCollector clean
	$(MAKE) -C wiburn clean
	$(MAKE) -C lib/utils clean
	$(MAKE) -C lib/WlctPciAcss clean
	$(MAKE) -C lib/FlashAcss clean

install: all

	@echo "Installing wigig debug tools libraries to /usr/sbin"
	install lib/utils/libwigig_utils.so /usr/lib
	install lib/FlashAcss/libwigig_flashaccess.so /usr/lib
	install lib/WlctPciAcss/libwigig_pciaccess.so /usr/lib

	@echo "Installing wigig debug tools to /usr/sbin"
	install host_manager_11ad/host_manager_11ad /usr/sbin
	install remoteserver/wigig_remoteserver /usr/sbin
	install LogCollector/wigig_logcollector /usr/sbin
	install wiburn/wigig_wiburn /usr/sbin
	install -m 0644 LogCollector/linux/wigig_logcollector.ini /etc

uninstall:

	rm -f /usr/sbin/host_manager_11ad
	rm -f /usr/sbin/wigig_*
	rm -f /etc/wigig_*
	rm -f /usr/lib/libwigig_*

