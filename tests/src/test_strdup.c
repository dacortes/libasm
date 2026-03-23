#include <tests.h>

// Structure for strdup test cases
typedef struct {
	const char *name;
	const char *input;
	char *expected_result;      // Expected duplicated string (NULL for crash)
	int expected_errno;          // Expected errno (0 for success)
	bool should_crash;           // Whether we expect a crash
	const char *description;
} StrdupTestCase;

// Macro to create test cases
#define STRDUP_TEST_CASE(name_str, input_str, expected, err, crash, desc) \
	{ \
		.name = name_str, \
		.input = input_str, \
		.expected_result = expected, \
		.expected_errno = err, \
		.should_crash = crash, \
		.description = desc \
	}

// Helper to check if strdup crashes
static bool check_strdup_crash(char *(*func)(const char *), const char *s)
{
	pid_t pid = fork();
	
	if (pid == ERROR) {
		dprintf(1, "%s Failed to create child process\n", STR_ERR);
		return false;
	}
	
	if (pid == 0) {
		// Child process
		func(s);
		_exit(0);
	} else {
		int status;
		waitpid(pid, &status, 0);
		
		if (WIFSIGNALED(status)) {
			return true;  // Crashed
		}
		return false;     // Did not crash
	}
}

// Helper to clear errno
static void clear_errno(void)
{
	errno = 0;
}

bool is_valid_memory(char *ptr, size_t size)
{
	if (ptr == NULL)
		return false;
	
	volatile char c = ptr[0];
	(void)c;
	return true;
}

