
.SUFFIX:

include config.mk

OBJECTS = $(patsubst %.cpp,build/%.o,$(SOURCES))
DEPS = $(patsubst %.cpp,build/%.deps,$(SOURCES))

.PHONY = all deps clean install install-all install-bin install-dev
.DEFAULT_GOAL = all

all: $(BIN) $(LIB) $(HEADERS)

-include $(DEPS)

$(HEADERS): include/$(PKG_NAME)/%: src/%
	@echo "[INST]" $(<:src/%=%)
	@$(MKDIR) $(MKDIRFLAGS) $(dir $@)
	@cp $< $(dir $@)

$(OBJECTS): build/%.o: %.cpp
	@echo "[CXX] " $@
	@$(MKDIR) $(MKDIRFLAGS) $(dir $@)
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(DEPS): build/%.deps: %.cpp
	@echo "[DEPS]" $@
	@$(MKDIR) $(MKDIRFLAGS) $(dir $@)
	@$(DEPS_BIN) $(DEPSFLAGS) -MM -MT build/$*.o $< > $@
	@$(DEPS_BIN) $(DEPSFLAGS) -MM -MT build/$*.deps $< >> $@

$(BIN): bin/%:
	@echo "[LD]  " $@
	@$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIB)

$(LIB): lib/%:
	@echo "[AR]  " $@
	@$(AR) $(ARFLAGS) $@ $^

deps: $(DEPS)

clean:
	@rm -f $(OBJECTS)
	@rm -f $(DEPS)
	@rm -f $(BIN)
	@rm -rf build/*
	@rm -rf include/*
	@rm -f $(LIB)

install: install-dev
install-dev: install-header install-lib
install-all: install-header install-lib install-bin

install-header: $(HEADERS)
ifneq ($(HEADERS),)
	@echo "[CP]  " $(HEADERS)
	@$(MKDIR) $(MKDIRFLAGS) $(PREFIX)/$(INCLUDE_DIR)/
	@cp -r include/* $(PREFIX)/$(INCLUDE_DIR)/
endif

install-lib: $(LIB)
ifneq ($(LIB),)
	@echo "[CP]  " $(LIB)
	@$(MKDIR) $(MKDIRFLAGS) $(PREFIX)/$(LIB_DIR)/
	@cp $(LIB) $(PREFIX)/$(LIB_DIR)/
endif

install-bin: $(BIN)
ifneq ($(BIN),)
	@echo "[CP]  " $(BIN)
	@$(MKDIR) $(MKDIRFLAGS) $(PREFIX)/$(BIN_DIR)/
	@cp $(BIN) $(PREFIX)/$(BIN_DIR)/
endif

print-%:
	@echo $*=$($*)
