-include config.mk

SRC=src/main.c 									\
		src/mozjpeg-recompress.c 		\
		src/turbojpeg-recompress.c 	\
		src/png-recompress.c 				\
		src/utils.c 								\
		src/jpeg-utils.c

OBJS=$(SRC:.c=.o)
DEPS=$(SRC:.c=.d)
PNGQUANT_OBJS=$(wildcard src/pngquant/*.o)

BIN=image-worker

all: config.mk pngquant $(OBJS)
	$(CC) -o $(BIN) $(OBJS) $(PNGQUANT_OBJS) $(LDFLAGS)

config.mk:
	./configure

pngquant:
	make -C src/pngquant

clean:
	make -C src/pngquant clean
	rm -f $(OBJS) $(DEPS) $(BIN)

distclean: clean
	rm config.mk

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: pngquant

-include $(DEPS)
