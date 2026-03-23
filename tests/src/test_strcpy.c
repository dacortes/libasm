/**
 * @file test_strcpy.c
 * @brief Validation scenarios for the ft_strcpy assembly implementation.
 */

#include <tests.h>

typedef struct {
	const char *name;
	size_t dst_size;
	const char *src;
	const char *description;
	bool should_crash;
} StrcpyTestCase;

#define TEST_CASE(name_str, dst_size_val, src_str) \
{ \
	.name = name_str, \
	.dst_size = dst_size_val, \
	.src = src_str, \
	.description = "strcpy test" \
}

typedef struct {
	char ft_result[256];
	char orig_result[256];
	bool ft_crashed;
	bool orig_crashed;
} SharedResult;

static SharedResult *shared = NULL;

/** Runs a strcpy-like function in a child process to detect crashes safely. */
static bool check_aborted(char *(*func)(char *, const char *), 
						char *dst, const char *src, 
						char *result_buffer, size_t result_size)
{
	pid_t pid = fork();
	
	if (pid == ERROR) {
		dprintf(1, "%s Failed to create child process\n", STR_ERR);
		return false;
	}
	
	if (pid == 0) {

		char local_buffer[256];
		(void)local_buffer;

		func(dst, src);

		strncpy(result_buffer, dst, result_size - 1);
		result_buffer[result_size - 1] = '\0';
		
		_exit(0);
	} 
	else {

		int status;
		waitpid(pid, &status, 0);
		
		if (WIFSIGNALED(status)) {
			int sig = WTERMSIG(status);
			if (sig == SIGABRT || sig == SIGSEGV) {
				return true;
			}
			return true;
		}
		
		if (WIFEXITED(status)) {
			int exit_code = WEXITSTATUS(status);
			if (exit_code == 0) {
				return false;
			}
		}
		
		return true;
	}
}

/** Runs one buffer-copy scenario and compares ft_strcpy with libc strcpy. */
bool run_test_case(StrcpyTestCase *test, int index) {
	char dst_orig[test->dst_size];
	char dst[test->dst_size];

	memset(dst_orig, 0xAA, test->dst_size);
	memset(dst, 0xAA, test->dst_size);
	
	dprintf(1, "\n%s--- Test %d: %s ---%s\n", CYAN, index + 1, test->name, END);
	dprintf(1, "  Destination size: %zu bytes\n", test->dst_size);
	dprintf(1, "  Source: \"%s\" (%zu bytes)\n", test->src, strlen(test->src) + 1);
	
	bool my_crashed = !check_aborted(ft_strcpy, dst, test->src, 
									shared->ft_result, sizeof(shared->ft_result));
	bool orig_crashed = !check_aborted(strcpy, dst_orig, test->src,
									shared->orig_result, sizeof(shared->orig_result));
	
	dprintf(1, "  Results:\n");
	dprintf(1, "    ft_strcpy: %s\n", my_crashed ? "CRASHED" : "OK");
	dprintf(1, "    strcpy:    %s\n", orig_crashed ? "CRASHED" : "OK");
	
	dprintf(1, "  Buffer content (hex):\n");
	dprintf(1, "    ft:  ");
	for (size_t i = 0; i < test->dst_size; i++) {
		dprintf(1, "%02x ", (unsigned char)dst[i]);
	}
	dprintf(1, "\n    orig:");
	for (size_t i = 0; i < test->dst_size; i++) {
		dprintf(1, "%02x ", (unsigned char)dst_orig[i]);
	}
	dprintf(1, "\n");
	
	dprintf(1, "    ft:  \"%s\"\n", dst);
	dprintf(1, "    orig:\"%s\"\n", dst_orig);
	
	bool correct = true;
	

	if (my_crashed != orig_crashed) {
		dprintf(1, "  %s✗ Different crash behavior: ft=%s, orig=%s%s\n", 
				RED, 
				my_crashed ? "CRASH" : "NO CRASH",
				orig_crashed ? "CRASH" : "NO CRASH",
				END);
		correct = false;
	}
	
	if (!my_crashed && !orig_crashed) {
		bool content_match = true;
		for (size_t i = 0; i < test->dst_size; i++) {
			if (dst[i] != dst_orig[i]) {
				content_match = false;
				break;
			}
		}
		
		if (content_match) {
			dprintf(1, "  %s✓ Same content%s\n", GREEN, END);
		} else {
			dprintf(1, "  %s✗ Different content%s\n", RED, END);
			correct = false;
		}
		

		bool ft_null = (dst[test->dst_size - 1] == 0);
		bool orig_null = (dst_orig[test->dst_size - 1] == 0);
		
		if (ft_null != orig_null) {
			dprintf(1, "  %s✗ Different null termination%s\n", RED, END);
			correct = false;
		}
	}
	
	if (correct) {
		dprintf(1, "  %s✓ Test PASSED (ft_strcpy behaves like strcpy)%s\n", GREEN, END);
	} else {
		dprintf(1, "  %s✗ Test FAILED (different behavior)%s\n", RED, END);
	}
	
	return correct;
}

