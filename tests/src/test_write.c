/**
 * @file test_write.c
 * @brief Validation scenarios for the ft_write assembly implementation.
 */

#include <tests.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

typedef struct {
	const char *name;
	int fd;
	const char *buf;
	size_t count;
	ssize_t expected_ret; 
	int expected_errno;
	bool should_crash;
	const char *description;
} WriteTestCase;

#define WRITE_TEST_CASE(name_str, fd_val, buf_str, count_val, exp_ret, exp_errno, crash, desc) \
	{ \
		.name = name_str, \
		.fd = fd_val, \
		.buf = buf_str, \
		.count = count_val, \
		.expected_ret = exp_ret, \
		.expected_errno = exp_errno, \
		.should_crash = crash, \
		.description = desc \
	}

/** Runs a write-like function in a child process to detect crash behavior. */
static bool check_write_crash(ssize_t (*func)(int, const void *, size_t), 
							   int fd, const void *buf, size_t count)
{
	pid_t pid = fork();
	
	if (pid == ERROR) {
		dprintf(1, "%s Failed to create child process\n", STR_ERR);
		return false;
	}
	
	if (pid == 0) {
		func(fd, buf, count);
		_exit(0);
	} else {
		int status;
		waitpid(pid, &status, 0);
		
		if (WIFSIGNALED(status)) {
			return true;
		}
		return false;
	}
}

/** Resets errno before a write-oriented assertion. */
static void clear_errno(void)
{
	errno = 0;
}

/** Runs one ft_write scenario and compares it against libc write. */
static bool run_write_test(WriteTestCase *test, int index)
{
	dprintf(1, "\n%s--- Test %d: %s ---%s\n", CYAN, index + 1, test->name, END);
	
	dprintf(1, "  Description: %s\n", test->description);
	dprintf(1, "  fd: %d\n", test->fd);
	dprintf(1, "  buf: \"%s\"\n", test->buf ? test->buf : "NULL");
	dprintf(1, "  count: %zu\n", test->count);
	dprintf(1, "  Expected return: %zd\n", test->expected_ret);
	dprintf(1, "  Expected errno: %d (%s)\n", test->expected_errno, 
			test->expected_errno ? strerror(test->expected_errno) : "none");
	dprintf(1, "  Should crash: %s\n", test->should_crash ? "YES" : "NO");
	
	bool my_crashed = false;
	bool orig_crashed = false;
	ssize_t my_ret = 0;
	ssize_t orig_ret = 0;
	int my_errno_saved = 0;
	int orig_errno_saved = 0;
	
	if (!test->should_crash) {
		clear_errno();
		my_ret = ft_write(test->fd, test->buf, test->count);
		my_errno_saved = errno;
		
		clear_errno();
		orig_ret = write(test->fd, test->buf, test->count);
		orig_errno_saved = errno;
	} else {
		my_crashed = check_write_crash(ft_write, test->fd, test->buf, test->count);
		orig_crashed = check_write_crash(write, test->fd, test->buf, test->count);
	}
	
	dprintf(1, "  Results:\n");
	
	if (!test->should_crash) {
		dprintf(1, "    ft_write: ret=%zd, errno=%d (%s)\n", 
				my_ret, my_errno_saved, strerror(my_errno_saved));
		dprintf(1, "    write:    ret=%zd, errno=%d (%s)\n", 
				orig_ret, orig_errno_saved, strerror(orig_errno_saved));
	} else {
		dprintf(1, "    ft_write: %s\n", my_crashed ? "CRASHED" : "OK");
		dprintf(1, "    write:    %s\n", orig_crashed ? "CRASHED" : "OK");
	}
	
	bool correct = true;

	if (test->should_crash) {
		if (my_crashed != orig_crashed) {
			dprintf(1, "  %s✗ Different crash behavior%s\n", RED, END);
			correct = false;
		}
		
		if (my_crashed != test->should_crash) {
			dprintf(1, "  %s✗ Expected crash but didn't%s\n", RED, END);
			correct = false;
		} else {
			dprintf(1, "  %s✓ Correct crash behavior%s\n", GREEN, END);
		}
		return correct;
	}

	if (my_ret != orig_ret) {
		dprintf(1, "  %s✗ Return value mismatch: ft=%zd, orig=%zd%s\n",
				RED, my_ret, orig_ret, END);
		correct = false;
	} else if (my_ret != test->expected_ret) {
		dprintf(1, "  %s✗ Wrong return value: got %zd, expected %zd%s\n",
				RED, my_ret, test->expected_ret, END);
		correct = false;
	}
	
	if (my_errno_saved != orig_errno_saved) {
		dprintf(1, "  %s✗ errno mismatch: ft=%d, orig=%d%s\n",
				RED, my_errno_saved, orig_errno_saved, END);
		correct = false;
	} else if (my_errno_saved != test->expected_errno) {
		dprintf(1, "  %s✗ Wrong errno: got %d (%s), expected %d (%s)%s\n",
				RED, my_errno_saved, strerror(my_errno_saved),
				test->expected_errno, strerror(test->expected_errno), END);
		correct = false;
	}
	
	if (correct) {
		dprintf(1, "  %s✓ Test PASSED%s\n", GREEN, END);
	} else {
		dprintf(1, "  %s✗ Test FAILED%s\n", RED, END);
	}
	
	return correct;
}

