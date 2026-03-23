/**
 * @file test_strlen.c
 * @brief Validation scenarios for the ft_strlen assembly implementation.
 */

#include <tests.h>

sigjmp_buf env;

typedef struct {
	const char *name;
	const char *input;
	size_t expected_len;
	bool should_crash;
	const char *description;
} StrlenTestCase;

#define STRLEN_TEST_CASE(name_str, input_str, expected, crash, desc) \
{ \
	.name = name_str, \
	.input = input_str, \
	.expected_len = expected, \
	.should_crash = crash, \
	.description = desc \
}

static const char* safe_string(const char *str) {
	return str ? str : "NULL";
}

/** Converts SIGSEGV into a long jump so crash-oriented tests can continue. */
void	handle_sigsegv(int sig, siginfo_t *info, void *context)
{
	(void)sig;
	(void)info;
	(void)context;
	printf("\t%s Captured signal %d (%s)\n", STR_WAR, sig, strsignal(sig));
	siglongjmp(env, 1);
}

/** Converts SIGABRT into a long jump so abort-oriented tests can continue. */
void handle_sigabrt(int sig, siginfo_t *info, void *context)
{
	(void)sig;
	(void)info;
	(void)context;
	printf("\t%s Captured stack smashing (SIGABRT)\n", STR_WAR);
	siglongjmp(env, 1);
}

/** Executes a strlen-like function while capturing crash signals for invalid input tests. */
static bool	check_null(size_t (*func)(const char *), const char *param)
{
	struct sigaction sa_segv, sa_abrt;
	struct sigaction old_sa_segv, old_sa_abrt;

	sa_segv.sa_sigaction = handle_sigsegv;
	sigemptyset(&sa_segv.sa_mask);
	sa_segv.sa_flags = SA_SIGINFO | SA_NODEFER;
	
	sa_abrt.sa_sigaction = handle_sigabrt;
	sigemptyset(&sa_abrt.sa_mask);
	sa_abrt.sa_flags = SA_SIGINFO | SA_NODEFER;

	if (sigaction(SIGSEGV, &sa_segv, &old_sa_segv) == ERROR ||
		sigaction(SIGABRT, &sa_abrt, &old_sa_abrt) == ERROR) {
		perror(ERR_SIG);
		return false;
	}
	
	bool had_error = false;
	
	if (sigsetjmp(env, 1) == 0) {
		func(param);
		had_error = false;
	} else 
		had_error = true;
	
	sigaction(SIGSEGV, &old_sa_segv, NULL);
	sigaction(SIGABRT, &old_sa_abrt, NULL);
	
	return had_error;
}

/** Runs one ft_strlen scenario and compares the result against the libc strlen implementation. */
static bool run_strlen_test(StrlenTestCase *test, int index) {
	dprintf(1, "\n%s--- Test %d: %s ---%s\n", CYAN, index + 1, test->name, END);
	
	if (test->description) {
		dprintf(1, "  Description: %s\n", test->description);
	}
	dprintf(1, "  Input: \"%s\"\n", safe_string(test->input));
	dprintf(1, "  Expected length: %zu\n", test->expected_len);
	dprintf(1, "  Should crash: %s\n", test->should_crash ? "YES" : "NO");

	bool my_crashed = false;
	bool orig_crashed = false;
	
	if (test->input == NULL) {
		my_crashed = check_null(ft_strlen, NULL);
		orig_crashed = check_null(strlen, NULL);
	} else {
		my_crashed = check_null(ft_strlen, test->input);
		orig_crashed = check_null(strlen, test->input);
	}
	
	dprintf(1, "  Crash results:\n");
	dprintf(1, "    ft_strlen: %s\n", my_crashed ? "CRASHED" : "OK");
	dprintf(1, "    strlen:    %s\n", orig_crashed ? "CRASHED" : "OK");
	
	bool correct = true;

	if (my_crashed != orig_crashed) {
		dprintf(1, "  %s✗ Different crash behavior: ft=%s, orig=%s%s\n", 
				RED,
				my_crashed ? "CRASH" : "NO CRASH",
				orig_crashed ? "CRASH" : "NO CRASH",
				END);
		correct = false;
	}

	if (my_crashed != test->should_crash) {
		dprintf(1, "  %s✗ ft_strlen: expected %s, got %s%s\n",
				RED,
				test->should_crash ? "CRASH" : "NO CRASH",
				my_crashed ? "CRASH" : "NO CRASH",
				END);
		correct = false;
	}

	if (!my_crashed && !orig_crashed && test->input) {
		size_t len_my = ft_strlen(test->input);
		size_t len_orig = strlen(test->input);
		
		dprintf(1, "  Length results:\n");
		dprintf(1, "    ft_strlen: %zu\n", len_my);
		dprintf(1, "    strlen:    %zu\n", len_orig);
		dprintf(1, "    Expected:  %zu\n", test->expected_len);

		if (len_my != len_orig) {
			dprintf(1, "  %s✗ Length mismatch: ft=%zu, orig=%zu%s\n",
					RED, len_my, len_orig, END);
			correct = false;
		}

		if (len_my != test->expected_len) {
			dprintf(1, "  %s✗ Wrong length: got %zu, expected %zu%s\n",
					RED, len_my, test->expected_len, END);
			correct = false;
		}
	}
	
	if (correct) {
		dprintf(1, "  %s✓ Test PASSED%s\n", GREEN, END);
	} else {
		dprintf(1, "  %s✗ Test FAILED%s\n", RED, END);
	}
	
	return correct;
}

