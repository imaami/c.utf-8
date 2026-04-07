override THIS_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

override BIN := test-utf8

override SRC_test-utf8 := utf8.c utf8_graph.c test-utf8.c
override EXT_test-utf8 := $(THIS_DIR)letopt/src/letopt.c.o

override CPPFLAGS_test-utf8.c := \
 -Wno-unterminated-string-initialization \
 -Wno-unused-label

include $(THIS_DIR)../mk/common.mk

.PHONY: clean-letopt
clean: | clean-letopt

$(EXT_test-utf8): private CPPFLAGS := $(CPPFLAGS) \
  -Wno-unterminated-string-initialization

$(EXT_test-utf8):
	$(strip @$(MAKE) -C letopt/src CC="$(CC)" \
	                 CPPFLAGS="$(CPPFLAGS)"   \
	                 CFLAGS="$(CFLAGS)" $(@F))

clean-letopt:
	+$(MAKE) -C letopt/src purge