/** Repeats the strcpy comparison with many source strings and a fixed buffer size. */
void test_with_different_sources(void)
{
	dprintf(1, "\n%s--- Testing with different source strings ---%s\n", CYAN, END);
	
	const char *sources[] = {
		"a",
		"ab", 
		"abc",
		"abcd",
		"abcde",
		"abcdef",
		"1234567890",
		"!@#$%^&*()",
		"áéíóúñ",
		"a very long string that definitely exceeds any small buffer"
	};
	
	size_t buffer_size = 5;
	int num_sources = sizeof(sources) / sizeof(sources[0]);
	int passed = 0;
	
	for (int i = 0; i < num_sources; i++) {
		size_t src_len = strlen(sources[i]) + 1;
		bool should_crash = (src_len > buffer_size);
		
		char test_name[100];
		snprintf(test_name, sizeof(test_name), 
				"Buffer[%zu] <- \"%s\"", buffer_size, sources[i]);
		
		StrcpyTestCase test = {
			.name = test_name,
			.dst_size = buffer_size,
			.src = sources[i],
			.description = "Testing different source strings",
			.should_crash = should_crash
		};
		
		if (run_test_case(&test, i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%s--- Source variation tests: %d/%d passed ---%s\n", 
			passed == num_sources ? GREEN : RED, 
			passed, num_sources, END);
}

/** Launches the complete ft_strcpy test suite. */
void inject_data_strcpy(void)
{
	dprintf(1, "\n%s========================================%s\n", BLUE, END);
	dprintf(1, "%s     TESTING STRCPY - BUFFER OVERFLOW%s\n", BLUE, END);
	dprintf(1, "%s========================================%s\n\n", BLUE, END);
	
	StrcpyTestCase tests[] = {
		TEST_CASE("Insufficient buffer (1 byte)", 1, "mundo"),
		TEST_CASE("Insufficient buffer (2 bytes)", 2, "mundo"),
		TEST_CASE("Insufficient buffer (3 bytes)", 3, "mundo"),
		TEST_CASE("Insufficient buffer (4 bytes)", 4, "mundo"),
		TEST_CASE("Insufficient buffer (5 bytes)", 5, "mundo"),
		TEST_CASE("Exact buffer (6 bytes)", 6, "mundo"),
		TEST_CASE("Large buffer (10 bytes)", 10, "mundo"),
		
		TEST_CASE("Empty string in buffer 1", 1, ""),
		TEST_CASE("Empty string in buffer 10", 10, ""),
		
		TEST_CASE("Exact buffer for 'a'", 2, "a"),
		TEST_CASE("Insufficient buffer for 'ab'", 2, "ab"),
		TEST_CASE("Exact buffer for 'ab'", 3, "ab"),
		
		TEST_CASE("Small buffer for long string", 5, "supercalifragilistico"),
		TEST_CASE("Medium buffer for long string", 20, "supercalifragilistico"),
		TEST_CASE("Sufficient buffer", 50, "supercalifragilistico"),
		
		TEST_CASE("Minimum buffer with empty string", 1, ""),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_test_case(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%s========================================%s\n", BLUE, END);
	dprintf(1, "%s     RESULTS: %d/%d tests passed%s\n", 
			passed == num_tests ? GREEN : RED, 
			passed, num_tests, END);
	dprintf(1, "%s========================================%s\n\n", BLUE, END);

	test_with_different_sources();
}