/** Covers typical ASCII strings with different lengths. */
static void test_normal_strings(void) {
	dprintf(1, "\n%s--- Normal Strings ---%s\n", CYAN, END);
	
	StrlenTestCase tests[] = {
		STRLEN_TEST_CASE("Empty string", "", 0, false,
						"Empty string should return 0"),
		
		STRLEN_TEST_CASE("Single character", "a", 1, false,
						"Single character string"),
		
		STRLEN_TEST_CASE("Two characters", "ab", 2, false,
						"Two character string"),
		
		STRLEN_TEST_CASE("Simple word", "hello", 5, false,
						"Regular word"),
		
		STRLEN_TEST_CASE("With space", "hello world", 11, false,
						"String with space"),
		
		STRLEN_TEST_CASE("With punctuation", "Hello, World!", 13, false,
						"String with punctuation"),
		
		STRLEN_TEST_CASE("Numbers", "1234567890", 10, false,
						"String of numbers"),
		
		STRLEN_TEST_CASE("Mixed", "abc123!@#", 9, false,
						"Mixed characters"),
		
		STRLEN_TEST_CASE("Very long", "This is a very long string that should still work correctly", 
						59, false, "Testing with long input")
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_strlen_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sNormal strings: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

/** Covers UTF-8 and control-character inputs handled byte by byte. */
static void test_special_characters(void) {
	dprintf(1, "\n%s--- Special Characters ---%s\n", CYAN, END);
	
	StrlenTestCase tests[] = {
		STRLEN_TEST_CASE("UTF-8: á", "á", 2, false,
						"á is 2 bytes in UTF-8"),
		
		STRLEN_TEST_CASE("UTF-8: é", "é", 2, false,
						"é is 2 bytes in UTF-8"),
		
		STRLEN_TEST_CASE("UTF-8: ñ", "ñ", 2, false,
						"ñ is 2 bytes in UTF-8"),
		
		STRLEN_TEST_CASE("UTF-8: ü", "ü", 2, false,
						"ü is 2 bytes in UTF-8"),
		
		STRLEN_TEST_CASE("UTF-8: áéíóú", "áéíóú", 10, false,
						"5 accented chars = 10 bytes"),
		
		STRLEN_TEST_CASE("UTF-8: ñoño", "ñoño", 6, false,
						"4 chars with ñ = 6 bytes"),
		
		STRLEN_TEST_CASE("UTF-8: 日本", "日本", 6, false,
						"Japanese characters are 3 bytes each"),
		
		STRLEN_TEST_CASE("UTF-8: 🌟", "🌟", 4, false,
						"Emoji is 4 bytes"),
		
		STRLEN_TEST_CASE("Mixed UTF-8", "Hello 世界 🌟", 17, false,
						"Mixed ASCII and UTF-8")
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_strlen_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sSpecial characters: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

/** Covers NULL pointers and other edge conditions. */
static void test_edge_cases(void) {
	dprintf(1, "\n%s--- Edge Cases ---%s\n", CYAN, END);
	
	StrlenTestCase tests[] = {
		STRLEN_TEST_CASE("NULL pointer", NULL, 0, true,
						"NULL should crash (segfault)"),
		
		STRLEN_TEST_CASE("String with \\0 inside", "hello\0world", 5, false,
						"strlen stops at first null terminator"),
		
		STRLEN_TEST_CASE("Just null terminator", "\0", 0, false,
						"String that starts with null"),
		
		STRLEN_TEST_CASE("Multiple nulls", "\0\0\0", 0, false,
						"Multiple nulls at start"),
		
		STRLEN_TEST_CASE("Very long (100 chars)", 
						"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 
						100, false, "Testing with long string (first 100 chars)")
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_strlen_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sEdge cases: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

/** Exercises strlen with arrays of strings to validate repeated calls. */
static void test_string_arrays(void) {
	dprintf(1, "\n%s--- String Arrays ---%s\n", CYAN, END);
	
	const char *test_strings[] = {
		"",
		"a",
		"ab",
		"abc",
		"abcd",
		"abcde",
		"abcdef",
		"abcdefg",
		"abcdefgh",
		"abcdefghi",
	};
	
	int num_strings = sizeof(test_strings) / sizeof(test_strings[0]);
	int passed = 0;
	
	for (int i = 0; i < num_strings; i++) {
		char test_name[100];
		snprintf(test_name, sizeof(test_name), "String length %d", i);
		
		StrlenTestCase test = {
			.name = test_name,
			.input = test_strings[i],
			.expected_len = i,
			.should_crash = false,
			.description = "Testing incremental string lengths"
		};
		
		if (run_strlen_test(&test, i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sString arrays: %d/%d passed%s\n", 
			passed == num_strings ? GREEN : RED, passed, num_strings, END);
}

/** Measures ft_strlen against libc for a coarse performance comparison. */
static void test_performance(void) {
	dprintf(1, "\n%s--- Performance Test ---%s\n", CYAN, END);
	
	const char *long_string = "This is a moderately long string that we'll use for performance testing";

	clock_t start, end;
	
	start = clock();
	for (int i = 0; i < 1000000; i++) {
		(void)ft_strlen(long_string);
	}
	end = clock();
	double ft_time = ((double)(end - start)) / CLOCKS_PER_SEC;
	
	start = clock();
	for (int i = 0; i < 1000000; i++) {
		(void)strlen(long_string);
	}
	end = clock();
	double orig_time = ((double)(end - start)) / CLOCKS_PER_SEC;
	
	dprintf(1, "  ft_strlen: %.3f seconds\n", ft_time);
	dprintf(1, "  strlen:    %.3f seconds\n", orig_time);
	dprintf(1, "  Ratio:     %.2fx\n", ft_time / orig_time);
	
	if (ft_time <= orig_time * 1.5) {
		dprintf(1, "  %s✓ Performance acceptable%s\n", GREEN, END);
	} else {
		dprintf(1, "  %s✗ Too slow%s\n", RED, END);
	}
}

/** Launches the complete ft_strlen test suite. */
void inject_data_strlen(void) {
	dprintf(1, "\n%s========================================%s\n", BLUE, END);
	dprintf(1, "%s         TESTING STRLEN%s\n", BLUE, END);
	dprintf(1, "%s========================================%s\n\n", BLUE, END);

	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s         SUITE 1: Normal Strings%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_normal_strings();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 2: Special Characters%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_special_characters();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s         SUITE 3: Edge Cases%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_edge_cases();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s        SUITE 4: String Arrays%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_string_arrays();

	test_performance();
	
	dprintf(1, "\n%s========================================%s\n", BLUE, END);
	dprintf(1, "%s           ALL TESTS COMPLETE%s\n", BLUE, END);
	dprintf(1, "%s========================================%s\n\n", BLUE, END);
}