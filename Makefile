RES := 1024
OUTPUT := output/mandel.png
OPTIMIZATION := 3

X=0.5
Y=0.5
S=1.0
I=512
COLOUR=1
ARGUMENTS := -o $(OUTPUT) -r $(RES) -x $(X) -y $(X) -s $(S) -c $(COLOUR) -i $(I)

ifeq ($(MARK),1)
	ARGUMENTS := $(ARGUMENTS) -m
endif
ifeq ($(TRADITIONAL),1)
	ARGUMENTS := $(ARGUMENTS) -t
endif
ifeq ($(THREADED),1)
	ARGUMENTS := $(ARGUMENTS) -a
endif

SOURCE_DIR := src
BUILD_DIR  := mandel
BINARY := $(BUILD_DIR)/mandel
OPTIMIZATION := 3

CXX := g++
FLAGS := -fopenmp
CXXFLAGS := -Wall -Wextra -Wpedantic -std=c++11 -O$(OPTIMIZATION) $(FLAGS)
LINKING := -fopenmp -lpthread

CXXFLAGS.valgrind := $(CXXFLAGS) -g
CXXFLAGS.gprof := $(CXXFLAGS) -g -pg -fno-omit-frame-pointer -fno-inline-functions -DNDEBUG

CHECK_FLAGS := $(BUILD_DIR)/.flags_$(shell echo '$(CXXFLAGS) $(DEFINES)' | md5sum | awk '{print $$1}')

SOURCE_PATHS := $(shell find $(SOURCE_DIR) -type f -name '*.cpp')
INCLUDE_DIRS := $(shell find $(SOURCE_DIR) -type f -name '*.hpp' -exec dirname {} \; | uniq)
OBJECTS     := $(SOURCE_PATHS:%.cpp=%.o)

INCLUDES := $(addprefix -I,$(INCLUDE_DIRS))

.PHONY: all verify call $(OUTPUTS) time gprof callgrind cachegrind .gprof .valgrind

help:
	@echo "TDT4200 Assignment 3"
	@echo ""
	@echo "Targets:"
	@echo "	all 		Builds $(BINARY)"
	@echo "	call		executes $(BINARY)"
	@echo "	clean		cleans up everything"
	@echo "	time		exeuction time"
	@echo "	gprof		gprof svg callgraph"
	@echo "	cachegrind	valgrind cachegrind using kcachegrind"
	@echo "	callgrind	valgrind callgrind using kcachegrind"
	@echo "	mandelnav	executes mandelnav with xviewer"
	@echo ""
	@echo "Options:"
	@echo "	FLAGS=$(FLAGS)"
	@echo "	OPTIMIZATION=$(OPTIMIZATION)"
	@echo "	RES=$(RES)"
	@echo "	X=$(X)"
	@echo "	Y=$(Y)"
	@echo "	S=$(S)"
	@echo "	I=$(I)"
	@echo "	OUTPUT=$(OUTPUT)"
	@echo "	MARK=$(MARK)"
	@echo "	TRADITIONAL=$(TRADITIONAL)"
	@echo "	THREADED=$(THREADED)"
	@echo "	PROFILE=$(PROFILE)"
	@echo ""
	@echo "Compiler Call:"
	@echo "	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c dummy.cpp -o dummy.o"
	@echo "Binary Call:"
	@echo "	$(PROFILE) $(BINARY) $(ARGUMENTS)"

all:
	@$(MAKE) --no-print-directory $(BINARY)

time: PROFILE := /usr/bin/time
time:
	@$(MAKE) --no-print-directory PROFILE="$(PROFILE)" call

gprof: PROFILE :=
gprof: CXXFLAGS := $(CXXFLAGS) -g -pg -fno-omit-frame-pointer -fno-inline-functions -DNDEBUG
gprof:
	@$(MAKE) --no-print-directory PROFILE="$(PROFILE)" CXXFLAGS="$(CXXFLAGS)" call
	gprof $(BINARY) gmon.out > $(BUILD_DIR)/gprof.txt
	@rm -f gmon.out
	utils/gprof2svg -s $(BUILD_DIR)/gprof.txt -o $(BUILD_DIR)/gprof.svg
	xdg-open $(BUILD_DIR)/gprof.svg 2>/dev/null


callgrind: PROFILE := valgrind --tool=callgrind --callgrind-out-file=$(BUILD_DIR)/callgrind.out
callgrind: CXXFLAGS := $(CXXFLAGS) -g
callgrind:
	@$(MAKE) --no-print-directory PROFILE="$(PROFILE)" CXXFLAGS="$(CXXFLAGS)" call
	kcachegrind $(BUILD_DIR)/callgrind.out >/dev/null 2>&1 &

cachegrind: PROFILE := valgrind --tool=cachegrind --cachegrind-out-file=$(BUILD_DIR)/cachegrind.out
cachegrind: CXXFLAGS := $(CXXFLAGS) -g
cachegrind: .valgrind
	@$(MAKE) --no-print-directory PROFILE="$(PROFILE)" CXXFLAGS="$(CXXFLAGS)" call
	kcachegrind $(BUILD_DIR)/cachegrind.out >/dev/null 2>&1 &

clean:
	rm -f $(BUILD_DIR)/.flags_*
	rm -f $(BINARY)
	rm -f $(OBJECTS)

.valgrind .gprof:
	@$(MAKE) --no-print-directory CXXFLAGS="${CXXFLAGS$@}" $(BINARY)

mandelnav: $(BINARY) mandelNav.sh
	./mandelNav.sh xviewer

call: $(BINARY)
	$(PROFILE) $(BINARY) $(ARGUMENTS)

$(OUTPUTS): $(BINARY)
	@$(MAKE) --no-print-directory INPUT=input/$(patsubst %.png,%.obj,$(notdir $@)) OUTPUT=$@  call

$(OBJECTS) : %.o : %.cpp $(CHECK_FLAGS)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(CHECK_FLAGS):
	@$(MAKE) --no-print-directory clean
	@mkdir -p $(dir $@)
	@touch $@

$(BINARY): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LINKING) -o $@

