#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the Common Development
# and Distribution License Version 1.0 (the "License").
#
# You can obtain a copy of the license at
# http://www.opensource.org/licenses/CDDL-1.0.  See the License for the
# specific language governing permissions and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each file and
# include the License file in a prominent location with the name LICENSE.CDDL.
# If applicable, add the following below this CDDL HEADER, with the fields
# enclosed by brackets "[]" replaced with your own identifying information:
#
# Portions Copyright (c) [yyyy] [name of copyright owner]. All rights reserved.
#
# CDDL HEADER END
#

#
# Copyright (c) 2013--2014, Regents of the University of Minnesota.
# All rights reserved.
#
# Contributors:
#    Ryan S. Elliott
#

#
# Release: This file is part of the kim-api.git repository.
#

ifeq ($(wildcard Makefile.KIM_Config),)
  $(error Makefile.KIM_Config does not exist.  Please create this file in order to compile the KIM API package)
endif
include Makefile.KIM_Config

#
# List of "installed" model drivers and models
#
export MODEL_DRIVERS_LIST := $(filter-out $(if $(wildcard $(srcdir)/$(modeldriversdir)/.kimignore),$(shell cat $(srcdir)/$(modeldriversdir)/.kimignore),),$(patsubst $(srcdir)/$(modeldriversdir)/%/,%,$(filter-out $(srcdir)/$(modeldriversdir)/,$(sort $(dir $(wildcard $(srcdir)/$(modeldriversdir)/*/))))))
export MODELS_LIST        := $(filter-out $(if $(wildcard $(srcdir)/$(modelsdir)/.kimignore),$(shell cat $(srcdir)/$(modelsdir)/.kimignore),),$(patsubst $(srcdir)/$(modelsdir)/%/,%,$(filter-out $(srcdir)/$(modelsdir)/,$(sort $(dir $(wildcard $(srcdir)/$(modelsdir)/*/))))))


#
# Main build settings and rules
#
.PHONY: all models_check config \
        $(patsubst %,%-all,$(MODEL_DRIVERS_LIST) $(MODELS_LIST)) \
        utils-all kim-api-objects kim-api-libs

ifeq (dynamic-load,$(KIM_LINK))
   all: models_check config kim-api-objects kim-api-libs utils-all $(patsubst %,%-all,$(MODEL_DRIVERS_LIST) \
        $(MODELS_LIST))
else
   all: models_check config kim-api-objects $(patsubst %,%-all,$(MODEL_DRIVERS_LIST) $(MODELS_LIST)) \
        kim-api-libs utils-all
endif

# build targets involved in "make all"
Makefile:
	@touch Makefile

models_check:
	@if test \( X"$(MODELS_LIST)" = X"" \) -a \( X"$(KIM_LINK)" = X"static-link" \); then     \
        printf "*******************************************************************************\n"; \
        printf "*******     Can't compile the API for static linking with no Models     *******\n"; \
        printf "*******              Maybe you want to do 'make examples'               *******\n"; \
        printf "*******************************************************************************\n"; \
        false; else true; fi

KIM_CONFIG_FILES = $(srcdir)/Makefile.KIM_Config \
                   $(srcdir)/utils/Makefile.KIM_Config \
                   $(srcdir)/$(modeldriversdir)/Makefile.KIM_Config \
                   $(srcdir)/$(modelsdir)/Makefile.KIM_Config \
                   $(KIM_DIR)/$(examplesdir)/Makefile.KIM_Config \
                   $(KIM_DIR)/$(examplesdir)/$(modelsdir)/Makefile.KIM_Config \
                   $(KIM_DIR)/$(examplesdir)/$(modeldriversdir)/Makefile.KIM_Config \
                   $(KIM_DIR)/$(examplesdir)/openkim_tests/Makefile.KIM_Config \
                   $(KIM_DIR)/$(examplesdir)/simulators/Makefile.KIM_Config

config: $(KIM_CONFIG_FILES)

$(KIM_CONFIG_FILES): Makefile $(KIM_DIR)/Makefile.KIM_Config
	$(QUELL)if test -d $(dir $@); then \
                  printf "Creating... KIM_Config file..... $(patsubst $(KIM_DIR)/%,%,$@).\n"; \
                  printf "# This file is automatically generated by the KIM API build system.\n" >  $@; \
                  printf "# Do not edit!\n"                                                      >> $@; \
                  printf "\n"                                                                    >> $@; \
                  printf "include $(KIM_DIR)/Makefile.KIM_Config\n"                              >> $@; \
                fi

