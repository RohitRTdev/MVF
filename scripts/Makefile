RM := rm -rf
CXX := g++
DEBUGFLAGS := -O0 -g -DMVF_DEBUG
RLSFLAGS := -O3
CXXFLAGS := -Wall -MMD -MP -std=c++20 $(shell pkg-config --with-path=$(PKG_CONFIG_PATH) --cflags gtkmm-4.0)
LIBS := $(shell pkg-config --with-path=$(PKG_CONFIG_PATH) --libs gtkmm-4.0) -lglew32 -lopengl32

SRC_DIR := src
BUILD_DIR := build
BIN := mvf

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

ifeq ($(OS),Windows_NT)
    RES := $(BUILD_DIR)/icon_res.o
else
    RES :=
endif

ifeq ($(CONFIG),Debug) 
	CXXFLAGS := $(DEBUGFLAGS) $(CXXFLAGS)
else
	CXXFLAGS := $(RLSFLAGS) $(CXXFLAGS)
	LDFLAGS := -mwindows
endif

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS) $(RES)
	@echo "Creating executable $@"
	$(CXX) $(LDFLAGS) $(OBJS) $(RES) -o $@ $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

ifeq ($(OS),Windows_NT)
$(RES): assets/icon.rc | $(BUILD_DIR)
	@echo "Compiling Windows icon resource"
	@windres assets/icon.rc -O coff -o $@
endif

-include $(DEPS)

clean:
	@echo "Cleaning files..."
	@$(RM) $(BIN) $(BUILD_DIR)
