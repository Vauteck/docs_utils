
CC := gcc
CFLAGS := -march=native -pipe -O2 -pedantic -Wall -Wextra -std=gnu99
LDFLAGS := -Wl,-O1 -s
LIBS :=

SRCS := $(wildcard *.c)
OBJS := $(SRCS:%.c=%)

all: $(OBJS)

$(OBJS): $(SRCS)
	@echo Compiling $@
	@$(CC) $(CFLAGS) $(LIBS) $(LDFLAGS) -o $@ $(@:%=%.c)

clean:
	@rm -f $(OBJS)