kim-api-objects: Makefile kim-api-objects-making-echo
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir) objects

kim-api-libs: Makefile kim-api-libs-making-echo
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir) libs

utils-all: Makefile src/utils-making-echo
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir)/utils all

$(patsubst %,%-all,$(MODEL_DRIVERS_LIST)): %: Makefile Model@Driver...@%-making-echo | kim-api-objects
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir)/$(modeldriversdir)/$(patsubst %-all,%,$@) all

$(patsubst %,%-all,$(MODELS_LIST)): %: Makefile Model..........@%-making-echo | kim-api-objects
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir)/$(modelsdir)/$(patsubst %-all,%,$@) all


#
# Main clean rules and targets
#
.PHONY: clean kim-api-clean config-clean utils-clean \
        $(patsubst %,%-clean,$(MODEL_DRIVERS_LIST) $(MODELS_LIST))

clean: config $(patsubst %,%-clean,$(MODEL_DRIVERS_LIST) $(MODELS_LIST)) kim-api-clean utils-clean config-clean

# build targets involved in "make clean"
$(patsubst %,%-clean,$(MODEL_DRIVERS_LIST)):
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir)/$(modeldriversdir)/$(patsubst %-clean,%,$@) clean

$(patsubst %,%-clean,$(MODELS_LIST)):
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir)/$(modelsdir)/$(patsubst %-clean,%,$@) clean

kim-api-clean:
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir) clean
	$(QUELL)rm -f kim.log

utils-clean:
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir)/utils clean

config-clean:
	@printf "Cleaning... KIM_Config files.\n"
	$(QUELL)rm -f $(KIM_CONFIG_FILES)


#
# Main ls-* settings and rules
#
.PHONY: ls-model-drivers ls-models ls-all

ls-model-drivers:
	$(QUELL)$(foreach mdr,$(notdir $(shell find "$(srcdir)/$(modeldriversdir)" -maxdepth 1 -mindepth 1 -type d -exec basename {} \;)),\
          printf "*@existing.....@%-50s@found@in@@@$(srcdir)/$(modeldriversdir)\n" $(mdr)@ | sed -e 's/ /./g' -e 's/@/ /g';)

ls-models:
	$(QUELL)$(foreach ml,$(notdir $(shell find "$(srcdir)/$(modelsdir)" -maxdepth 1 -mindepth 1 -type d -exec basename {} \;)),\
          printf "*@existing.....@%-50s@found@in@@@$(srcdir)/$(modelsdir)\n" $(mdr)@ | sed -e 's/ /./g' -e 's/@/ /g';)

ls-all: ls-model-drivers ls-models


#
# Main add-* settings and rules
#
.PHONY: add-examples add-OpenKIM

add-examples:
	$(QUELL)$(foreach exmpl,$(notdir $(shell find "$(KIM_DIR)/$(examplesdir)/$(modeldriversdir)" -maxdepth 1 -mindepth 1 -type d -exec basename {} \;)),\
          if test -e "$(srcdir)/$(modeldriversdir)/$(exmpl)"; then \
          printf "*@existing.....@%-50s@no@copy@performed!\n" $(exmpl)@ | sed -e 's/ /./g' -e 's/@/ /g'; else \
          printf "*@adding.......@%-50s@copied@to@@$(srcdir)/$(modeldriversdir)\n" $(exmpl)@ | sed -e 's/ /./g' -e 's/@/ /g'; \
          cp -r "$(KIM_DIR)/$(examplesdir)/$(modeldriversdir)/$(exmpl)" "$(srcdir)/$(modeldriversdir)/"; fi;)
	$(QUELL)$(foreach exmpl,$(notdir $(shell find "$(KIM_DIR)/$(examplesdir)/$(modelsdir)" -maxdepth 1 -mindepth 1 -type d -exec basename {} \;)),\
          if test -e "$(srcdir)/$(modelsdir)/$(exmpl)"; then \
          printf "*@existing.....@%-50s@no@copy@performed!\n" $(exmpl)@ | sed -e 's/ /./g' -e 's/@/ /g'; else \
          printf "*@adding.......@%-50s@copied@to@@$(srcdir)/$(modelsdir)\n" $(exmpl)@ | sed -e 's/ /./g' -e 's/@/ /g'; \
          cp -r "$(KIM_DIR)/$(examplesdir)/$(modelsdir)/$(exmpl)" "$(srcdir)/$(modelsdir)/"; fi;)

