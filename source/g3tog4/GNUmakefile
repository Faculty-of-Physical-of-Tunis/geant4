###############################################################################
###  GNUmakefile for the G3 to G4 conversion package.
###
###  T. Wenaus 20/11/1995
###  Uses chunks of John Allison's generic makefiles.
###
###  The presence of Fortran and several standalone programs
###  make this package non-generic enough to require its
###  own makefile.
###
###############################################################################

G4TARGET := rztog4

name := G3toG4

ifndef G4INSTALL
  G4INSTALL := ../..
endif

GLOBLIBS  = libG4tracking.lib libG4processes.lib libG4digits_hits.lib
GLOBLIBS += libG4track.lib libG4particles.lib libG4geometry.lib
GLOBLIBS += libG4materials.lib libG4graphics_reps.lib
GLOBLIBS += libG4intercoms.lib libG4global.lib

include $(G4INSTALL)/config/architecture.gmk

CPPFLAGS  += -DG3TOG4_EXPORT
CPPFLAGS  += \
             -I$(G4BASE)/global/management/include \
             -I$(G4BASE)/global/HEPGeometry/include \
             -I$(G4BASE)/global/HEPRandom/include \
             -I$(G4BASE)/geometry/magneticfield/include \
             -I$(G4BASE)/geometry/volumes/include \
             -I$(G4BASE)/geometry/management/include \
             -I$(G4BASE)/geometry/solids/CSG/include \
             -I$(G4BASE)/geometry/solids/specific/include \
             -I$(G4BASE)/geometry/solids/Boolean/include \
             -I$(G4BASE)/geometry/solids/BREPS/include \
             -I$(G4BASE)/graphics_reps/include \
             -I$(G4BASE)/materials/include \
             -I$(G4BASE)/particles/management/include \
             -I$(G4BASE)/processes/management/include \
             -I$(G4BASE)/processes/general/include \
             -I$(G4BASE)/processes/decay/include \
             -I$(G4BASE)/track/include \
             -I$(G4BASE)/tracking/include \
             -I$(G4BASE)/digits_hits/detector/include \
             -I$(G4BASE)/digits_hits/hits/include

###	kludge for AIX

ifeq ($(FC),xlf)
	cpp_include_flag=-WF,-I
### -d writes out F*.f file
	FCFLAGS += -d $(patsubst -I%,$(cpp_include_flag)%,-Iinclude)
### xlf prepends .f name with F
	dotf_prepend=F
else
### other compilers do not prepend .f name with F
	dotf_prepend=
	FCFLAGS += -Iinclude
endif

.SUFFIXES: 
.SUFFIXES:	.a .o .cc .cxx .c .f .F .d

### --------------------- vpaths ----------------------------------------------

vpath %.hh  $(patsubst -I%,%,$(filter -I%,$(CPPFLAGS)))
vpath %.inc $(patsubst -I%,%,$(filter -I%,$(CPPFLAGS)))

G4LIBDIR := $(G4LIB)/$(G4SYSTEM)
G4TMPDIR := $(G4TMP)/$(G4SYSTEM)/$(name)
G4BINDIR := $(G4BIN)/$(G4SYSTEM)

