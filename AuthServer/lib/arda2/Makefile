################################################################################
#
# Name: Makefile
# Purpose: Makefile for arda2 (based on platform/common.mk)
#
################################################################################

TOP     = .
HERE    = .

LIBNAME = arda2

################################################################################
# build type
#   debug    - full debug build
#   release  - production release, full optimization

.PHONY : default
default : all

ifndef BUILD
  BUILD=debug
endif

# Allow the targets "debug" and "release" to be used as a shortcut for the 
# preferred form "make BUILD=xxx"
.PHONY : debug
debug: all

.PHONY : release
release:
	+make BUILD=release all

################################################################################
# OS Commands
CC              = gcc
CP              = cp
CXX             = g++
AR              = ar
ARFLAGS	        = cvq
INSTALL         = install -c
INSTALL_DATA    = $(INSTALL) -m 0644
MKDIR           = mkdir -p
RM              = rm -f
MAKE            = gmake


################################################################################
# Build directory structure

# standard paths
SRCDIR = .
BLDDIR = $(BUILD)

# dependencies
DEPS = $(BLDDIR)/.deps
ifeq ($(TARGET), clean)
  NO_DEPENDENCIES = 1
endif

# search paths
INCDIRS    =	-I.						\
		-I$(TOP)/..					\
		-I$(TOP)/../external/bzip2/include			\
		-I$(TOP)/../external/openssl/include/linux		\
		-I$(TOP)/../external/zlib
LIBDIRS    = -L. 

################################################################################
# Build flags

ifeq ($(BUILD), debug)
  BUILDFLAGS = -g -D_DEBUG -DBUILD_UNIT_TESTS -DLINUX
	DEBUG_FLAG = d
else
    ifeq ($(BUILD), release)
      BUILDFLAGS = -O2
    else
      $(error "Invalid BUILD type.  Must be release or debug")
    endif
endif

CPPFLAGS = $(BUILDFLAGS) -W -Wcast-align -Wno-unused -Wno-multichar -D_REENTRANT

################################################################################
# Macros

define mk-deps
	$(CXX) -MM $(CPPFLAGS) $(INCDIRS) $^ | sed "s@^[_A-Z0-9a-z]*\.o: \([_A-Z0-9a-z\/]*\)\.cpp@$(BLDDIR)/\1.o: \1.cpp@" > $(DEPS)
endef

define mk-dir
	test -d $(patsubst %/, %, $(@D)) || $(MKDIR) $(patsubst %/, %, $(@D))
endef

################################################################################
# if SUBDIRS is specified, there is nothing to build in this directory.
# Recurse into subdirectories instead

ifdef SUBDIRS

.PHONY : all
all:
	$(foreach d,$(SUBDIRS),	$(MAKE) -C $d $@ -s || exit 1;)

else

################################################################################
# Common steps for building a library or executable

ifndef SRCS_CPP
#automatically find source files
  SRCS_CPP = $(shell find . -name '*.cpp' | sed "s@\./@@")
endif

# assemble list of source and object files
srcs = $(SRCS_CPP)
objs = $(SRCS_CPP:%.cpp=$(BLDDIR)/%.o )

# build object files from c++ files
$(BLDDIR)/%.o: %.cpp
	@echo "Building" $@
	@$(mk-dir)
	@$(COMPILE.cpp) $(INCDIRS) $(OUTPUT_OPTION) $< || ($(RM) $@; exit 1)

################################################################################
# Build a library

ifdef LIBNAME

ifndef LIBFILENAME
  LIBFILENAME = lib$(LIBNAME)$(DEBUG_FLAG).a
endif

# set targets
deps-targets = $(srcs)

.PHONY : clean
clean: 
	rm -rf debug release
	rm  -f linux/lib$(LIBNAME)d.a
	rm  -f linux/lib$(LIBNAME).a

.PHONY : all
all: linux/$(LIBFILENAME)

# build lib from object files
linux/$(LIBFILENAME): $(objs)
	@echo "Linking " $@
	@$(mk-dir)
	@$(AR) $(ARFLAGS) $@ $^ > /dev/null  || ($(RM) $@; exit 1)

ifndef NO_DEPENDENCIES
$(DEPS): $(deps-targets)
	@echo "Checking dependencies for" $(LIBNAME)
	@$(mk-dir)
	@$(mk-deps)
endif

-include $(DEPS)

else

################################################################################
# Build an executable

ifdef EXENAME

ifndef EXEFILENAME
	EXEFILENAME = $(EXENAME)$(DEBUG_FLAG)
endif

################################################################################
#
# Name: exe.mk
# Purpose: builds executable
#
################################################################################

deps-targets = $(srcs)
prog-targets = $(BLDDIR)/$(EXEFILENAME) $(LNKDIR)/$(EXEFILENAME)

# setup library dependencies
LIBS += $(foreach m, $(LIB_DEPS), -l$(m)$(DEBUG_FLAG))
LIBS += $(foreach m, $(EXTRA_LIB_DEPS), -l$(m))

###############################################################################
# executable build rules

.PHONY : clean
clean: 
	rm -rf debug release
	rm  -f $(LNKDIR)/$(EXENAME)d
	rm  -f $(LNKDIR)/$(EXENAME)

.PHONY : all
all: $(LNKDIR)/$(EXEFILENAME)

$(BLDDIR)/$(EXEFILENAME): $(objs)
	@echo "Linking " $@
	@$(mk-dir)
	@$(RM) $@
	@$(LINK.C) $(LIBDIRS) -o $@ $^ $(LIBS) || ($(RM) $@; exit 1)

# copy prog to distribution directory
$(LNKDIR)/$(EXEFILENAME): $(BLDDIR)/$(EXEFILENAME)
	@echo "Installing " $@
	$(install-binary)


endif
endif
endif