add-OpenKIM:
	$(QUULL)printf "Complete download of OpenKIM Repository Model Drivers and Models not yet possible.\n" && false

add-%:
	$(QUELL)if test x"__MD_" = x`printf "$*" | sed 's/.*\(__MD_\).*/\1/'`; then \
                  (cd "$(srcdir)/$(modeldriversdir)" && \
                   wget -nv --content-disposition 'https://kim-items.openkim.org/archive?kimid=$*&compression=gz' && \
                   tar zxvf "$*.tgz" && \
                   rm -f "$*.tgz"); \
                elif test x"__MO_" = x`printf "$*" | sed 's/.*\(__MO_\).*/\1/'`; then \
                  (cd "$(srcdir)/$(modelsdir)" && \
                   wget -nv --content-disposition 'https://kim-items.openkim.org/archive?kimid=$*&compression=gz' && \
                   tar zxvf "$*.tgz" && \
                   rm -f "$*.tgz"); \
                elif test \( x"ex_model_driver_" = x`printf "$*" | sed 's/^\(ex_model_driver_\).*/\1/'` -a -d "$(KIM_DIR)/$(examplesdir)/$(modeldriversdir)/$*" \); then \
                   if test -e "$(srcdir)/$(modeldriversdir)/$*"; then \
                     printf "*@existing.....@%-50s@no@copy@performed!\n" $*@ | sed -e 's/ /./g' -e 's/@/ /g'; else \
                     printf "*@adding.......@%-50s@copied@to@@$(srcdir)/$(modeldriversdir)\n" $*@ | sed -e 's/ /./g' -e 's/@/ /g'; \
                     cp -r "$(KIM_DIR)/$(examplesdir)/$(modeldriversdir)/$*" "$(srcdir)/$(modeldriversdir)/"; \
                  fi; \
                elif test \( x"ex_model_" = x`printf "$*" | sed 's/^\(ex_model_\).*/\1/'` -a -d "$(KIM_DIR)/$(examplesdir)/$(modelsdir)/$*" \); then \
                   if test -e "$(srcdir)/$(modelsdir)/$*"; then \
                     printf "*@existing.....@%-50s@no@copy@performed!\n" $*@ | sed -e 's/ /./g' -e 's/@/ /g'; else \
                     printf "*@adding.......@%-50s@copied@to@@$(srcdir)/$(modelsdir)\n" $*@ | sed -e 's/ /./g' -e 's/@/ /g'; \
                     cp -r "$(KIM_DIR)/$(examplesdir)/$(modelsdir)/$*" "$(srcdir)/$(modelsdir)/"; \
                   fi; \
                else \
                  printf "Unknown OpenKIM item or example name: $*.\n" && false; \
                fi


#
# Main rm-* settings and rules
#
.PHONY: rm-examples rm-all rm-all-model-drivers rm-all-models

rm-examples:
	$(QUELL)$(foreach exmpl,$(notdir $(shell find "$(KIM_DIR)/$(examplesdir)/$(modeldriversdir)" -maxdepth 1 -mindepth 1 -type d -exec basename {} \;)),\
          if test -e "$(srcdir)/$(modeldriversdir)/$(exmpl)"; then \
          printf "*@removing.....@%-50s@rm'ed@from@$(srcdir)/$(modeldriversdir)\n" $(exmpl)@ | sed -e 's/ /./g' -e 's/@/ /g'; \
          rm -rf "$(srcdir)/$(modeldriversdir)/$(exmpl)"; fi;)
	$(QUELL)$(foreach exmpl,$(notdir $(shell find "$(KIM_DIR)/$(examplesdir)/$(modelsdir)" -maxdepth 1 -mindepth 1 -type d -exec basename {} \;)),\
          if test -e $(srcdir)/$(modelsdir)/$(exmpl); then \
          printf "*@removing.....@%-50s@rm'ed@from@$(srcdir)/$(modelsdir)\n" $(exmpl)@ | sed -e 's/ /./g' -e 's/@/ /g'; \
          rm -rf "$(srcdir)/$(modelsdir)/$(exmpl)"; fi;)

