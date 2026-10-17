#
# MaBoSS project Makefile wrapper
#
# Institut Curie, France
#
# Eric Viara, Gautier Stoll March 2011
#

# To compile:
#  - standard maboss and popmaboss: make [flags]
#  - MaBoSS_dn: make USE_DYNAMIC_BITSET_STD_ALLOC=1 [flags]
#  - MaBoSS_dn_exp: make USE_DYNAMIC_BITSET=1 [flags]
#  - MaBoSS_<maxnodes>n: make MAXNODES=<maxnodes>, for instance: make MAXNODES=128 will compile MaBoSS_128n

MAKEFILE = Makefile.maboss

.PHONY: maboss popmaboss

maboss popmaboss:
	@rm -f depend.mk
	@touch depend.mk
	@$(MAKE) -f $(MAKEFILE) init
	@$(MAKE) -f $(MAKEFILE) depend
	$(MAKE) -f $(MAKEFILE) $@

tar_perform:
	$(MAKE) -f $(MAKEFILE) $@

clean:
	@rm -f depend.mk
	@touch depend.mk
	$(MAKE) -f $(MAKEFILE) $@
	$(MAKE) -f $(MAKEFILE) USE_DYNAMIC_BITSET=1 $@
	rm -f *.o MaBoSS MaBoSS_dn MaBoSS_[0-9]*n MaBoSS_[0-9]*n-server MaBoSS_[0-9]*n-client
	rm -f *.o PopMaBoSS PopMaBoSS_dn PopMaBoSS_[0-9]*n
	rm -f libMaBoSS.so libMaBoSS_[0-9]*n.so libPopMaBoSS.so libPopMaBoSS_[0-9]*n.so
	rm -f libMaBoSS-static.a libMaBoSS_[0-9]*n-static.a libPopMaBoSS-static.a libPopMaBoSS_[0-9]*n-static.a

%:
	@rm -f depend.mk
	@touch depend.mk
	@$(MAKE) -f $(MAKEFILE) init
	@$(MAKE) -f $(MAKEFILE) depend
	$(MAKE) -f $(MAKEFILE) $@
