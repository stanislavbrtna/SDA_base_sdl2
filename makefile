CC = gcc

# define any compile-time flags
CFLAGS = -std=c99 -O3

EMCFLAGS = -std=c99 -O0 --js-opts 0

EMSETTINGS = --preload-file webdata@ -s USE_SDL=2 -s ALLOW_MEMORY_GROWTH=1

LIBS = -lSDL2 -lm

DEFINES = -DPC -DPLATFORM_PC -DPPM_SUPPORT_ENABLED

SRCS = sda-sdl.c sda_fs_pc.c SDA_OS/sda_main.c SDA_OS/GR2/*.c SDA_OS/sda_system/*.c SDA_OS/sda_gui/*.c SDA_OS/sda_util/*.c SDA_OS/SVP_SCREENS/*.c SDA_OS/SVS/*.c SDA_OS/SVS_WRAP/*.c SDA_OS/GR2_WRAP/*.c

MAIN = SDA_os

all: sim_cz sim_en docs

docs:
	@echo Generating wrapper docs
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/sda_os_wrapper.c | sed 's .\{2\}  ' > SDA_OS/Docs/sda_main.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/sda_files.c      | sed 's .\{2\}  ' > SDA_OS/Docs/sda_files.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/sda_overlays.c   | sed 's .\{2\}  ' > SDA_OS/Docs/sda_overlays.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/sda_time.c       | sed 's .\{2\}  ' > SDA_OS/Docs/sda_time.md)
	$(shell grep -o "#\!.*" SDA_OS/GR2_WRAP/svs_gr2_wrap.c   | sed 's .\{2\}  ' > SDA_OS/Docs/sda_gr2_wrapper.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/wrap_directS.c   | sed 's .\{2\}  ' > SDA_OS/Docs/sda_directS.md)

sim_cz:
	$(CC) $(CFLAGS) $(SRCS) $(LIBS) $(DEFINES) -DLANG_CZ -o BIN/SDA_OS_sim_cz

sim_en:
	$(CC) $(CFLAGS) $(SRCS) $(LIBS) $(DEFINES) -DLANG_EN -o BIN/SDA_OS_sim_eng

sim_emcc:
	emcc $(SRCS) $(EMCFLAGS) $(LIBS) $(DEFINES) -DLANG_EN -DWEBTARGET -DTOKEN_CACHE_DISABLED -o binweb/SDA_OS.html $(EMSETTINGS)

sim_emcc_cz:
	emcc $(SRCS) $(EMCFLAGS) $(LIBS) $(DEFINES) -DLANG_EN -DWEBTARGET -DTOKEN_CACHE_DISABLED -o binweb/SDA_OS.html $(EMSETTINGS)

scan-build:
	scan-build $(CC) $(CFLAGS) $(SRCS) $(LIBS) $(DEFINES) -DLANG_CZ -o BIN/SDA_OS_sim_cz

#view hotfix PYTHONPATH=/usr/share/clang/scan-view-3.8/share:$PYTHONPATH scan-view

update:
	git submodule update --init --recursive --force --remote
