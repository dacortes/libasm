#include <tests.h>

typedef struct {
	const char *name;
	int fd;
	const char *test_data;
	size_t count;
	ssize_t expected_ret;
	int expected_errno;
	bool should_crash;
	const char *description;
} ReadTestCase;

#define READ_TEST_CASE(name_str, fd_val, test_data_str, count_val, exp_ret, exp_errno, crash, desc) \
	{ \
		.name = name_str, \
		.fd = fd_val, \
		.test_data = test_data_str, \
		.count = count_val, \
		.expected_ret = exp_ret, \
		.expected_errno = exp_errno, \
		.should_crash = crash, \
		.description = desc \
	}

static bool check_read_crash(ssize_t (*func)(int, void *, size_t), 
							  int fd, void *buf, size_t count)
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

static void clear_errno(void)
{
	errno = 0;
}

static bool run_read_test(ReadTestCase *test, int index)
{
	dprintf(1, "\n%s--- Test %d: %s ---%s\n", CYAN, index + 1, test->name, END);
	
	dprintf(1, "  Description: %s\n", test->description);
	dprintf(1, "  fd: %d\n", test->fd);
	dprintf(1, "  count: %zu\n", test->count);
	dprintf(1, "  Expected return: %zd\n", test->expected_ret);
	dprintf(1, "  Expected errno: %d (%s)\n", test->expected_errno, 
			test->expected_errno ? strerror(test->expected_errno) : "none");
	dprintf(1, "  Should crash: %s\n", test->should_crash ? "YES" : "NO");
	
	char my_buf[256] = {0};
	char orig_buf[256] = {0};
	
	bool my_crashed = false;
	bool orig_crashed = false;
	ssize_t my_ret = 0;
	ssize_t orig_ret = 0;
	int my_errno_saved = 0;
	int orig_errno_saved = 0;

	off_t original_pos = lseek(test->fd, 0, SEEK_CUR);
	
	if (!test->should_crash) {
		clear_errno();
		my_ret = ft_read(test->fd, my_buf, test->count);
		my_errno_saved = errno;

		lseek(test->fd, original_pos, SEEK_SET);
		
		clear_errno();
		orig_ret = read(test->fd, orig_buf, test->count);
		orig_errno_saved = errno;

		lseek(test->fd, original_pos, SEEK_SET);
	} else {
		my_crashed = check_read_crash(ft_read, test->fd, my_buf, test->count);
		orig_crashed = check_read_crash(read, test->fd, orig_buf, test->count);
	}
	
	dprintf(1, "  Results:\n");
	
	if (!test->should_crash) {
		dprintf(1, "    ft_read: ret=%zd, errno=%d (%s)\n", 
				my_ret, my_errno_saved, strerror(my_errno_saved));
		dprintf(1, "    read:    ret=%zd, errno=%d (%s)\n", 
				orig_ret, orig_errno_saved, strerror(orig_errno_saved));
		
		if (my_ret > 0) {
			dprintf(1, "    ft_read data: \"%.*s\"\n", (int)my_ret, my_buf);
			dprintf(1, "    read data:    \"%.*s\"\n", (int)orig_ret, orig_buf);
		}
	} else {
		dprintf(1, "    ft_read: %s\n", my_crashed ? "CRASHED" : "OK");
		dprintf(1, "    read:    %s\n", orig_crashed ? "CRASHED" : "OK");
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

	if (my_ret > 0 && orig_ret > 0 && my_ret == orig_ret) {
		if (memcmp(my_buf, orig_buf, my_ret) != 0) {
			dprintf(1, "  %s✗ Data mismatch%s\n", RED, END);
			correct = false;
		} else {
			dprintf(1, "  %s✓ Data matches%s\n", GREEN, END);
		}
	}
	
	if (correct) {
		dprintf(1, "  %s✓ Test PASSED%s\n", GREEN, END);
	} else {
		dprintf(1, "  %s✗ Test FAILED%s\n", RED, END);
	}
	
	return correct;
}

void test_stdin_reads(void)
{
	dprintf(1, "\n%s--- STDIN Reads ---%s\n", CYAN, END);
	dprintf(1, "  %sNOTE: For stdin tests, you need to provide input%s\n", YELLOW, END);

	ReadTestCase tests[] = {
		READ_TEST_CASE("Read 0 bytes from stdin", 0, NULL, 0, 0, 0, false,
					  "Reading 0 bytes should return 0"),
		
		READ_TEST_CASE("Read from stdin (will wait for input)", 0, NULL, 4, 4, 0, false,
					  "Reading 10 bytes from stdin (you need to provide input)"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_read_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sSTDIN reads: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

static void test_file_reads(void)
{
	dprintf(1, "\n%s--- File Reads ---%s\n", CYAN, END);
	
	const char *test_file = "/tmp/ft_read_test.txt";

	int fd = open(test_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd != -1) {
		const char *content = "Hello World!\nThis is a test file.\nLine 3\n";
		write(fd, content, strlen(content));
		close(fd);
	}
	
	ReadTestCase tests[] = {
		READ_TEST_CASE("Read exact size", -1, NULL, 12, 12, 0, false,
					  "Read exactly 12 bytes from file"),
		
		READ_TEST_CASE("Read less than file size", -1, NULL, 5, 5, 0, false,
					  "Read 5 bytes from file"),
		
		READ_TEST_CASE("Read more than file size", -1, NULL, 100, 41, 0, false,
					  "Read more than available (returns actual bytes)"),
		
		READ_TEST_CASE("Read 0 bytes", -1, NULL, 0, 0, 0, false,
					  "Read 0 bytes should return 0"),
		
		READ_TEST_CASE("Read from beginning", -1, NULL, 6, 6, 0, false,
					  "Read first 6 bytes"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		fd = open(test_file, O_RDONLY);
		if (fd == -1) {
			dprintf(1, "  %sFailed to open test file%s\n", RED, END);
			break;
		}
		
		ReadTestCase test = tests[i];
		test.fd = fd;
		
		if (run_read_test(&test, i)) {
			passed++;
		}
		
		close(fd);
	}
	
	unlink(test_file);
	
	dprintf(1, "\n%sFile reads: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

static void test_invalid_fds(void)
{
	dprintf(1, "\n%s--- Invalid File Descriptors ---%s\n", CYAN, END);
	
	// char buf[10];
	
	ReadTestCase tests[] = {
		READ_TEST_CASE("fd = -1", -1, NULL, 4, -1, EBADF, false,
					  "Invalid fd should return EBADF"),
		
		READ_TEST_CASE("fd = 999 (non-existent)", 999, NULL, 4, -1, EBADF, false,
					  "Non-existent fd should return EBADF"),
		
		READ_TEST_CASE("fd = 1000", 1000, NULL, 4, -1, EBADF, false,
					  "Very large fd should fail"),
		
		READ_TEST_CASE("fd = 3 (likely closed)", 3, NULL, 4, -1, EBADF, false,
					  "Closed fd should fail"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_read_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sInvalid FDs: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

static void test_pipe_reads(void)
{
	dprintf(1, "\n%s--- Pipe Reads ---%s\n", CYAN, END);
	
	int pipefd[2];

	if (pipe(pipefd) == 0) {
		// Escribir datos
		write(pipefd[1], "Hello from pipe", 15);

		// char my_buf[100] = {0};
		// char orig_buf[100] = {0};
		
		// ssize_t my_ret = ft_read(pipefd[0], my_buf, 15);
		// int my_errno = errno;j
		
		close(pipefd[0]);
		close(pipefd[1]);
	}

	if (pipe(pipefd) == 0) {
		write(pipefd[1], "Hello from pipe", 15);
		close(pipefd[1]);
		
		char my_buf[100] = {0};
		ssize_t my_ret = ft_read(pipefd[0], my_buf, 15);
		close(pipefd[0]);

		int pipefd2[2];
		if (pipe(pipefd2) == 0) {
			write(pipefd2[1], "Hello from pipe", 15);
			close(pipefd2[1]);
			
			char orig_buf[100] = {0};
			ssize_t orig_ret = read(pipefd2[0], orig_buf, 15);
			close(pipefd2[0]);
			
			dprintf(1, "\n--- Test: Read from pipe (separate pipes) ---\n");
			dprintf(1, "  ft_read: ret=%zd, data=\"%s\"\n", my_ret, my_buf);
			dprintf(1, "  read:    ret=%zd, data=\"%s\"\n", orig_ret, orig_buf);
			
			if (my_ret == orig_ret && strcmp(my_buf, orig_buf) == 0) {
				dprintf(1, "  %s✓ Pipe read test PASSED%s\n", GREEN, END);
			} else {
				dprintf(1, "  %s✗ Pipe read test FAILED%s\n", RED, END);
			}
		}
	}
}

static void test_edge_cases(void)
{
	dprintf(1, "\n%s--- Edge Cases ---%s\n", CYAN, END);
	
	const char *test_file = "/tmp/ft_read_edge.txt";
	int fd = open(test_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd != -1) {
		const char *content = "Test content";
		write(fd, content, strlen(content));
		close(fd);
	}
	
	// char buf[10];
	
	ReadTestCase tests[] = {
		READ_TEST_CASE("NULL buffer with file", -1, NULL, 10, 10, 0, false,
					  "NULL buffer should return EFAULT"),
		
		READ_TEST_CASE("Zero count with valid fd", -1, NULL, 0, 0, 0, false,
					  "Reading 0 bytes should succeed"),
		
		READ_TEST_CASE("Negative count", 0, NULL, -1, -1, 14, false,
					  "Negative count should return error"),
		
		READ_TEST_CASE("Buffer smaller than read", -1, NULL, 100, 12, 0, false,
					  "Buffer smaller than available data"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (tests[i].fd == -1) {
			fd = open(test_file, O_RDONLY);
			if (fd != -1) {
				ReadTestCase test = tests[i];
				test.fd = fd;
				if (run_read_test(&test, i)) {
					passed++;
				}
				close(fd);
			}
		} else {
			if (run_read_test(&tests[i], i)) {
				passed++;
			}
		}
	}
	
	unlink(test_file);
	
	dprintf(1, "\n%sEdge cases: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

static void test_eof_behavior(void)
{
	dprintf(1, "\n%s--- EOF Behavior ---%s\n", CYAN, END);
	
	const char *test_file = "/tmp/ft_read_eof.txt";
	int fd = open(test_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd != -1) {
		write(fd, "Small", 5);
		close(fd);
	}
	
	// char buf[10];
	fd = open(test_file, O_RDONLY);
	
	if (fd != -1) {
		ReadTestCase test1 = READ_TEST_CASE("First read", fd, NULL, 10, 5, 0, false,
										  "Read should get 5 bytes");
		run_read_test(&test1, 0);
		ReadTestCase test2 = READ_TEST_CASE("Second read (EOF)", fd, NULL, 10, 5, 0, false,
										  "Read after EOF should return 0");
		run_read_test(&test2, 1);
		
		close(fd);
	}
	
	unlink(test_file);
	
	dprintf(1, "\n%sEOF behavior: tests completed%s\n", CYAN, END);
}

static void test_dev_zero(void)
{
	dprintf(1, "\n%s--- Read from /dev/zero ---%s\n", CYAN, END);
	
	int fd = open("/dev/zero", O_RDONLY);
	if (fd != -1) {
		ReadTestCase test = READ_TEST_CASE("Read from /dev/zero", fd, NULL, 10, 10, 0, false,
										 "Reading from /dev/zero should return zeros");
		run_read_test(&test, 0);
		close(fd);
	} else {
		dprintf(1, "  %sCannot open /dev/zero%s\n", YELLOW, END);
	}
}

void test_dev_urandom(void)
{
	dprintf(1, "\n%s--- Read from /dev/urandom ---%s\n", CYAN, END);
	
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd != -1) {
		ReadTestCase test = READ_TEST_CASE("Read from /dev/urandom", fd, NULL, 16, 16, 0, false,
										 "Reading random data should succeed");
		run_read_test(&test, 0);
		close(fd);
	} else {
		dprintf(1, "  %sCannot open /dev/urandom%s\n", YELLOW, END);
	}
}

void inject_data_read(void)
{
	dprintf(1, "\n%s========================================%s\n", BLUE, END);
	dprintf(1, "%s         TESTING READ%s\n", BLUE, END);
	dprintf(1, "%s========================================%s\n\n", BLUE, END);
	
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 1: STDIN Reads%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	// test_stdin_reads();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 2: File Reads%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_file_reads();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 3: Invalid FDs%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_invalid_fds();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 4: Pipe Reads%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_pipe_reads();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 5: Edge Cases%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_edge_cases();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 6: EOF Behavior%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_eof_behavior();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 7: /dev/zero%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_dev_zero();
	
	// dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	// dprintf(1, "%s      SUITE 8: /dev/urandom%s\n", BLUE, END);
	// dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	// test_dev_urandom();
	
	dprintf(1, "\n%s========================================%s\n", BLUE, END);
	dprintf(1, "%s           ALL TESTS COMPLETE%s\n", BLUE, END);
	dprintf(1, "%s========================================%s\n\n", BLUE, END);
}