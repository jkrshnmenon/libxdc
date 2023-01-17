CC = gcc-7
CFLAGS += -Ofast -fPIC -fvisibility=hidden -flto -finline-functions #-fprofile-use=program.gcda #-fprofile-generate #-g -fsanitize=address 
# CFLAGS += -DDEBUG_TRACES -fPIC -g
LDFLAGS += -flto
PREFIX ?= /usr

ODIR=build
SDIR=src

_OBJ = cfg.o disassembler.o tnt_cache.o decoder.o libxdc.o mmh3.o trace_cache.o shm_bitmap.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

default: libxdc.so libxdc.a

$(ODIR)/%.o: $(SDIR)/%.c $(SDIR)/*.h libxdc.h
	mkdir -p build
	$(CC) -c -o $@ $< $(CFLAGS)

libxdc.so: $(OBJ)
	$(CC) $^ -o $@ -shared $(CFLAGS) $(LDFLAGS) -l:libcapstone.so.4

libxdc.a: $(OBJ)
	$(AR) rcs $@ $^

tester_dyn: libxdc.so test/*.c test/*.h
	$(CC) test/tester.c test/page_cache.c test/helper.c -o $@ -Itest/ -I./ $(CFLAGS) $(LDFLAGS) -L. -lxdc -l:libcapstone.so.4

tester_static: libxdc.a test/*.c test/*.h
	$(CC) test/tester.c test/page_cache.c test/helper.c -o $@ -Itest/ -I./ $(CFLAGS) $(LDFLAGS) -L. -l:libxdc.a -l:libcapstone.so.4

install: libxdc.so libxdc.a
	mkdir -p $(PREFIX)/include $(PREFIX)/lib
	install -m0644 libxdc.h $(PREFIX)/include/
	install -m0755 libxdc.so $(PREFIX)/lib/
	install -m0755 libxdc.a $(PREFIX)/lib/

.PHONY: clean install

clean:
	rm -f $(ODIR)/*.o build/*
	rm -f libxdc.so
	rm -f libxdc.a