// Run a single test case
static bool run_strdup_test(StrdupTestCase *test, int index)
{
	dprintf(1, "\n%s--- Test %d: %s ---%s\n", CYAN, index + 1, test->name, END);
	
	dprintf(1, "  Description: %s\n", test->description);
	dprintf(1, "  Input: \"%s\"\n", test->input ? test->input : "NULL");
	dprintf(1, "  Expected result: \"%s\"\n", 
			test->expected_result ? test->expected_result : "NULL (should crash)");
	dprintf(1, "  Expected errno: %d (%s)\n", test->expected_errno,
			test->expected_errno ? strerror(test->expected_errno) : "none");
	dprintf(1, "  Should crash: %s\n", test->should_crash ? "YES" : "NO");
	
	bool my_crashed = false;
	bool orig_crashed = false;
	char *my_result = NULL;
	char *orig_result = NULL;
	int my_errno_saved = 0;
	int orig_errno_saved = 0;
	
	if (!test->should_crash) {
		clear_errno();
		my_result = ft_strdup(test->input);
		my_errno_saved = errno;
		
		clear_errno();
		orig_result = strdup(test->input);
		orig_errno_saved = errno;
	} else {
		my_crashed = check_strdup_crash(ft_strdup, test->input);
		orig_crashed = check_strdup_crash(strdup, test->input);
	}
	
	dprintf(1, "  Results:\n");
	
	if (!test->should_crash) {
		dprintf(1, "    ft_strdup: %p (\"%s\"), errno=%d (%s)\n", 
				my_result, my_result ? my_result : "NULL",
				my_errno_saved, strerror(my_errno_saved));
		dprintf(1, "    strdup:    %p (\"%s\"), errno=%d (%s)\n", 
				orig_result, orig_result ? orig_result : "NULL",
				orig_errno_saved, strerror(orig_errno_saved));
	} else {
		dprintf(1, "    ft_strdup: %s\n", my_crashed ? "CRASHED" : "OK");
		dprintf(1, "    strdup:    %s\n", orig_crashed ? "CRASHED" : "OK");
	}
	
	bool correct = true;
	
	// Check crash behavior
	if (test->should_crash) {
		if (my_crashed != orig_crashed) {
			dprintf(1, "  %s✗ Different crash behavior: ft=%s, orig=%s%s\n",
					RED,
					my_crashed ? "CRASH" : "NO CRASH",
					orig_crashed ? "CRASH" : "NO CRASH",
					END);
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
	
	// Check errno
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
	
	// Check if memory was allocated
	if (my_result == NULL && test->expected_result != NULL) {
		dprintf(1, "  %s✗ ft_strdup returned NULL when expecting a string%s\n",
				RED, END);
		correct = false;
	} else if (orig_result == NULL && test->expected_result != NULL) {
		dprintf(1, "  %s✗ strdup returned NULL when expecting a string%s\n",
				RED, END);
		correct = false;
	}
	
	// Compare results
	if (my_result && orig_result) {
		if (strcmp(my_result, orig_result) != 0) {
			dprintf(1, "  %s✗ Content mismatch: ft=\"%s\", orig=\"%s\"%s\n",
					RED, my_result, orig_result, END);
			correct = false;
		} else if (strcmp(my_result, test->expected_result) != 0) {
			dprintf(1, "  %s✗ Wrong content: got \"%s\", expected \"%s\"%s\n",
					RED, my_result, test->expected_result, END);
			correct = false;
		} else {
			dprintf(1, "  %s✓ Content correct: \"%s\"%s\n", GREEN, my_result, END);
		}
		
		// Check if memory is independent (modify one shouldn't affect the other)
		if (my_result && orig_result && my_result != orig_result) {
			char original = my_result[0];
			if (original != '\0') {
				my_result[0] = 'X';
				if (orig_result[0] == original) {
					dprintf(1, "  %s✓ Memory is independent (modifying ft doesn't affect orig)%s\n",
							GREEN, END);
				} else {
					dprintf(1, "  %s✗ Memory is shared (modifying ft affected orig)%s\n",
							RED, END);
					correct = false;
				}
				my_result[0] = original;  // Restore
			}
		}
		
		// Free allocated memory
		free(my_result);
		free(orig_result);
	}
	
	if (correct) {
		dprintf(1, "  %s✓ Test PASSED%s\n", GREEN, END);
	} else {
		dprintf(1, "  %s✗ Test FAILED%s\n", RED, END);
	}
	
	return correct;
}

// Test 1: Normal strings
static void test_normal_strings(void)
{
	dprintf(1, "\n%s--- Normal Strings ---%s\n", CYAN, END);
	
	StrdupTestCase tests[] = {
		STRDUP_TEST_CASE("Empty string", "", "", 0, false,
						"Empty string should duplicate to empty string"),
		
		STRDUP_TEST_CASE("Single character", "a", "a", 0, false,
						"Single character string"),
		
		STRDUP_TEST_CASE("Simple word", "hello", "hello", 0, false,
						"Regular word"),
		
		STRDUP_TEST_CASE("With spaces", "hello world", "hello world", 0, false,
						"String with spaces"),
		
		STRDUP_TEST_CASE("With punctuation", "Hello, World!", "Hello, World!", 0, false,
						"String with punctuation"),
		
		STRDUP_TEST_CASE("Numbers", "1234567890", "1234567890", 0, false,
						"String of numbers"),
		
		STRDUP_TEST_CASE("Mixed", "abc123!@#", "abc123!@#", 0, false,
						"Mixed characters"),
		
		STRDUP_TEST_CASE("Long string", 
			"This is a very long string that should still work correctly", 
			"This is a very long string that should still work correctly", 
			0, false, "Long string duplication"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_strdup_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sNormal strings: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

// Test 2: Special characters
static void test_special_characters(void)
{
	dprintf(1, "\n%s--- Special Characters ---%s\n", CYAN, END);
	
	StrdupTestCase tests[] = {
		STRDUP_TEST_CASE("String with newline", "hello\nworld", "hello\nworld", 0, false,
						"String with newline character"),
		
		STRDUP_TEST_CASE("String with tab", "hello\tworld", "hello\tworld", 0, false,
						"String with tab character"),
		
		STRDUP_TEST_CASE("String with null inside", "hello\0world", "hello", 0, false,
						"String with embedded null (should stop at null)"),
		
		STRDUP_TEST_CASE("Only null terminator", "\0", "", 0, false,
						"String that starts with null"),
		
		STRDUP_TEST_CASE("UTF-8: áéíóú", "áéíóú", "áéíóú", 0, false,
						"UTF-8 accented characters"),
		
		STRDUP_TEST_CASE("UTF-8: 日本", "日本", "日本", 0, false,
						"UTF-8 Japanese characters"),
		
		STRDUP_TEST_CASE("UTF-8: 🌟", "🌟", "🌟", 0, false,
						"UTF-8 emoji"),
		
		STRDUP_TEST_CASE("Mixed UTF-8", "Hello 世界 🌟", "Hello 世界 🌟", 0, false,
						"Mixed ASCII and UTF-8"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_strdup_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sSpecial characters: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

// Test 3: Edge cases
static void test_edge_cases(void)
{
	dprintf(1, "\n%s--- Edge Cases ---%s\n", CYAN, END);
	
	// Test with huge string (1000 characters)
	char huge_string[1001];
	memset(huge_string, 'A', 1000);
	huge_string[1000] = '\0';
	
	StrdupTestCase tests[] = {
		STRDUP_TEST_CASE("NULL pointer", NULL, NULL, 0, true,
						"NULL pointer should crash (segfault)"),
		
		STRDUP_TEST_CASE("Very large string", huge_string, huge_string, 0, false,
						"Very large string (1000 chars)"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_strdup_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sEdge cases: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

// Test 4: Memory independence test
static void test_memory_independence(void)
{
	dprintf(1, "\n%s--- Memory Independence ---%s\n", CYAN, END);
	
	const char *original = "test string";
	
	char *my_dup = ft_strdup(original);
	char *orig_dup = strdup(original);
	
	if (my_dup && orig_dup) {
		dprintf(1, "  Original string: \"%s\" at %p\n", original, (void*)original);
		dprintf(1, "  ft_strdup: %p\n", (void*)my_dup);
		dprintf(1, "  strdup:    %p\n", (void*)orig_dup);
		
		bool correct = true;
		
		// Check different addresses
		if (my_dup == (char*)original) {
			dprintf(1, "  %s✗ ft_strdup returned the original pointer (no allocation)%s\n",
					RED, END);
			correct = false;
		}
		
		if (orig_dup == (char*)original) {
			dprintf(1, "  %s✗ strdup returned the original pointer (no allocation)%s\n",
					RED, END);
			correct = false;
		}
		
		// Check that they are different from each other
		if (my_dup == orig_dup) {
			dprintf(1, "  %s✗ Both functions returned the same pointer%s\n",
					RED, END);
			correct = false;
		} else {
			dprintf(1, "  %s✓ Different pointers (independent allocations)%s\n",
					GREEN, END);
		}
		
		// Modify one and check the other remains unchanged
		if (my_dup && orig_dup && my_dup != orig_dup) {
			char saved = my_dup[0];
			my_dup[0] = 'X';
			
			if (orig_dup[0] == saved) {
				dprintf(1, "  %s✓ Modifying ft_strdup result doesn't affect strdup result%s\n",
						GREEN, END);
			} else {
				dprintf(1, "  %s✗ Modifying ft_strdup result affected strdup result%s\n",
						RED, END);
				correct = false;
			}
			
			my_dup[0] = saved;
		}
		
		free(my_dup);
		free(orig_dup);
		
		if (correct) {
			dprintf(1, "\n  %s✓ Memory independence test PASSED%s\n", GREEN, END);
		} else {
			dprintf(1, "\n  %s✗ Memory independence test FAILED%s\n", RED, END);
		}
	} else {
		dprintf(1, "  %sFailed to allocate memory for test%s\n", RED, END);
	}
}

// Test 5: Multiple allocations (memory leak test)
static void test_multiple_allocations(void)
{
	dprintf(1, "\n%s--- Multiple Allocations ---%s\n", CYAN, END);
	
	const char *strings[] = {
		"first",
		"second",
		"third",
		"fourth",
		"fifth",
		"this is a much longer string to test with"
	};
	
	int num_strings = sizeof(strings) / sizeof(strings[0]);
	char *my_dups[num_strings];
	char *orig_dups[num_strings];
	bool all_ok = true;
	
	dprintf(1, "  Allocating %d strings...\n", num_strings);
	
	// Allocate all
	for (int i = 0; i < num_strings; i++) {
		my_dups[i] = ft_strdup(strings[i]);
		orig_dups[i] = strdup(strings[i]);
		
		if (!my_dups[i] || !orig_dups[i]) {
			dprintf(1, "  %s✗ Allocation failed at index %d%s\n", RED, i, END);
			all_ok = false;
			break;
		}
		
		if (strcmp(my_dups[i], strings[i]) != 0) {
			dprintf(1, "  %s✗ Wrong content at index %d: \"%s\" vs \"%s\"%s\n",
					RED, i, my_dups[i], strings[i], END);
			all_ok = false;
		}
	}
	
	// Free all
	for (int i = 0; i < num_strings; i++) {
		if (my_dups[i]) free(my_dups[i]);
		if (orig_dups[i]) free(orig_dups[i]);
	}
	
	if (all_ok) {
		dprintf(1, "  %s✓ Multiple allocations test PASSED%s\n", GREEN, END);
	} else {
		dprintf(1, "  %s✗ Multiple allocations test FAILED%s\n", RED, END);
	}
}

// Test 6: Stress test (many allocations)
static void test_stress(void)
{
	dprintf(1, "\n%s--- Stress Test ---%s\n", CYAN, END);
	
	const char *test_string = "Stress test string";
	int iterations = 10000;
	bool all_ok = true;
	
	dprintf(1, "  Performing %d allocations and frees...\n", iterations);
	
	for (int i = 0; i < iterations; i++) {
		char *my_dup = ft_strdup(test_string);
		char *orig_dup = strdup(test_string);
		
		if (!my_dup || !orig_dup) {
			dprintf(1, "  %s✗ Allocation failed at iteration %d%s\n", RED, i, END);
			all_ok = false;
			break;
		}
		
		if (strcmp(my_dup, test_string) != 0 || strcmp(orig_dup, test_string) != 0) {
			dprintf(1, "  %s✗ Content mismatch at iteration %d%s\n", RED, i, END);
			all_ok = false;
			free(my_dup);
			free(orig_dup);
			break;
		}
		
		free(my_dup);
		free(orig_dup);
	}
	
	if (all_ok) {
		dprintf(1, "  %s✓ Stress test PASSED (%d iterations)%s\n", 
				GREEN, iterations, END);
	} else {
		dprintf(1, "  %s✗ Stress test FAILED%s\n", RED, END);
	}
}

// Test 7: Very large string (memory limit test)
static void test_very_large_string(void)
{
	dprintf(1, "\n%s--- Very Large String ---%s\n", CYAN, END);
	
	size_t large_size = 1024 * 1024;  // 1MB
	char *large_string = malloc(large_size + 1);
	
	if (large_string) {
		memset(large_string, 'A', large_size);
		large_string[large_size] = '\0';
		
		dprintf(1, "  Testing with %zu byte string...\n", large_size);
		
		char *my_dup = ft_strdup(large_string);
		char *orig_dup = strdup(large_string);
		
		if (my_dup && orig_dup) {
			if (strcmp(my_dup, large_string) == 0 && 
				strcmp(orig_dup, large_string) == 0) {
				dprintf(1, "  %s✓ Large string duplication successful%s\n", GREEN, END);
			} else {
				dprintf(1, "  %s✗ Large string content mismatch%s\n", RED, END);
			}
			
			free(my_dup);
			free(orig_dup);
		} else {
			dprintf(1, "  %s✗ Failed to allocate for large string%s\n", RED, END);
		}
		
		free(large_string);
	} else {
		dprintf(1, "  %sFailed to allocate test string%s\n", YELLOW, END);
	}
}

// Main test function
void inject_data_strdup(void)
{
	dprintf(1, "\n%s========================================%s\n", BLUE, END);
	dprintf(1, "%s         TESTING STRDUP%s\n", BLUE, END);
	dprintf(1, "%s========================================%s\n\n", BLUE, END);
	
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 1: Normal Strings%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_normal_strings();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 2: Special Characters%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_special_characters();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 3: Edge Cases%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_edge_cases();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 4: Memory Independence%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_memory_independence();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 5: Multiple Allocations%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_multiple_allocations();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 6: Stress Test%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_stress();
	
	dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	dprintf(1, "%s      SUITE 7: Very Large String%s\n", BLUE, END);
	dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
	test_very_large_string();
	
	dprintf(1, "\n%s========================================%s\n", BLUE, END);
	dprintf(1, "%s           ALL TESTS COMPLETE%s\n", BLUE, END);
	dprintf(1, "%s========================================%s\n\n", BLUE, END);
}