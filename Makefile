# build type: header-only or library
type = library
# build mode: debug or release
mode = release
# install prefix
prefix = $(HOME)/test/galotfa

# project root directory
PROJECT_ROOT = ${shell env pwd}

BUILD_DIR = $(PROJECT_ROOT)/build

# C++ compiler: priority: clang++ > icpx > icpc (legacy) > g++
ifneq ($(shell which clang++), )
CXX = clang++
else ifneq ($(shell which icpx), )
CXX = icpx
else ifneq ($(shell which icpc), )
CXX = icpc
else ifneq ($(shell which g++), )
CXX = g++
endif

MPICXX = ${shell which mpicxx} -cxx=$(CXX)

include ./make-config/flags  		# settings for build flags
include ./make-config/target		# settings for target

# HACK: this is a dummy target related to the build target 
all: build
	@echo "Building the whole project ..."


clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR)
	@rm -rf $(PROJECT_ROOT)/test_dir/*test.h*
	@rm -rf $(PROJECT_ROOT)/test_dir/test $(PROJECT_ROOT)/test_dir/mpi_test
	@echo "Done!"


ifndef units
TEST_TARGET =
else
TEST_TARGET = $(units)
endif
# support values: which should not be interested by user
include ./make-config/unit_test # settings for test: add -DXX to CXXFLAGS
test: check test_dir/test
	@echo "Testing units: " $(TEST_TARGET)
	@cd $(PROJECT_ROOT)/test_dir && ./test


mpi_test: check test_dir/mpi_test
	@echo "Testing units: " $(TEST_TARGET)
	@cd $(PROJECT_ROOT)/test_dir && mpirun -np 8 ./mpi_test


build: check $(TARGET)
	@mkdir -p $(BUILD_DIR)
ifeq ($(type), header-only)
	@echo "Building header-only ..."
else ifeq ($(type), library)
	@echo "Building library ..."
else
	@echo "Invalid build type: should be one of (header-only, library)"
	@echo "Get:" $(type)
	@exit 1
endif
	@echo "Build done successfully!"


check:
	@echo "Compiler for building:" $(CXX)
ifeq ($(CXX), )
	@echo "C++ compiler not found!"
	@exit 1
else ifeq ($(MPICXX),)
	@echo "MPI C++ compiler not found!"
	@exit 1
endif


install: build
	@echo "Installing ..."
	@mkdir -p $(prefix)
ifeq ($(type), header-only)
	@cp -r $(BUILD_DIR)/include $(prefix)/
else
	@cp -r $(BUILD_DIR)/lib $(prefix)/
	@mkdir $(prefix)/include
	@cp -r $(BUILD_DIR)/src/galotfa.h $(prefix)/include/
endif
	@echo "Done!"

headers = $(shell (ls $(PROJECT_ROOT)/src/))
uninstall:
	@echo "Uninstalling ..."
ifeq ($(findstring galotfa, $(prefix)), "") # if there is no 'galotfa' in prefix
# only remove the files in galotfa
	@rm -rf $(prefix)/include/$(headers)
	@rm -rf $(prefix)/lib/libgalotfa*
else
# if it's a directory only for galotfa, remove the whole directory
	@rm -rf $(prefix)
endif
	@echo "Done!"
	
.PHONY: all clean test install uninstall check test_make