rm-all-model-drivers:
	$(QUELL)$(foreach mdr,$(notdir $(shell find "$(srcdir)/$(modeldriversdir)" -maxdepth 1 -mindepth 1 -type d -exec basename {} \;)),\
                printf "*@removing.....@%-50s@rm'ed@from@$(srcdir)/$(modeldriversdir)\n" $(mdr)@ | sed -e 's/ /./g' -e 's/@/ /g'; \
                rm -rf "$(srcdir)/$(modeldriversdir)/$(mdr)";)

rm-all-models:
	$(QUELL)$(foreach ml,$(notdir $(shell find "$(srcdir)/$(modelsdir)" -maxdepth 1 -mindepth 1 -type d -exec basename {} \;)),\
                printf "*@removing.....@%-50s@rm'ed@from@$(srcdir)/$(modelsdir)\n" $(ml)@ | sed -e 's/ /./g' -e 's/@/ /g'; \
                rm -rf "$(srcdir)/$(modelsdir)/$(ml)";)

rm-all: rm-all-model-drivers rm-all-models

rm-%:
	$(QUELL)if test \( x"__MD_" = x`printf "$*" | sed 's/.*\(__MD_\).*/\1/'` -a -d "$(srcdir)/$(modeldriversdir)/$*" \); then \
                  printf "*@removing.....@%-50s@rm'ed@from@$(srcdir)/$(modeldriversdir)\n" $*@ | sed -e 's/ /./g' -e 's/@/ /g'; \
                  (cd "$(srcdir)/$(modeldriversdir)" && rm -rf "$*"); \
                elif test \( x"__MO_" = x`printf "$*" | sed 's/.*\(__MO_\).*/\1/'` -a -d "$(srcdir)/$(modelsdir)/$*" \); then \
                  printf "*@removing.....@%-50s@rm'ed@from@$(srcdir)/$(modelsdir)\n" $*@ | sed -e 's/ /./g' -e 's/@/ /g'; \
                  (cd "$(srcdir)/$(modelsdir)" && rm -rf "$*"); \
                else \
                  printf "OpenKIM item name, $*, not found.  Nothing removed.\n"; \
                fi


#
# Main install settings and rules
#
.PHONY: install install-check installdirs kim-api-objects-install kim-api-libs-install config-install utils-install\
        $(patsubst %,%-install,$(MODEL_DRIVERS_LIST) $(MODELS_LIST))

ifeq (dynamic-load,$(KIM_LINK))
  install: install-check config kim-api-objects-install kim-api-libs-install utils-install $(patsubst %,%-install,$(MODEL_DRIVERS_LIST) $(MODELS_LIST)) config-install
else
  install: install-check config kim-api-objects-install $(patsubst %,%-install,$(MODEL_DRIVERS_LIST) $(MODELS_LIST)) kim-api-libs-install utils-install config-install
endif

# build targets involved in "make install"
install_builddir = $(dest_package_dir)/$(builddir)
install_make = Makefile.Generic Makefile.LoadDefaults Makefile.Model Makefile.ModelDriver Makefile.ParameterizedModel Makefile.SanityCheck Makefile.Test parameterized_model.cpp
install_compilerdir = $(dest_package_dir)/$(buildcompilerdir)
install_compiler = Makefile.GCC Makefile.INTEL
install_linkerdir = $(dest_package_dir)/$(buildlinkerdir)
install_linker = Makefile.DARWIN Makefile.FREEBSD Makefile.LINUX

install-check:
ifneq (dynamic-load,$(KIM_LINK))
	@if test -d "$(dest_package_dir)"; then \
        printf "*******************************************************************************\n"; \
        printf "*******               This package is already installed.                *******\n"; \
        printf "*******                 Please 'make uninstall' first.                  *******\n"; \
        printf "*******************************************************************************\n"; \
        false; else true; fi
