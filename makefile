
# !!! FIXME: Make this more robust. MUCH more robust.
# !!! FIXME: ...or at least comment the rest of these options...

ifeq ($(PANDORA),1)
  macosx := false
  CPUARCH := arm
  CC := g++
  LINKER := g++
  steamworks := false
  CFLAGS += -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp -ftree-vectorize -ffast-math -DPANDORA
else ifeq ($(ODROID),1)
  macosx := false
  CPUARCH := arm
  CC := g++
  LINKER := g++
  steamworks := false
  CFLAGS += -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -ftree-vectorize -ffast-math -DODROID
else ifeq ($(linux_x86),1)
  target := linux_x86
else
  target := macosx_x86
  steamworks := true
endif

BINDIR := ./bin
SRCDIR := .
#debug := true
debug := false

# ----------------------------------------------------- ... bleh.

ifeq ($(strip $(target)),linux_x86)
  macosx := false
  CPUARCH := x86
  CC := g++
  LINKER := g++
endif
ifeq ($(strip $(target)),macosx_x86)
  macosx := true
  CPUARCH := x86
  CC := g++
  LINKER := g++
endif

CLEANUP := $(wildcard *.exe) $(wildcard *.obj) \
          $(wildcard $(BINDIR)/*.exe) $(wildcard $(BINDIR)/*.obj) \
          $(wildcard *~) $(wildcard *.err) \
          $(wildcard .\#*) core

SRCS := \
	AnimThing.cpp \
	ball.cpp \
	barrel.cpp \
	bouy.cpp \
	character.cpp \
	chunk.cpp \
	deathWad.cpp \
	demon.cpp \
	dispenser.cpp \
	doofus.cpp \
	dude.cpp \
	explode.cpp \
	fire.cpp \
	fireball.cpp \
	firebomb.cpp \
	flag.cpp \
	flagbase.cpp \
	grenade.cpp \
	heatseeker.cpp \
	hood.cpp \
	item3d.cpp \
	mine.cpp \
	napalm.cpp \
	navnet.cpp \
	ostrich.cpp \
	person.cpp \
	PowerUp.cpp \
	pylon.cpp \
	rocket.cpp \
	sentry.cpp \
	SndRelay.cpp \
	SoundThing.cpp \
	thing.cpp \
	Thing3d.cpp \
	trigger.cpp \
	warp.cpp \
	weapon.cpp \
	alphablitforpostal.cpp \
	Anim3D.cpp \
	BufQ.cpp \
	bulletFest.cpp \
	camera.cpp \
	crawler.cpp \
	cutscene.cpp \
	encrypt.cpp \
	gameedit.cpp \
	GameSettings.cpp \
	grip.cpp \
	IdBank.cpp \
	InputSettings.cpp \
	InputSettingsDlg.cpp \
	keys.cpp \
	Log.cpp \
	logtab.cpp \
	MemFileFest.cpp \
	MenuSettings.cpp \
	MenuTrans.cpp \
	net.cpp \
	NetBrowse.cpp \
	NetClient.cpp \
	NetDlg.cpp \
	netmsgr.cpp \
	NetServer.cpp \
	organ.cpp \
	Personatorium.cpp \
	ProtoBSDIP.cpp \
	realm.cpp \
	scene.cpp \
	score.cpp \
	settings.cpp \
	smash.cpp \
	socket.cpp \
	StockPile.cpp \
	TexEdit.cpp \
	toolbar.cpp \
	TriggerRegions.cpp \
	update.cpp \
	yatime.cpp \
	aivars.cpp \
	band.cpp \
	credits.cpp \
	game.cpp \
	input.cpp \
	localize.cpp \
	main.cpp \
	menus.cpp \
	play.cpp \
	SampleMaster.cpp \
	title.cpp \
	RSPiX/Src/BLUE/unix/Bdebug.cpp \
	RSPiX/Src/BLUE/unix/Bjoy.cpp \
	RSPiX/Src/BLUE/unix/Bkey.cpp \
	RSPiX/Src/BLUE/unix/Bmain.cpp \
	RSPiX/Src/BLUE/unix/Bmouse.cpp \
	RSPiX/Src/BLUE/unix/Btime.cpp \
	RSPiX/Src/BLUE/unix/Bdisp.cpp \
	RSPiX/Src/BLUE/unix/Bsound.cpp \
	RSPiX/Src/GREEN/Hot/hot.cpp \
	RSPiX/Src/GREEN/Image/Image.cpp \
	RSPiX/Src/GREEN/Image/Imagecon.cpp \
	RSPiX/Src/GREEN/Image/ImageFile.cpp \
	RSPiX/Src/GREEN/InputEvent/InputEvent.cpp \
	RSPiX/Src/GREEN/Mix/mix.cpp \
	RSPiX/Src/GREEN/Mix/MixBuf.cpp \
	RSPiX/Src/GREEN/Image/pal.cpp \
	RSPiX/Src/GREEN/Image/PalFile.cpp \
	RSPiX/Src/GREEN/Sample/sample.cpp \
	RSPiX/Src/GREEN/Snd/snd.cpp \
	RSPiX/Src/GREEN/SndFx/SndFx.cpp \
	RSPiX/Src/GREEN/Task/task.cpp \
	RSPiX/Src/GREEN/3D/pipeline.cpp \
	RSPiX/Src/GREEN/3D/render.cpp \
	RSPiX/Src/GREEN/3D/types3d.cpp \
	RSPiX/Src/GREEN/3D/zbuffer.cpp \
	RSPiX/Src/GREEN/BLiT/alphablit.cpp \
	RSPiX/Src/GREEN/BLiT/BLIT.cpp \
	RSPiX/Src/GREEN/BLiT/BLITINIT.cpp \
	RSPiX/Src/GREEN/BLiT/BLiTT.cpp \
	RSPiX/Src/GREEN/BLiT/CFNT.cpp \
	RSPiX/Src/GREEN/BLiT/Fspr1.cpp \
	RSPiX/Src/GREEN/BLiT/FSPR8.cpp \
	RSPiX/Src/GREEN/BLiT/line.cpp \
	RSPiX/Src/GREEN/BLiT/mono.cpp \
	RSPiX/Src/GREEN/BLiT/Rotate96.cpp \
	RSPiX/Src/GREEN/BLiT/RPrint.cpp \
	RSPiX/Src/GREEN/BLiT/ScaleFlat.cpp \
	RSPiX/Src/ORANGE/GameLib/ANIMSPRT.cpp \
	RSPiX/Src/ORANGE/Attribute/attribute.cpp \
	RSPiX/Src/ORANGE/GUI/btn.cpp \
	RSPiX/Src/ORANGE/Channel/channel.cpp \
	RSPiX/Src/ORANGE/color/colormatch.cpp \
	RSPiX/Src/ORANGE/DirtRect/DirtRect.cpp \
	RSPiX/Src/ORANGE/color/dithermatch.cpp \
	RSPiX/Src/ORANGE/GUI/dlg.cpp \
	RSPiX/Src/ORANGE/GUI/edit.cpp \
	RSPiX/Src/ORANGE/File/file.cpp \
	RSPiX/Src/ORANGE/QuickMath/FixedPoint.cpp \
	RSPiX/Src/ORANGE/GUI/guiItem.cpp \
	RSPiX/Src/ORANGE/IFF/iff.cpp \
	RSPiX/Src/ORANGE/ImageTools/lasso.cpp \
	RSPiX/Src/ORANGE/Laymage/laymage.cpp \
	RSPiX/Src/ORANGE/GUI/ListBox.cpp \
	RSPiX/Src/ORANGE/GUI/ListContents.cpp \
	RSPiX/Src/ORANGE/Meter/meter.cpp \
	RSPiX/Src/ORANGE/MsgBox/MsgBox.cpp \
	RSPiX/Src/ORANGE/GUI/MultiBtn.cpp \
	RSPiX/Src/ORANGE/MultiGrid/MultiGrid.cpp \
	RSPiX/Src/ORANGE/MultiGrid/MultiGridIndirect.cpp \
	RSPiX/Src/ORANGE/GUI/ProcessGui.cpp \
	RSPiX/Src/ORANGE/Debug/profile.cpp \
	RSPiX/Src/ORANGE/GUI/PushBtn.cpp \
	RSPiX/Src/ORANGE/QuickMath/QuickMath.cpp \
	RSPiX/Src/ORANGE/GameLib/Region.cpp \
	RSPiX/Src/ORANGE/RString/rstring.cpp \
	RSPiX/Src/ORANGE/GUI/scrollbar.cpp \
	RSPiX/Src/ORANGE/GameLib/SHAPES.cpp \
	RSPiX/Src/ORANGE/Parse/SimpleBatch.cpp \
	RSPiX/Src/ORANGE/GameLib/SPRITE.cpp \
	RSPiX/Src/ORANGE/str/str.cpp \
	RSPiX/Src/ORANGE/GUI/txt.cpp \
	RSPiX/Src/CYAN/Unix/uDialog.cpp \
	RSPiX/Src/CYAN/Unix/uColors.cpp \
	RSPiX/Src/CYAN/Unix/uPath.cpp \
	WishPiX/Menu/menu.cpp \
	WishPiX/Prefs/prefline.cpp \
	WishPiX/Prefs/prefs.cpp \
	WishPiX/ResourceManager/resmgr.cpp \
	WishPiX/Spry/spry.cpp

    # wtf is THIS?!
	#RSPiX/Src/ORANGE/MTask/mtask.cpp \

CLIENTEXE := $(BINDIR)/postal1-bin
OBJS0 := $(SRCS:.s=.o)
OBJS1 := $(OBJS0:.c=.o)
OBJS2 := $(OBJS1:.cpp=.o)
OBJS3 := $(OBJS2:.nasm=.o)
OBJS4 := $(OBJS3:.s=.o)
OBJS := $(foreach f,$(OBJS4),$(BINDIR)/$(f))
SRCS := $(foreach f,$(SRCS),$(SRCDIR)/$(f))

# !!! FIXME: Get -Wall in here, some day.
CFLAGS += -fsigned-char -g -DPLATFORM_UNIX -w

ifeq ($(strip $(macosx)),true)
  CFLAGS += -DPLATFORM_MACOSX
endif

# defines the game needs...
CFLAGS += -DLOCALE=US -DTARGET=POSTAL_2015

# includes ...
CFLAGS += -I$(SRCDIR)
CFLAGS += -I$(SRCDIR)/SDL2/include
CFLAGS += -I$(SRCDIR)/RSPiX
CFLAGS += -I$(SRCDIR)/RSPiX/Inc
CFLAGS += -I$(SRCDIR)/RSPiX/Src
CFLAGS += -I$(SRCDIR)/RSPiX/Src/BLUE
CFLAGS += -I$(SRCDIR)/RSPiX/Src/BLUE/unix
CFLAGS += -I$(SRCDIR)/RSPiX/Src/CYAN
CFLAGS += -I$(SRCDIR)/RSPiX/Src/CYAN/Unix
CFLAGS += -I$(SRCDIR)/RSPiX/Src/ORANGE
CFLAGS += -I$(SRCDIR)/RSPiX/Src/ORANGE/CDT
CFLAGS += -I$(SRCDIR)/RSPiX/Src/ORANGE/GameLib
CFLAGS += -I$(SRCDIR)/RSPiX/Src/ORANGE/File
CFLAGS += -I$(SRCDIR)/RSPiX/Src/ORANGE/Multigrid
CFLAGS += -I$(SRCDIR)/RSPiX/Src/GREEN/Image
CFLAGS += -I$(SRCDIR)/WishPiX
CFLAGS += -I$(SRCDIR)/WishPiX/Spry

ifeq ($(strip $(expiring_beta)),true)
  CFLAGS += -DBETAEXPIRE=$(shell date +%s)
endif

ifeq ($(strip $(debug)),true)
  CFLAGS += -DDEBUG -D_DEBUG -O0
else
  OPTFLAG := -O3
  CFLAGS += -DNDEBUG -D_NDEBUG $(OPTFLAG)
endif

ifeq ($(strip $(macosx)),true)
  CFLAGS += -arch i386 -mmacosx-version-min=10.5
  LDFLAGS += -arch i386 -mmacosx-version-min=10.5
  LDFLAGS += -framework CoreFoundation -framework Cocoa
  LIBS += SDL2/libs/macosx/libSDL2-2.0.0.dylib
  STEAMLDFLAGS += steamworks/sdk/redistributable_bin/osx32/libsteam_api.dylib
else
  ifeq ($(CPUARCH),arm)
    LIBS += -lSDL2
  else
    LIBS += SDL2/libs/linux-x86/libSDL2-2.0.so.0
    LDFLAGS += -Wl,-rpath,\$$ORIGIN
    STEAMLDFLAGS += steamworks/sdk/redistributable_bin/linux32/libsteam_api.so
 endif
endif

ifeq ($(strip $(steamworks)),true)
  CFLAGS += -DWITH_STEAMWORKS=1 -Isteamworks/sdk/public
  LDFLAGS += $(STEAMLDFLAGS)
endif

CFLAGS += -DALLOW_TWINSTICK

.PHONY: all bindir

all: $(CLIENTEXE)


$(BINDIR)/%.o: $(SRCDIR)/%.s
	$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<

$(BINDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

$(BINDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(BINDIR)/%.a: $(SRCDIR)/%.a
	cp $< $@
	ranlib $@

$(CLIENTEXE): $(BINDIR) $(OBJS) $(LIBS)
	$(LINKER) -o $(CLIENTEXE) $(OBJS) $(LDFLAGS) $(LIBS)

$(BINDIR) :
	$(MAKE) bindir

bindir :
	mkdir -p $(BINDIR)
	mkdir -p $(BINDIR)/RSPiX/Src/BLUE/unix
	mkdir -p $(BINDIR)/RSPiX/Src/GREEN/Hot
	mkdir -p $(BINDIR)/RSPiX/Src/GREEN/Image
	mkdir -p $(BINDIR)/RSPiX/Src/GREEN/InputEvent
	mkdir -p $(BINDIR)/RSPiX/Src/GREEN/Mix
	mkdir -p $(BINDIR)/RSPiX/Src/GREEN/Sample
	mkdir -p $(BINDIR)/RSPiX/Src/GREEN/Snd
	mkdir -p $(BINDIR)/RSPiX/Src/GREEN/SndFx
	mkdir -p $(BINDIR)/RSPiX/Src/GREEN/Task
	mkdir -p $(BINDIR)/RSPiX/Src/GREEN/3D
	mkdir -p $(BINDIR)/RSPiX/Src/GREEN/BLiT
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/GameLib
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/Attribute
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/GUI
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/Channel
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/color
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/DirtRect
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/File
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/QuickMath
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/IFF
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/ImageTools
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/Laymage
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/Meter
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/MsgBox
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/MTask
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/MultiGrid
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/Debug
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/RString
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/Parse
	mkdir -p $(BINDIR)/RSPiX/Src/ORANGE/str
	mkdir -p $(BINDIR)/RSPiX/Src/CYAN/Unix
	mkdir -p $(BINDIR)/WishPiX/Menu
	mkdir -p $(BINDIR)/WishPiX/Prefs
	mkdir -p $(BINDIR)/WishPiX/ResourceManager
	mkdir -p $(BINDIR)/WishPiX/Spry
	mkdir -p $(BINDIR)/libs

distclean: clean

clean:
	rm -f $(CLEANUP)
	rm -rf $(BINDIR)
	#rm -f $(SRCDIR)/parser/y.tab.c
	#rm -f $(SRCDIR)/parser/lex.yy.c

# end of Makefile ...
