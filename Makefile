PRJ=$(OO_SDK_HOME)
SETTINGS=$(OO_SDK_HOME)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
APP_NAME=doccat

OUT_APP_INC = $(OUT_INC)/$(APP_NAME)
OUT_APP_GEN = $(OUT_MISC)/$(APP_NAME)
OUT_APP_OBJ=$(OUT_OBJ)/$(APP_NAME)
ETC = /usr/include/libreoffice
ETC_LIB = /usr/lib/libreoffice/sdk/lib

CXXFILES = doccat.cxx

OBJFILES = $(patsubst %.cxx,$(OUT_SLO_COMP)/%.$(OBJ_EXT),$(CXXFILES))

# Targets
.PHONY: ALL
ALL : \
	doccat

include $(SETTINGS)/stdtarget.mk

$(OUT_APP_OBJ)/%.$(OBJ_EXT) : %.cxx $(SDKTYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) -g $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_APP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) -I$(ETC) $<

$(OUT_BIN)/_$(APP_NAME)$(EXE_EXT) : $(OUT_APP_OBJ)/$(APP_NAME).$(OBJ_EXT)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_APP_GEN))
ifeq "$(OS)" "WIN"
	$(LINK) $(EXE_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_APP_GEN)/$(basename $(@F)).map \
	  $< $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB)
else
	$(LINK) $(EXE_LINK_FLAGS) $(LINK_LIBS) -o $@ $< \
	  $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STDC++LIB) -L$(ETC_LIB)
ifeq "$(OS)" "MACOSX"
	$(INSTALL_NAME_URELIBS_BIN)  $@
endif
endif

$(OUT_BIN)/$(APP_NAME)$(EXE_EXT) : $(OUT_BIN)/_$(APP_NAME)$(EXE_EXT)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(COPY) $(subst /,$(PS),$(BIN_DIR)/unoapploader$(EXE_EXT)) $(subst /,$(PS),$@)
# workaround for touch problem under Windows with full qualified paths
	make -t $@

doccat : $(OUT_BIN)/$(APP_NAME)$(EXE_EXT)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute the example!
	@echo -
	@echo $(MAKE) doccat.run
	@echo --------------------------------------------------------------------------------	

%.run: $(OUT_BIN)/doccat$(EXE_EXT)
	cd sandbox && $(subst /,$(PS),$(OUT_BIN))/$(basename $@) -o out.doc i1.docx i3.doc i1.doc i2.doc i3.doc i2.doc i1.docx

.PHONY: copy clean
copy :
	$(COPY) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/*doccat*)) sandbox

clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_APP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_APP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_APP_OBJ))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/*doccat*))