else
        # should we check that the installed stuff is actually dynamic-load and the right settings (32bit, etc.)?
	$(QUELL)if test -d "$(dest_package_dir)"; then \
                  rm -rf "$(install_linkerdir)"; \
                  rm -rf "$(install_compilerdir)"; \
                  rm -rf "$(install_builddir)"; \
                  rm -f  "$(dest_package_dir)/Makefile.KIM_Config"; \
                  rm -f  "$(dest_package_dir)/Makefile.Version"; \
                fi
endif

kim-api-objects-install:
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir) objects-install

kim-api-libs-install:
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir) libs-install

utils-install:
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir)/utils install

$(patsubst %,%-install,$(MODEL_DRIVERS_LIST)):
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir)/$(modeldriversdir)/$(patsubst %-install,%,$@) install

$(patsubst %,%-install,$(MODELS_LIST)):
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir)/$(modelsdir)/$(patsubst %-install,%,$@) install

config-install: installdirs
	@printf "Installing...($(dest_package_dir))................................. KIM_Config files"
ifeq (dynamic-load,$(KIM_LINK))
        # Install make directory
	$(QUELL)for fl in $(install_make); do $(INSTALL_PROGRAM) -m 0644 "$(builddir)/$$fl" "$(install_builddir)/$$fl"; done
	$(QUELL)printf ',s|^ *srcdir *=.*|srcdir = $$(KIM_DIR)|\nw\nq\n' | ed "$(install_builddir)/Makefile.Generic" > /dev/null 2>&1
	$(QUELL)printf ',s|^ *KIMINCLUDEFLAGS *=.*|KIMINCLUDEFLAGS = -I$$(KIM_DIR)/include|\nw\nq\n' | ed "$(install_builddir)/Makefile.Generic" > /dev/null 2>&1
        # Install compiler defaults directory
	$(QUELL)for fl in $(install_compiler); do $(INSTALL_PROGRAM) -m 0644 "$(buildcompilerdir)/$$fl" "$(install_compilerdir)/$$fl"; done
        # Install linker defaults directory
	$(QUELL)for fl in $(install_linker); do $(INSTALL_PROGRAM) -m 0644 "$(buildlinkerdir)/$$fl" "$(install_linkerdir)/$$fl"; done
        # Install KIM_Config file
	$(QUELL)$(INSTALL_PROGRAM) -m 0644 Makefile.KIM_Config "$(dest_package_dir)/Makefile.KIM_Config"
	$(QUELL)fl="$(dest_package_dir)/Makefile.KIM_Config" && \
                printf ',s|^ *KIM_DIR *=.*|KIM_DIR = $(dest_package_dir)|\nw\nq\n' | ed "$$fl" > /dev/null 2>&1
        # Install version file
	$(QUELL)$(INSTALL_PROGRAM) -m 0644 Makefile.Version "$(dest_package_dir)/Makefile.Version"
  ifeq (true,$(shell git rev-parse --is-inside-work-tree 2> /dev/null))
	$(QUELL)printf ',s|\$$(shell[^)]*).|$(shell git rev-parse --short HEAD)|\nw\nq\n' | ed "$(dest_package_dir)/Makefile.Version" > /dev/null 2>&1
  endif
	@printf ".\n"
else
	@printf ": nothing to be done for $(KIM_LINK).\n";
endif

installdirs:
ifeq (dynamic-load,$(KIM_LINK))
	$(QUELL)$(INSTALL_PROGRAM) -d -m 0755 "$(install_builddir)"
	$(QUELL)$(INSTALL_PROGRAM) -d -m 0755 "$(install_compilerdir)"
	$(QUELL)$(INSTALL_PROGRAM) -d -m 0755 "$(install_linkerdir)"
endif

# targets for setting default system-wide library
install-set-default-to-v%: EXT:=$(if $(filter-out static-link,$(KIM_LINK)),so,a)
install-set-default-to-v%:
	@printf "Setting default $(package_name) to $(package_name)-v$*\n"
        # ignore the bin files at this stage (maybe add support for default bins later...)
	$(QUELL)fl="$(DESTDIR)$(includedir)/$(package_name)"       && if test -L "$$fl"; then rm -f "$$fl"; fi && ln -fs "$(package_name)-v$*" "$$fl"
	$(QUELL)fl="$(DESTDIR)$(libdir)/$(package_name)"           && if test -L "$$fl"; then rm -f "$$fl"; fi && ln -fs "$(package_name)-v$*" "$$fl"
	$(QUELL)fl="$(DESTDIR)$(libdir)/lib$(package_name).$(EXT)" && if test -L "$$fl"; then rm -f "$$fl"; fi && ln -fs "lib$(package_name)-v$*.$(EXT)" "$$fl"


