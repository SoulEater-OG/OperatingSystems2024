CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
OBJDIR = obj
BINDIR = bin
TESTDIR = test

# Object files
OBJS = $(OBJDIR)/job_control.o $(OBJDIR)/main.o $(OBJDIR)/parser.o $(OBJDIR)/shell.o $(OBJDIR)/vfs.o $(OBJDIR)/commands.o
FORMAT_OBJS = $(OBJDIR)/format.o $(OBJDIR)/format_lib.o
TEST_WRITE_OBJS = $(OBJDIR)/test_vfs_write.o $(OBJDIR)/vfs.o $(OBJDIR)/format_lib.o
TEST_READ_OBJS = $(OBJDIR)/test_vfs_read.o $(OBJDIR)/vfs.o $(OBJDIR)/format_lib.o
TEST_LSROOT_OBJS = $(OBJDIR)/test_vfs_lsroot.o $(OBJDIR)/vfs.o $(OBJDIR)/format_lib.o
TEST_DIRCYCLE_OBJS = $(OBJDIR)/test_vfs_dircycle.o $(OBJDIR)/vfs.o $(OBJDIR)/format_lib.o
TEST_ROOTCLR_OBJS = $(OBJDIR)/test_vfs_rootclr.o $(OBJDIR)/vfs.o $(OBJDIR)/format_lib.o
TEST_FILESTAT_OBJS = $(OBJDIR)/test_vfs_filestat.o $(OBJDIR)/vfs.o $(OBJDIR)/format_lib.o

# Binaries
BIN = $(BINDIR)/mysh
FORMAT_BIN = $(BINDIR)/format
TEST_WRITE_BIN = $(BINDIR)/test_vfs_write
TEST_READ_BIN = $(BINDIR)/test_vfs_read
TEST_LSROOT_BIN = $(BINDIR)/test_vfs_lsroot
TEST_DIRCYCLE_BIN = $(BINDIR)/test_vfs_dircycle
TEST_ROOTCLR_BIN = $(BINDIR)/test_vfs_rootclr
TEST_FILESTAT_BIN = $(BINDIR)/test_vfs_filestat

# Targets
all: $(BIN) $(FORMAT_BIN) $(TEST_WRITE_BIN) $(TEST_READ_BIN) $(TEST_LSROOT_BIN) $(TEST_DIRCYCLE_BIN) $(TEST_ROOTCLR_BIN) $(TEST_FILESTAT_BIN)

$(BIN): $(OBJS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(FORMAT_BIN): $(FORMAT_OBJS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_WRITE_BIN): $(TEST_WRITE_OBJS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_READ_BIN): $(TEST_READ_OBJS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_LSROOT_BIN): $(TEST_LSROOT_OBJS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_DIRCYCLE_BIN): $(TEST_DIRCYCLE_OBJS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_ROOTCLR_BIN): $(TEST_ROOTCLR_OBJS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_FILESTAT_BIN): $(TEST_FILESTAT_OBJS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: src/%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/test_vfs_write.o: $(TESTDIR)/test_vfs_write.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/test_vfs_read.o: $(TESTDIR)/test_vfs_read.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/test_vfs_lsroot.o: $(TESTDIR)/test_vfs_lsroot.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/test_vfs_dircycle.o: $(TESTDIR)/test_vfs_dircycle.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/test_vfs_rootclr.o: $(TESTDIR)/test_vfs_rootclr.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/test_vfs_filestat.o: $(TESTDIR)/test_vfs_filestat.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJDIR) $(BINDIR)

.PHONY: all clean
