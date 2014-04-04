include $(MENUETDEV)/osrules.mak

WAIT =
ifdef ON_MINGW
WAIT = -DWEXITSTATUS=
endif

TOOLS = mgcc$(EXESUFFIX) mld$(EXESUFFIX) mgpp$(EXESUFFIX) mmkdep$(EXESUFFIX) \
	mchmem$(EXESUFFIX)

all: $(TOOLS)
	@echo $(MENUETDEV)
	
mgcc$(EXESUFFIX): mgcc.c
	gcc mgcc.c -o mgcc$(EXESUFFIX) -DHAS_DEVENV=$(HAS_DEVENV) -s $(WAIT)

mgpp$(EXESUFFIX): mgpp.c
	gcc mgpp.c -o mgpp$(EXESUFFIX) -DHAS_DEVENV=$(HAS_DEVENV) -s \
	-DTOOLNAME=\"$(GPP_TOOLNAME)\" $(WAIT)

mld$(EXESUFFIX): mld.c
	gcc mld.c -o mld$(EXESUFFIX) -DHAS_DEVENV=$(HAS_DEVENV) -s $(WAIT)

mchmem$(EXESUFFIX): mchmem.c
	gcc mchmem.c -o mchmem$(EXESUFFIX) -DHAS_DEVENV=$(HAS_DEVENV) -s $(WAIT)

mmkdep$(EXESUFFIX): mmkdep.c
	gcc mmkdep.c -o mmkdep$(EXESUFFIX) -DHAS_DEVENV=$(HAS_DEVENV) -s $(WAIT)

clean:
	$(RM) $(TOOLS)