#
# Main uninstall settings and rules
#
.PHONY: uninstall kim-api-objects-uninstall kim-api-libs-uninstall utils-uninstall config-uninstall

uninstall: config kim-api-objects-uninstall utils-uninstall kim-api-libs-uninstall config-uninstall

# targets involved in "make uninstall"
kim-api-objects-uninstall:
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir) objects-uninstall

utils-uninstall:
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir)/utils uninstall

kim-api-libs-uninstall:
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(srcdir) libs-uninstall

config-uninstall:
	@printf "Uninstalling...($(dest_package_dir))................................. KIM_Config files.\n"
        # Make sure the package directory is gone
	$(QUELL)if test -d "$(dest_package_dir)"; then rm -rf "$(dest_package_dir)"; fi
        # Uninstall the rest
	$(QUELL)if test -d "$(DESTDIR)$(includedir)"; then rmdir "$(DESTDIR)$(includedir)" > /dev/null 2>&1 || true; fi
	$(QUELL)if test -d "$(DESTDIR)$(bindir)"; then rmdir "$(DESTDIR)$(bindir)" > /dev/null 2>&1 || true; fi
	$(QUELL)if test -d "$(DESTDIR)$(libdir)"; then rmdir "$(DESTDIR)$(libdir)" > /dev/null 2>&1 || true; fi
	$(QUELL)if test -d "$(DESTDIR)$(exec_prefix)"; then rmdir "$(DESTDIR)$(exec_prefix)" > /dev/null 2>&1 || true; fi
	$(QUELL)if test -d "$(DESTDIR)$(prefix)"; then rmdir "$(DESTDIR)$(prefix)" > /dev/null 2>&1 || true; fi

# targets for unsetting default system-wide library
uninstall-set-default: EXT:=$(if $(filter-out static-link,$(KIM_LINK)),so,a)
uninstall-set-default:
	@printf "Removing default $(package_name) settings.\n"
	$(QUELL)fl="$(DESTDIR)$(includedir)/$(package_name)"       && if test -L "$$fl"; then rm -f "$$fl"; fi
	$(QUELL)fl="$(DESTDIR)$(libdir)/$(package_name)"           && if test -L "$$fl"; then rm -f "$$fl"; fi
	$(QUELL)fl="$(DESTDIR)$(libdir)/lib$(package_name).$(EXT)" && if test -L "$$fl"; then rm -f "$$fl"; fi


#
# Examples build settings and rules
#
.PHONY: examples examples-all examples-clean

INPLACE_CONFIG = $(KIM_DIR)/.$(package_name)/config-v$(VERSION_MAJOR)

$(INPLACE_CONFIG): Makefile
	@printf "Creating... User Config file.... $@.\n"
	$(QUELL)$(INSTALL_PROGRAM) -d -m 0755 $(KIM_DIR)/.$(package_name)
	$(QUELL)printf "model_drivers_dir = %s\n" $(KIM_DIR)/$(examplesdir)/$(modeldriversdir) >  $@; \
                printf "models_dir = %s\n" $(KIM_DIR)/$(examplesdir)/$(modelsdir)              >> $@

examples: all $(INPLACE_CONFIG) examples-all

examples-all:
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(KIM_DIR)/$(examplesdir) all

examples-clean: config
	$(QUELL)$(MAKE) $(MAKE_FLAGS) -C $(KIM_DIR)/$(examplesdir) clean
	$(QUELL)rm -rf $(KIM_DIR)/.$(package_name)


########### for internal use ###########
%-making-echo:
	@printf "\n%79s\n" " " | sed -e 's/ /*/g'
	@printf "%-77s%2s\n" "** Making... `printf "$(patsubst %-all,%,$*)" | sed -e 's/@/ /g'`" "**"
	@printf "%79s\n" " " | sed -e 's/ /*/g'
