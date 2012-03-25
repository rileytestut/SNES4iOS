VERSION=5.1
COPT = -F/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS${VERSION}.sdk/System/Library/Frameworks -F/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS${VERSION}.sdk/System/Library/PrivateFrameworks -I../../ -I../../Classes/ -I/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/usr/lib/clang/3.0/include -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS${VERSION}.sdk  -L/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS${VERSION}.sdk/usr/lib -I/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS${VERSION}.sdk/usr/include/
COPT += -march=armv7-a -miphoneos-version-min=${VERSION} -I. -O0 -D__IPHONE__ -D_SNESPPC -DASM_SPC700 -DZLIB -DUNZIP_SUPPORT -DOLD_COLOUR_BLENDING -DUSE_SA1 -DSUPER_FX -DLSB_FIRST -DCPU_SHUTDOWN -DVAR_CYCLES
COPT += -fnested-functions -funsigned-char -ffast-math -ftemplate-depth-36 -mstructure-size-boundary=32 -falign-functions=32 -falign-loops -falign-labels -falign-jumps -finline -finline-functions -fno-builtin -fno-common -fomit-frame-pointer -fpeel-loops -fstrength-reduce -funroll-loops -fstrict-aliasing -arch armv7
GCC = /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/clang
GXX = /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/clang++
STRIP = /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/strip
#-DSPC700_SHUTDOWN
#-DRC_OPTIMIZED 
# SNES stuff (c-based)
# cpuops.o sa1cpu.o 
OBJS = apu.o c4.o c4emu.o cheats.o cheats2.o clip.o cpu.o cpuexec.o data.o 
OBJS += dma.o dsp1.o fxemu.o fxinst.o gfx.o globals.o input.o ioapi.o loadzip.o memmap.o menu.o ppu.o  
OBJS += sa1.o sdd1.o sdd1emu.o snapshot.o soundux.o spc700.o srtc.o tile.o unzip.o os9x_asm_cpu.o os9x_65c816.o spc700a.o 
#
# ASM CPU Core, ripped from Yoyo's OpenSnes9X
#
#OBJS += os9x_65c816.o 
# os9x_asm_cpu.o spc700a.o
#
# and some asm from LJP...
#
# OBJS += m3d_func.o
#
# Dave's minimal SDK
#
OBJS += iphone_sdk.o iphone_menutile.o iphone_highlightbar.o iphone_menu_header.o 
OBJS += ../../main.o ../../Classes/EmulationViewController.o ../../Classes/ControlPadConnectViewController.o   ../../Classes/ControlPadManager.o ../../Classes/RomDetailViewController.o ../../Classes/RomSelectionViewController.o ../../Classes/ScreenView.o ../../Classes/ScreenLayer.o ../../Classes/SaveStateSelectionViewController.o ../../Classes/SettingsViewController.o ../../Classes/SNES4iOSAppDelegate.o ../../Classes/WebBrowserViewController.o ../../Classes/SNESControllerAppDelegate.o ../../Classes/SessionController.o ../../Classes/SNESControllerViewController.o


#
# and the glue code that sticks it all together :)
#
OBJS += main.o

# Inopia's menu system, hacked for the GP2X under rlyeh's sdk
PRELIBS = -multiply_defined suppress -lobjc -fobjc-exceptions \
	        -lpthread \
          -framework CoreFoundation \
          -framework Foundation \
          -framework UIKit \
          -framework QuartzCore \
          -framework CoreGraphics \
          -framework IOSurface \
          -framework CoreLocation \
          -framework AudioToolbox \
          -framework GraphicsServices \
          -framework OpenGLES \
          -framework AddressBook -lsqlite3 -framework SystemConfiguration \
          -lz \
          -framework WebKit \
          -framework GameKit \
          -allow_stack_execute

all: pocketsnes
clean: tidy

.c.o:
	$(GCC) $(COPT) -c $< -o $@

.cpp.o:
	$(GXX) $(COPT) -c $< -o $@

%.o: %.m
	$(GCC) ${COPT} -c $< -o $@

%.o: %.S
	$(GCC) ${COPT} -c $< -o $@

%.o: %.s
	$(GCC) ${COPT} -c $< -o $@

main.o:
	$(GXX) $(COPT) -c main.mm -o $@	

# make seems to lowercase the extensions, so files with '.S' end up being passed to the compiler as '.s', which means thousands of errors.
# this is a small workaround. 

#spc700a.o: spc700a.s
#	$(GCC) $(COPT) -c $< -o $@

#os9x_65c816.o: os9x_65c816.s
#	$(GCC) $(COPT) -c $< -o $@

#osnes9xgp_asmfunc.o: osnes9xgp_asmfunc.s
#	$(GCC) $(COPT) -c $< -o $@

#m3d_func.o: m3d_func.S
#	$(GCC) $(COPT) -c $< -o $@

#spc_decode.o: spc_decode.s
#	$(GCC) $(COPT) -c $< -o $@

#memset.o: memset.s
#	$(GCC) $(COPT) -c $< -o $@

#memcpy.o: memcpy.s
#	$(GCC) $(COPT) -c $< -o $@

#dspMixer.o: dspMixer.s
#	$(GCC) $(COPT) -c $< -o $@

pocketsnesd: $(OBJS)
	$(GXX) $(COPT) $(OBJS) $(PRELIBS) -o $@
	ar rcs libpocketsnes.a $(OBJS)

pocketsnes: pocketsnesd
	$(STRIP) pocketsnesd -o snes4iphone
	
tidy:
	rm -f $(OBJS)
	rm -f pocketsnesd
	rm -f snes4iphone
	rm -f pocketsnes.a
