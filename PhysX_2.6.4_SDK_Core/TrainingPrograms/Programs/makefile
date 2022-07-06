################################################################################
# Makefile for setting environment flags
#
# This Makefile is included from every Lesson. It is the common base of them all
################################################################################

default: ../../../../shared_source all

../../../../shared_source:
	cd ../../../.. && ln -s "Shared Source" shared_source

CC	:= gcc
CXX := g++

NOVODEX_DIR = ../../../../../..
LESSON_DIR=../../../..
LESSON_ROOT = ../..
SDK_DIR = $(NOVODEX_DIR)/SDKs
F_DIR = $(SDK_DIR)/Foundation
F_LINUX_DIR = $(F_DIR)/compiler/linux/$(BUILD)
P_DIR = $(SDK_DIR)/Physics
P_LINUX_DIR = $(P_DIR)/compiler/linux/$(BUILD)

include $(NOVODEX_DIR)/SDKs/compiler/linux/Makefile.builds

CFLAGS  += -I$(LESSON_DIR)/shared_source

#PFLAGS = -lc

LIB_DIR := $(SDK_DIR)/lib/linux

TFLAGS := -L/usr/X11R6/lib
TFLAGS += -L/usr/local/lib
TFLAGS += -lm
TFLAGS += -lX11
TFLAGS += -lXi
TFLAGS += -lXmu
TFLAGS += -lGL
TFLAGS += -lGLU
TFLAGS += -lglut
TFLAGS += -lpthread
#TFLAGS += /usr/share/glut-3.7/lib/glut/libglut.a
#TFLAGS += -lIL
#TFLAGS += -lILU
#TFLAGS += -lILUT
#TFLAGS += -lm

ifeq ($(BUILD),debug)
TFLAGS += $(LIB_DIR)/libNxCookingDebug-2.2.a
TFLAGS += $(LIB_DIR)/libNxPhysicsDebug-2.2.a
else
TFLAGS += $(LIB_DIR)/libNxCooking-2.2.a
TFLAGS += $(LIB_DIR)/libNxPhysics-2.2.a
endif


flags:
	@echo CFLAGS:
	@echo $(CFLAGS)

$(BUILD)/%.o : %.cpp
	@echo "Compiling `basename $<` ..."
	@$(CXX) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o : %.c
	@echo "Compiling `basename $<` ..."
	@$(CC) $(CFLAGS) -c $< -o $@

all: $(BUILD) $(BUILD)/$(TARGET)

################################################################################
# Generate full list of C++ files to compile
################################################################################
C_DIR := $(LESSON_ROOT)/src
D_DIR := $(LESSON_ROOT)/Src
E_DIR := $(LESSON_ROOT)/source
D_LIST := $(wildcard $(C_DIR)/*.cpp)
D_LIST += $(wildcard $(D_DIR)/*.cpp)
D_LIST += $(wildcard $(E_DIR)/*.cpp)
D_LIST += $(ADDITIONALS:%.cpp=$(LESSON_DIR)/shared_source/%.cpp)

vpath %.cpp $(D_DIR) $(C_DIR) $(E_DIR) $(LESSON_DIR)/shared_source

################################################################################
# Generate list of target object files
################################################################################
D_SRC := $(notdir $(D_LIST))
D_OBJ := $(D_SRC:%.cpp=$(BUILD)/%.o)

$(BUILD)/$(TARGET) : $(D_OBJ) $(NOVODEX_DIR)/Bin/linux/$(BUILD)
	@echo linking $(TARGET)
	@g++ -o $@ $(D_OBJ) $(TFLAGS)
	@echo stripping...
	@$(STRIP) $@
	@echo copying to bin/linux/$(BUILD)
	@cp $@ $(NOVODEX_DIR)/Bin/linux/$(BUILD)

$(NOVODEX_DIR)/Bin/linux:
	@mkdir $(NOVODEX_DIR)/Bin/linux >/dev/null || true

$(NOVODEX_DIR)/Bin/linux/$(BUILD): $(NOVODEX_DIR)/Bin/linux
	@mkdir $(NOVODEX_DIR)/Bin/linux/$(BUILD) 2>/dev/null || true
	
	
################################################################################
# Auto-dependency rules
################################################################################

#ifneq (.deps,)
    -include .deps
#endif

clean:
	@echo cleaning all builds
	@for i in `find . -type d | grep \/`; do rm -rf $$i; done
	@echo cleaning .deps*
	@rm -f .deps*

