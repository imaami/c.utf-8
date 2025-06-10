override BIN := test-utf8

override SRC_test-utf8 := utf8.c utf8_graph.c test-utf8.c
override LIB_test-utf8 := letopt/src/letopt.c.o

override SRC := $(foreach b,$(BIN),$(SRC_$b))
override OBJ := $(SRC:%=%.o)
override DEP := $(SRC:%=%.d)

override ARCHFLAGS := -march=native -mtune=native
override WARNFLAGS := -Wall -Wextra -Wpedantic
override BASEFLAGS := $(WARNFLAGS) -pipe -flto -O3 $(ARCHFLAGS)

CC       := gcc
CFLAGS   := $(BASEFLAGS) -std=$(or $(STD),gnu2x)
CPPFLAGS := -DNDEBUG=1

all: $(BIN)

test-utf8: | letopt

.PHONY: letopt
letopt:
	+$(MAKE) -C letopt/src CC="$(CC)" CFLAGS="$(CFLAGS)" CPPFLAGS="$(CPPFLAGS)"

$(foreach b,$(BIN),$(eval $b: $$(SRC_$b:%=%.o); \
	+$$(CC) $$(CPPFLAGS) $$(CFLAGS) -o $$@ $$^ $$(LIB_$$@)))

%.c.o: %.c
	+$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c -MMD $<

purge: purge-letopt
clean: clean-letopt

.PHONY: purge-letopt
purge-letopt:
	+$(MAKE) -C letopt/src purge

.PHONY: clean-letopt
clean-letopt:
	+$(MAKE) -C letopt/src purge

purge: override purge_FILES = $(wildcard $(DEP))
purge: | clean; $(if $(purge_FILES),$(RM) $(purge_FILES),@:)

clean: override clean_FILES = $(wildcard $(BIN) $(OBJ))
clean:; $(if $(clean_FILES),$(RM) $(clean_FILES),@:)

.PHONY: all clean purge

-include $(DEP)
