
CC=clang

CFLAGS+=-g -Wall -Wextra -Werror -Wconversion -Wpedantic -Wdeclaration-after-statement -W -Wshadow
CFLAGS+=-Wcast-qual -Wwrite-strings -Wundef -Wcast-align -Wmissing-prototypes -Wno-missing-field-initializers
# enable all warnings for clang
CFLAGS+=-Weverything

OBJ=can_wrap.o can_wrap_raw.o

all: ../libcanwrap.so

../libcanwrap.so: $(OBJ)
	$(CC) -shared -g -o $@ $(OBJ)

.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	@rm -rf *.o
	@rm -rf ../libcanwrap.so

