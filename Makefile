TARGET      := 3ds_github
BUILD       := build
SOURCES     := src
INCLUDES    := include

CXX         := /opt/devkitpro/devkitARM/bin/arm-none-eabi-g++
CC          := /opt/devkitpro/devkitARM/bin/arm-none-eabi-gcc

ARCH        := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CXXFLAGS    := -g -Wall -O2 -mword-relocations -ffunction-sections \
							 -fno-rtti -fno-exceptions -std=gnu++23 \
							 $(ARCH) -D__3DS__
LDFLAGS     := -specs=$(DEVKITARM)/arm-none-eabi/lib/3dsx.specs -g $(ARCH)

LIBS        := -lcitro2d -lcitro3d -lctru -lm
LIBDIRS     := $(DEVKITPRO)/libctru $(DEVKITPRO)/portlibs/3ds

INCLUDE     := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
							 $(foreach dir,$(LIBDIRS),-I$(dir)/include)
LIBPATHS    := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

CXXFILES    := $(wildcard $(SOURCES)/*.cpp)
OFILES      := $(CXXFILES:$(SOURCES)/%.cpp=$(BUILD)/%.o)

SMDH        := $(BUILD)/$(TARGET).smdh
APP_TITLE   := 3DS GitHub
APP_DESC    := Browse and interact with GitHub from your 3DS
APP_AUTHOR  := MeiMei
ICON        := icon.png

all: $(BUILD)/cacert.h $(BUILD)/$(TARGET).3dsx $(BUILD)/$(TARGET).cia
	@echo "I suggest using azahar to emulate the 3DS if you can't use a real one!"

$(BUILD)/$(TARGET).elf: $(OFILES)
	$(CXX) $(LDFLAGS) $^ $(LIBPATHS) $(LIBS) -o $@

$(SMDH): $(ICON) | $(BUILD)
	smdhtool --create "$(APP_TITLE)" "$(APP_DESC)" "$(APP_AUTHOR)" $(ICON) $@

$(BUILD)/$(TARGET).3dsx: $(BUILD)/$(TARGET).elf $(SMDH)
	3dsxtool $< $@ --smdh=$(SMDH)

$(BUILD)/$(TARGET).cia: $(BUILD)/$(TARGET).elf $(SMDH) | $(BUILD)
	makerom -f cia -o $@ -target t -elf $< -rsf gh.rsf -icon $(SMDH)

$(BUILD)/%.o: $(SOURCES)/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)

.PHONY: all clean
