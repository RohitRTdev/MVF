RM := rm -rf
CXX := g++
CXXFLAGS := -O3 -Wall -MMD -MP -std=c++20 $(shell pkg-config --with-path=$(PKG_CONFIG_PATH) --cflags gtkmm-4.0)
LIBS := $(shell pkg-config --with-path=$(PKG_CONFIG_PATH) --libs gtkmm-4.0) -lglew32 -lopengl32

ROOT_DIR := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/..)
SRC_DIR := $(ROOT_DIR)/src
BUILD_DIR := $(ROOT_DIR)/build
BIN := $(ROOT_DIR)/mvf

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

ifeq ($(OS),Windows_NT)
    RES := $(BUILD_DIR)/icon_res.o
else
    RES :=
endif

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS) $(RES)
	@echo "Creating executable $@"
	$(CXX) $(OBJS) -mwindows $(RES) -o $@ $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

ifeq ($(OS),Windows_NT)
$(RES): $(ROOT_DIR)/assets/icon.rc | $(BUILD_DIR)
	@echo "Compiling Windows icon resource"
	@windres $(ROOT_DIR)/assets/icon.rc -O coff -o $@
endif

-include $(DEPS)

clean:
	@echo "Cleaning files..."
	@$(RM) $(BIN) $(BUILD_DIR)
