CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -pedantic -Iinclude
LDFLAGS = -lm

SRCDIR  = src
TESTDIR = tests
OBJDIR  = obj

SRCS    = $(SRCDIR)/griot.c $(SRCDIR)/adinkra.c $(SRCDIR)/palaver.c
OBJS    = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

.PHONY: all test clean

all: libwestafricanmath.a

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

libwestafricanmath.a: $(OBJS)
	ar rcs $@ $^

test: $(TESTDIR)/test_west_african.c libwestafricanmath.a
	$(CC) $(CFLAGS) -o $(TESTDIR)/test_runner $(TESTDIR)/test_west_african.c -L. -lwestafricanmath $(LDFLAGS)
	@echo ""
	@./$(TESTDIR)/test_runner

clean:
	rm -rf $(OBJDIR) libwestafricanmath.a $(TESTDIR)/test_runner
