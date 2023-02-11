CC = gcc
MKDIR_P ?= mkdir -p
RM = rm

# define any compile-time flags
CFLAGS = -std=c99 -O3 -g -no-pie

EMCFLAGS = -std=c99 -O3 -g

EMSETTINGS = --preload-file webdata@ -s USE_SDL=2 -s TOTAL_STACK=32mb -s INITIAL_MEMORY=1gb -s MAXIMUM_MEMORY=2gb -s ALLOW_MEMORY_GROWTH=1 -s EMULATE_FUNCTION_POINTER_CASTS=1 -s GLOBAL_BASE=2048

# -s TOTAL_STACK=20mb -s INITIAL_MEMORY=10mb -s ALLOW_MEMORY_GROWTH=1 -s EMULATE_FUNCTION_POINTER_CASTS=0

LIBS = -lSDL2 -lm

DEFINES = -DPC -DPLATFORM_PC -DPPM_SUPPORT_ENABLED -DSYSCALL_WRAPPERS=21 -DSVS_RND_FUNCTION="rand()"

LANG = -DLANG_CZ

BUILD_DIR ?= ./build

SRCS := $(shell find "SDA_OS" -name "*.c")
SRCS += sda-sdl.c sda_fs_pc.c sda_api_stubs.c
OBJS := $(addprefix $(BUILD_DIR),$(addprefix /, $(addsuffix .o,$(basename $(SRCS)))))

all: sim_cz sim_en docs

# generate docs
docs:
	@echo Generating wrapper docs
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/os_wrapper/sda_os_wrapper.c   | sed 's .\{2\}  ' > SDA_OS/Docs/sda_main.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/os_wrapper/sda_os_gui.c       | sed 's .\{2\}  ' > SDA_OS/Docs/sda_os_gui.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/os_wrapper/sda_os_sound.c     | sed 's .\{2\}  ' > SDA_OS/Docs/sda_os_sound.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/os_wrapper/sda_os_widgets.c   | sed 's .\{2\}  ' > SDA_OS/Docs/sda_os_widgets.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/os_wrapper/sda_os_crypto.c    | sed 's .\{2\}  ' > SDA_OS/Docs/sda_os_crypto.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/sda_os_hw_wrapper.c | sed 's .\{2\}  ' > SDA_OS/Docs/sda_hw.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/sda_os_hw_comm.c    | sed 's .\{2\}  ' >> SDA_OS/Docs/sda_hw.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/sda_files.c         | sed 's .\{2\}  ' > SDA_OS/Docs/sda_files.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/sda_files_ini_csv.c | sed 's .\{2\}  ' >> SDA_OS/Docs/sda_files.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/sda_overlays.c      | sed 's .\{2\}  ' > SDA_OS/Docs/sda_overlays.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/sda_time.c          | sed 's .\{2\}  ' > SDA_OS/Docs/sda_time.md)
	$(shell grep -o "#\!.*" SDA_OS/GR2_WRAP/svs_gr2_wrap.c      | sed 's .\{2\}  ' > SDA_OS/Docs/sda_gr2_wrapper.md)
	$(shell grep -o "#\!.*" SDA_OS/GR2_WRAP/sda_gr2_inits.c     | sed 's .\{2\}  ' >> SDA_OS/Docs/sda_gr2_wrapper.md)
	$(shell grep -o "#\!.*" SDA_OS/GR2_WRAP/sda_gr2_get_set.c   | sed 's .\{2\}  ' >> SDA_OS/Docs/sda_gr2_wrapper.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/wrap_directS.c      | sed 's .\{2\}  ' > SDA_OS/Docs/sda_directS.md)

# fancy quick build
$(BUILD_DIR)/%.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) $(DEFINES) $(LANG) $(LIBS) -c $< -o $@

sim_cz: $(OBJS)
	$(CC) $(OBJS) -o BIN/SDA_OS_sim_cz $(LIBS) -no-pie
	#Done

# since quick english build is not needed
sim_en:
	$(CC) $(CFLAGS) $(SRCS) $(LIBS) $(DEFINES) -DLANG_EN -o BIN/SDA_OS_sim_eng 

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

sim_emcc:
	emcc $(SRCS) $(EMCFLAGS) $(LIBS) $(DEFINES) -DLANG_EN -DWEBTARGET -o binweb/SDA_OS.html $(EMSETTINGS)

sim_emcc_cz:
	emcc $(SRCS) $(EMCFLAGS) $(LIBS) $(DEFINES) -DLANG_EN -DWEBTARGET -DTOKEN_CACHE_DISABLED -o binweb/SDA_OS.html $(EMSETTINGS)

scan-build:
	scan-build $(CC) $(CFLAGS) $(SRCS) $(LIBS) $(DEFINES) -DLANG_CZ -o BIN/SDA_OS_sim_cz

#view hotfix PYTHONPATH=/usr/share/clang/scan-view-3.8/share:$PYTHONPATH scan-view

update:
	git submodule update --init --recursive --force --remote

autoupdate-apps:
	find BIN/APPS/ | grep svs | grep -Ev "backup" | awk '{print "sda_app_updater "$$0"\n"}' | bash
