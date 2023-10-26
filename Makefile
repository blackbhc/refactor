# build type: header-only or static or shared or all
type = header-only
# build mode: debug or release
mode = release
# install prefix
prefix = $(HOME)/.local

# project root directory
PROJECT_ROOT = ${shell env pwd}
CXX = ${shell env which mpicxx}
include ./make-config/flags  		# settings for build flags
include ./make-config/target		# settings for target

BUILD_DIR = $(PROJECT_ROOT)/build

# HACK: this is a dummy target related to the build target 
all: build
	@echo "Building the whole project ..."


clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR) $(PROJECT_ROOT)/build
	@rm -rf $(PROJECT_ROOT)/test_dir/test $(PROJECT_ROOT)/test_dir/mpi_test
	@echo "Done!"


ifndef units
TEST_TARGET = prompt
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
ifeq ($(type), header-only)
	@echo "Building header-only ..."
else ifeq ($(type), static)
	@echo "Building static lib ..."
else ifeq ($(type), shared)
	@echo "Building dynamic lib ..."
else ifeq ($(type), all)
	@echo "Building all ..."
else
	@echo "Invalid build type: should be one of (header-only, static, shared, all)"
	@exit 1
endif
	@echo "Build done successfully!"


check:
	@echo "Compiler for building:" $(CXX)
ifeq ($(CXX),)
	@echo "MPI C++ compiler not found!"
	@exit 1
endif


install: build
	@echo "Installing ..."
	@mkdir -p $(prefix)
	@cp -r $(BUILD_DIR)/* $(prefix)/
	@echo "Done!"

uninstall:
	@echo "Uninstalling ..."
# BUG: determine the exact header file path
	@rm -rf $(prefix)/include/$(TARGET)
	@rm -rf $(prefix)/lib/libgalotfa*
	@echo "Done!"
	

.PHONY: all clean test build install check
