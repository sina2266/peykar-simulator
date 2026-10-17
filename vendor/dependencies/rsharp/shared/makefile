BUILDTYPE ?= Release
TGTPLATFORM=x64
RUNTIMECONFIG=RSharp.runtimeconfig
OBJ_DIR := obj
OUT_DIR := out
SRC_DIR := .
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
INSTDIR= ../inst
UNAME := $(shell uname)
UNAME_ARCH := $(shell uname -m)

ifeq ($(UNAME_ARCH), x86_64)
  ARCH=x64
else
  ARCH=arm64
endif

ifeq ($(UNAME), Darwin)
	OUTPUT := rSharp.mac.$(ARCH).so
	RUNTIME := osx
else
	OUTPUT := rSharp.linux.so
endif

# only nuget supports packages.config
NUGET_CMD=nuget
DOTNET_CMD=dotnet

ifeq "$(BUILDTYPE)" "Release"
BuildConfiguration=UnixRelease
else
BuildConfiguration=UnixDebug
endif  ## ifeq "$(BUILDTYPE)" "Release"


MODE=Rebuild

SLNPROPERTIES=/p:Configuration=$(BuildConfiguration) /p:Platform="$(TGTPLATFORM)" /nologo /consoleloggerparameters:ErrorsOnly /verbosity:minimal

RM = 		rm -f

# .PHONY: 	all clean

all:		printarch instdir rSharpLib $(OUT_DIR)/$(OUTPUT) rSharpInstrSharpUX

clean:
	${RM} $(OUT_DIR)/* $(OBJ_DIR)/*

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	-@mkdir -p $(OBJ_DIR) 2>/dev/null
ifeq ($(UNAME), Darwin)
	$(CXX) -std=c++11 -Ipackages/Microsoft.NETCore.App.Host.osx-$(ARCH).8.0.2/runtimes/osx-$(ARCH)/native $(ALL_CPPFLAGS) -I/Library/Frameworks/R.framework/Resources/include -I/usr/share/R/include/ $(ALL_CFLAGS) -fPIC -c $<  -o $@  -lsupc++
else
	$(CXX) -std=c++11 -Ipackages/Microsoft.NETCore.App.Host.linux-x64.8.0.2/runtimes/linux-x64/native $(ALL_CPPFLAGS) -I/usr/share/R/include/ $(ALL_CFLAGS) -fPIC -c $< -o $@ -lsupc++
endif

$(OUT_DIR)/$(OUTPUT):  $(OBJ_FILES)
	-@mkdir -p $(OUT_DIR) 2>/dev/null
ifeq ($(UNAME), Darwin)
	$(CXX) -std=c++11 -shared -Wl,-undefined,dynamic_lookup -o $@  $^ $(ALL_LIBS) ./packages/Microsoft.NETCore.App.Host.osx-$(ARCH).8.0.2/runtimes/osx-$(ARCH)/native/libnethost.a
else
	$(CXX) -std=c++11 -shared -Wl,-z,relro -o $@ $^ $(ALL_LIBS) -L./packages/Microsoft.NETCore.App.Host.linux-x64.8.0.2/runtimes/linux-x64/native -l:libnethost.a
endif

printarch:
	-@echo **Variable information only for diagnosis purposes**
	-@echo $(UNAME)
	-@echo $(UNAME_ARCH)
	-@echo $(OUTPUT)
	-@echo CC=$(CC)
	-@echo CXX=$(CXX)
	-@echo **END Variable**

instdir:
	-@rm -rf $(INSTDIR)/lib$(R_ARCH)
	-@mkdir -p $(INSTDIR) 2>/dev/null
	-@mkdir -p $(INSTDIR)/lib$(R_ARCH) 2>/dev/null

rSharpInstrSharpUX: $(OUT_DIR)/$(OUTPUT)
ifeq ($(UNAME), Darwin)
	cp -p $< $(INSTDIR)/lib$(R_ARCH)
else
	cp -u -p $< $(INSTDIR)/lib$(R_ARCH)
endif

rSharpLib: rSharpLibComp
ifeq ($(UNAME), Darwin)
	if [ -e symbols.rds ] ; then cp -p symbols.rds $(INSTDIR)/lib$(R_ARCH) ; fi
	-cp -p ./$(RUNTIMECONFIG).json $(INSTDIR)/lib/
else
	if [ -e symbols.rds ] ; then cp -u -p symbols.rds $(INSTDIR)/lib$(R_ARCH) ; fi
	-cp -u -p ./$(RUNTIMECONFIG).json $(INSTDIR)/lib/
endif

rSharpNugetRestore:
	-$(NUGET_CMD) restore packages.config -PackagesDirectory packages
	-$(DOTNET_CMD) restore

rSharpLibComp: rSharpNugetRestore
	"$(DOTNET_CMD)" build /t:$(MODE) $(SLNPROPERTIES)