/** Covers successful writes to standard output. */
static void test_stdout_writes(void)
{
	dprintf(1, "\n%s--- STDOUT Writes ---%s\n", CYAN, END);
	
	WriteTestCase tests[] = {
		WRITE_TEST_CASE("Write short string", 1, "Hello\n", 6, 6, 0, false,
					   "Write to stdout should succeed"),
		
		WRITE_TEST_CASE("Write empty string", 1, "", 0, 0, 0, false,
					   "Writing 0 bytes should return 0"),
		
		WRITE_TEST_CASE("Write single char", 1, "A", 1, 1, 0, false,
					   "Write single character"),
		
		WRITE_TEST_CASE("Write newline only", 1, "\n", 1, 1, 0, false,
					   "Write only newline"),
		
		WRITE_TEST_CASE("Write long string", 1, 
			"This is a very long string that tests multiple write operations\n", 
			64, 64, 0, false, "Write long string"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_write_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sSTDOUT writes: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

/** Covers invalid file descriptor failures. */
static void test_invalid_fds(void)
{
	dprintf(1, "\n%s--- Invalid File Descriptors ---%s\n", CYAN, END);
	
	WriteTestCase tests[] = {
		WRITE_TEST_CASE("fd = -1", -1, "test", 4, -1, EBADF, false,
					   "Invalid fd should return EBADF"),
		
		WRITE_TEST_CASE("fd = 999 (non-existent)", 999, "test", 4, -1, EBADF, false,
					   "Non-existent fd should return EBADF"),
		
		WRITE_TEST_CASE("fd = 1000", 1000, "test", 4, -1, EBADF, false,
					   "Very large fd should fail"),
		
		WRITE_TEST_CASE("fd = 3 (likely closed)", 3, "test", 4, -1, EBADF, false,
					   "Closed fd should fail"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_write_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sInvalid FDs: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

/** Covers writes to a regular file and verifies the resulting file contents. */
static void test_file_writes(void)
{
	dprintf(1, "\n%s--- File Writes ---%s\n", CYAN, END);
	
	const char *test_file = "/tmp/ft_write_test.txt";
	int fd = open(test_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	
	if (fd == -1) {
		dprintf(1, "  %sFailed to create test file%s\n", RED, END);
		return;
	}
	
	WriteTestCase tests[] = {
		WRITE_TEST_CASE("Write to file", fd, "Hello World\n", 12, 12, 0, false,
					   "Write to regular file should succeed"),
		
		WRITE_TEST_CASE("Write empty to file", fd, "", 0, 0, 0, false,
					   "Write 0 bytes to file"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_write_test(&tests[i], i)) {
			passed++;
		}
	}
	
	close(fd);

	fd = open(test_file, O_RDONLY);
	char buffer[100] = {0};
	read(fd, buffer, sizeof(buffer));
	close(fd);
	
	dprintf(1, "  File content after tests: \"%s\"\n", buffer);
	unlink(test_file);
	
	dprintf(1, "\n%sFile writes: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

/** Covers successful writes to standard error. */
static void test_stderr_writes(void)
{
	dprintf(1, "\n%s--- STDERR Writes ---%s\n", CYAN, END);
	
	WriteTestCase tests[] = {
		WRITE_TEST_CASE("Write to stderr", 2, "Error message\n", 14, 14, 0, false,
					   "Write to stderr should succeed"),
		
		WRITE_TEST_CASE("Write error prefix", 2, "[ERROR] ", 7, 7, 0, false,
					   "Write error prefix to stderr"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_write_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sSTDERR writes: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

/** Covers NULL buffers, zero counts, and oversized counts. */
static void test_edge_cases(void)
{
	dprintf(1, "\n%s--- Edge Cases ---%s\n", CYAN, END);

	WriteTestCase tests[] = {
		WRITE_TEST_CASE("NULL buffer", 1, NULL, 10, -1, 14, false,
					   "Writing NULL buffer should NO crash"),
		
		WRITE_TEST_CASE("Zero count with valid fd", 1, "test", 0, 0, 0, false,
					   "Writing 0 bytes should succeed"),
		
		WRITE_TEST_CASE("Count larger than buffer", 1, "test", 100, 100, 0, false,
					   "Write more bytes than buffer length (reads beyond buffer)"),
		
		WRITE_TEST_CASE("Negative count", 1, "test", -1, -1, 14, false,
					   "Negative count should return error"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_write_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sEdge cases: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

/** Covers pipe writes and the broken-pipe error path. */
static void test_pipe_writes(void)
{
	dprintf(1, "\n%s--- Pipe Writes ---%s\n", CYAN, END);
	
	int pipefd[2];
	struct sigaction old_sa;

	if (pipe(pipefd) == 0) {
		WriteTestCase test = WRITE_TEST_CASE("Write to pipe", pipefd[1], "pipe test", 9, 9, 0, false,
										"Write to pipe should succeed");
		run_write_test(&test, 0);
		close(pipefd[0]);
		close(pipefd[1]);
	}

	if (pipe(pipefd) == 0) {
		close(pipefd[0]);
		struct sigaction sa;
		sa.sa_handler = SIG_IGN;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sigaction(SIGPIPE, &sa, &old_sa);
		
		WriteTestCase test = WRITE_TEST_CASE("Broken pipe", pipefd[1], "test", 4, -1, EPIPE, false,
										"Write to broken pipe should return EPIPE");
		run_write_test(&test, 1);
		
		sigaction(SIGPIPE, &old_sa, NULL);
		close(pipefd[1]);
	}
}

/** Covers EBADF behavior when writing through a read-only descriptor. */
static void test_readonly_fd(void)
{
	dprintf(1, "\n%s--- Read-only File Descriptor ---%s\n", CYAN, END);
	
	const char *test_file = "/tmp/ft_write_readonly.txt";
	int fd = open(test_file, O_RDONLY | O_CREAT, 0644);
	
	if (fd != -1) {
		WriteTestCase test = WRITE_TEST_CASE("Write to read-only fd", fd, "test", 4, -1, EBADF, false,
										   "Write to read-only fd should return EBADF");
		run_write_test(&test, 0);
		close(fd);
		unlink(test_file);
	}
}

/** Launches the complete ft_write test suite. */
void inject_data_write(void)
{
	dprintf(1, "\n%s========================================%s\n", BLUE, END);
	dprintf(1, "%s         TESTING WRITE%s\n", BLUE, END);
	dprintf(1, "%s========================================%s\n\n", BLUE, END);
	
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 1: STDOUT Writes%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_stdout_writes();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 2: Invalid FDs%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_invalid_fds();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 3: File Writes%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_file_writes();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 4: STDERR Writes%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_stderr_writes();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 5: Edge Cases%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_edge_cases();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 6: Pipe Writes%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_pipe_writes();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 7: Read-only FD%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_readonly_fd();
	
	dprintf(1, "\n%s========================================%s\n", BLUE, END);
	dprintf(1, "%s           ALL TESTS COMPLETE%s\n", BLUE, END);
	dprintf(1, "%s========================================%s\n\n", BLUE, END);
}