Fbinsources = src/$(G4TARGET).F
Fbinobjects = $(patsubst src/%.F,$(G4TMPDIR)/%.o,$(Fbinsources))
Flibsources = $(filter-out $(Fbinsources),$(wildcard src/*.F))
Flibobjects = $(patsubst src/%.F,$(G4TMPDIR)/%.o,$(Flibsources))
Fdependencies = $(patsubst %.o,%.d,$(Fbinobjects) $(Flibobjects))

ifdef G3G4DEBUG
  CPPFLAGS += -DG3G4DEBUG
endif
ifdef G3G4_NO_REFLECTION
  CPPFLAGS += -DG3G4_NO_REFLECTION
endif

###############################################################################
### user beware! .rz files made with cernlib v 95a cannot be read with later
### versions. You can use cernlib -v option to specify the cernlib version 
###############################################################################

### .PHONY targets are executed regardless of time-stamp of any file of same 
###  name

.PHONY: global all lib bin libc libF

###--------------------- section to define explicit rules --------------------

lib: libc

global: cleanf lib bin

all: lib


ifneq ($(G4LIB_BUILD_SHARED),)
libc : $(G4LIBDIR)/lib$(name).$(SHEXT)
else
libc : $(G4LIBDIR)/lib$(name).a
endif
	@echo $@ stage done

libF : $(G4LIBDIR)/lib$(name)F.a
	@echo $@ stage done

bin: $(G4BINDIR)/$(G4TARGET)
	@echo $@ stage done

### Make Fortran library.

$(G4LIBDIR)/lib$(name)F.a: $(Flibobjects)
	@if [ ! -d $(G4LIBDIR) ] ; then mkdir $(G4LIBDIR) ;fi
	@echo Creating/replacing object files in lib$(name)F.a
	@ar r $@ $?
	@if [ -f /usr/bin/ranlib -o -f /bin/ranlib ] ; then \
	echo Running ranlib on lib$(name)F.a ; ranlib $@ ; fi

### rztog4: Standalone Fortran program that reads an RZ file and converts
###       the geometry and other init structures therein to G4. A call
###       list file and G4 C++ code are generated; either can be used
###       on the G4 side to build the geometry.

$(G4BINDIR)/$(G4TARGET) : $(Fbinobjects) $(G4LIBDIR)/lib$(name)F.a
	@if [ ! -d $(G4BINDIR) ] ; then mkdir $(G4BINDIR) ;fi
	@echo Linking $(G4TARGET)
	$(RM) $@
	echo $(G4TARGET) dependencies: $^
	$(FC) $(Fbinobjects) -o $@ $(G4LIBDIR)/lib$(name)F.a $(shell cernlib -v pro geant321 packlib)
	@echo Finished linking $(G4TARGET)

###-------------------- locally defined pattern rules ------------------------

### Make the .o files from src/*.F
#     need to wait until libc is done, before strting to compile fortran
#         or building libc may pick up fortran compiled objects into libc
$(G4TMPDIR)/%.o : src/%.F libc
	$(FC) $(FCFLAGS) -o $(G4TMPDIR)/$(*F).o -c src/$(*F).F
	@file=$(dotf_prepend)$(*F).f;\
	if [ -f $$file ]; then echo Moving $$file to $(G4TMPDIR)/$$file; \
	mv $$file $(G4TMPDIR)/$$file; fi	

### Move the .f files
$(G4TMPDIR)/%.f : %.f
	@echo Moving $< to $@
	@mv $< $@

### --------------------- .d files --------------------------------------------

### Make the .d file(s) and include it(them).
### g++ -MM is good at this, except it forgets the subdirectory (hence
###  the echo $(@D)/).

$(G4TMPDIR)/%.d: src/%.F
	@if [ ! -d $(G4TMP)/$(G4SYSTEM) ] ; then mkdir $(G4TMP)/$(G4SYSTEM) ;fi
	@if [ ! -d $(G4TMPDIR) ] ; then mkdir $(G4TMPDIR) ;fi
	@if test -f $@ ; then rm -f $@; fi
	@echo $(G4LIBDIR)/lib$(name)F.a\($*.o\) $@ " : " $? "\\" > $@
	@if test -f $< ; then awk '$$1=="#include" { print $$2 " \\" }' $< | tr -d \"\>\< >> $@ ; echo >> $@ ; fi

###@if test -f $< ; then echo Making $@ depend file for $<; awk '$$1=="#include" { print $$2 " \\" }' $< | tr -d \"\>\< >> $@ ; echo >> $@ ; fi

### --------------------- end of pattern rules --------------------------------

-include $(Fdependencies)

clean:: cleanf

cleanf:
	@$(RM) $(Flibobjects)
	@$(RM) $(Fbinobjects)
	@$(RM) $(Fdependencies)
	@$(RM) $(G4BINDIR)/$(G4TARGET)

include $(G4INSTALL)/config/common.gmk

test1:
	@echo $(sources)
	@echo $(objects)
	@echo $(dependencies)
