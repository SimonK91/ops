## Configurable defines
BUILD_ROOT?=build
BUILD_SUFFIX?=
DEPLOY_SUFFIX?=

BUILD_DEBUG?=$(BUILD_ROOT).debug$(BUILD_SUFFIX)
BUILD_OPT?=$(BUILD_ROOT).opt$(BUILD_SUFFIX)
BUILD_BOOTSTRAP?=$(BUILD_ROOT).bootstrap$(BUILD_SUFFIX)

CC?=$(shell which gcc)
CXX?=$(shell which g++)
INSTALL_PREFIX?=$(CURDIR)/deploy$(DEPLOY_SUFFIX)

OPS_BUILD_UNITTESTS?=ON
OPS_BUILD_EXAMPLES?=ON

# Common defines
CCV=$(shell $(CC) -dumpversion)
CXXV=$(shell $(CXX) -dumpversion)

COMMON_DEFINES_FOR_CMAKE= \
			-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) \
			-DOPS_BUILD_UNITTESTS=$(OPS_BUILD_UNITTESTS) \
			-DOPS_BUILD_EXAMPLES=$(OPS_BUILD_EXAMPLES)

## Rules

.PHONY : all
all: debug opt
	$(MAKE) install

.PHONY : clean
clean: clean_debug clean_opt clean_deploy clean_bootstrap clean_tools

.PHONY : clean_bootstrap
clean_bootstrap:
	@echo "Cleaning bootstrap"
	rm -rf $(BUILD_BOOTSTRAP)
	rm -rf Common/idl/Generated
	rm -rf Cpp/include/OPSStringLengths.h

.PHONY : clean_debug
clean_debug:
	@echo "Cleaning debug"
	rm -rf $(BUILD_DEBUG)

.PHONY : clean_opt
clean_opt:
	@echo "Cleaning opt"
	rm -rf $(BUILD_OPT)

.PHONY : clean_deploy
clean_deploy:
	@echo "Cleaning deploy"
	rm -rf $(INSTALL_PREFIX)

.PHONY : clean_tools
clean_tools:
	@echo "Cleaning tools"
	rm -rf Tools/OPSBridge/idl/Generated

# rebuild_cache is used so we can change the configurable sizes and get them
# generated without cleaning all
.PHONY : bootstrap
bootstrap: $(BUILD_BOOTSTRAP)/Makefile
	$(MAKE) -C $(BUILD_BOOTSTRAP) --no-print-directory rebuild_cache all install

$(BUILD_BOOTSTRAP)/Makefile : %/Makefile :
	export CC=$(CC) && \
	export CXX=$(CXX) && \
	mkdir -p $* && \
	cd $* && \
	cmake -DCMAKE_BUILD_TYPE=Bootstrap $(COMMON_DEFINES_FOR_CMAKE) $(CURDIR)

.PHONY : opt
opt: $(BUILD_OPT)/Makefile
	$(MAKE) -C $(BUILD_OPT) --no-print-directory

$(BUILD_OPT)/Makefile : %/Makefile : bootstrap
	export CC=$(CC) && \
	export CXX=$(CXX) && \
	mkdir -p $* && \
	cd $* && \
	cmake -DCMAKE_BUILD_TYPE=Release $(COMMON_DEFINES_FOR_CMAKE) $(CURDIR)

.PHONY : debug
debug: $(BUILD_DEBUG)/Makefile
	$(MAKE) -C $(BUILD_DEBUG) --no-print-directory

$(BUILD_DEBUG)/Makefile : %/Makefile : bootstrap
	export CC=$(CC) && \
	export CXX=$(CXX) && \
	mkdir -p $* && \
	cd $* && \
	cmake -DCMAKE_BUILD_TYPE=Debug $(COMMON_DEFINES_FOR_CMAKE) $(CURDIR)

.PHONY : unittest-c++
unittest-c++ : debug
	@echo "Running C++ unit tests!!!"
	cd UnitTests/OPStest-C++ && \
	./pizzatest.sh

.PHONY : unittest-python
unittest-python :
	@echo "Running C++ unit tests!!!"
	cd UnitTests/OPStest-Python && \
	./pizzatest.sh

.PHONY : install
install :
	$(MAKE) install_debug
	$(MAKE) install_opt
	$(MAKE) $(INSTALL_PREFIX)/lib/README
	@echo "Installed ops4 in $(INSTALL_PREFIX)"

.PHONY : install_debug
install_debug : debug
	$(MAKE) -C $(BUILD_DEBUG) install

.PHONY : install_opt
install_opt : opt
	$(MAKE) -C $(BUILD_OPT) install

$(INSTALL_PREFIX)/lib/README :
	mkdir -p $(shell dirname $@)
	echo "gcc version: $(CCV)" > $@
	echo "g++ version: $(CXXV)" >> $@

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... bootstrap"
	@echo "... opt (optimized/release)"
	@echo "... debug"
	@echo "... unittest-c++"
	@echo "... unittest-python"
	@echo "... install"
	@echo "... clean (cleans all)"
	@echo "... clean_bootstrap"
	@echo "... clean_debug"
	@echo "... clean_opt"
.PHONY : help
