/**
 * @file test_strcmp.c
 * @brief Validation scenarios for the ft_strcmp assembly implementation.
 */

#include <tests.h>

typedef struct {
	const char *name;
	const char *s1;
	const char *s2;
	int expected_result;
	int expected_sign; 
	bool should_crash;
	const char *description;
} StrcmpTestCase;

#define STRCMP_TEST_CASE_SIGN(name_str, input1, input2, sign, crash, desc) \
{ \
	.name = name_str, \
	.s1 = input1, \
	.s2 = input2, \
	.expected_sign = sign, \
	.should_crash = crash, \
	.description = desc \
}

#define STRCMP_TEST_CASE_EXACT(name_str, input1, input2, expected, crash, desc) \
{ \
	.name = name_str, \
	.s1 = input1, \
	.s2 = input2, \
	.expected_result = expected, \
	.expected_sign = (expected > 0) ? 1 : (expected < 0) ? -1 : 0, \
	.should_crash = crash, \
	.description = desc \
}

/** Normalizes an integer comparison result to -1, 0, or 1. */
static int get_sign(int val) {
    if (val > 0)
		return 1;
    if (val < 0)
		return -1;
    return 0;
}

/** Executes a strcmp-like function while capturing crash signals for invalid input tests. */
bool check_null(int (*func)(const char *, const char *), const char *s1, const char *s2)
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
		func(s1, s2);
		had_error = false;
	} else 
		had_error = true;
	
	sigaction(SIGSEGV, &old_sa_segv, NULL);
	sigaction(SIGABRT, &old_sa_abrt, NULL);
	
	return had_error;
}

/** Runs one ft_strcmp scenario and compares it against libc strcmp. */
static bool run_strcmp_test(StrcmpTestCase *test, int index) {
	dprintf(1, "\n%s--- Test %d: %s ---%s\n", CYAN, index + 1, test->name, END);
	
	dprintf(1, "  s1: \"%s\"\n", test->s1 ? test->s1 : "NULL");
	dprintf(1, "  s2: \"%s\"\n", test->s2 ? test->s2 : "NULL");
	dprintf(1, "  Expected sign: %s\n", 
			test->expected_sign == 0 ? "0 (equal)" :
			test->expected_sign > 0 ? "positive (s1 > s2)" : "negative (s1 < s2)");
	dprintf(1, "  Should crash: %s\n", test->should_crash ? "YES" : "NO");
	dprintf(1, "  Description: %s\n", test->description);
	
	bool my_crashed = false;
	bool orig_crashed = false;
	int my_result = 0;
	int orig_result = 0;

	if (!test->should_crash) {
		my_result = ft_strcmp(test->s1, test->s2);
		orig_result = strcmp(test->s1, test->s2);
	} else {
		my_crashed = check_null(ft_strcmp, test->s1, test->s2);
		orig_crashed = check_null(strcmp, test->s1, test->s2);
	}

	dprintf(1, "  Results:\n");
	dprintf(1, "    ft_strcmp: %d (sign: %d)\n", my_result, get_sign(my_result));
	dprintf(1, "    strcmp:    %d (sign: %d)\n", orig_result, get_sign(orig_result));

	bool correct = true;

	if (my_crashed != orig_crashed) {
		dprintf(1, "  %s✗ Different crash behavior: ft=%s, orig=%s%s\n", 
			RED,
			my_crashed ? "CRASH" : "NO CRASH",
			orig_crashed ? "CRASH" : "NO CRASH",
			END);
		correct = false;
	}

	if (test->should_crash) {
		if (my_crashed) {
			dprintf(1, "  %s✓ Correctly crashed%s\n", GREEN, END);
		} else {
			dprintf(1, "  %s✗ Should have crashed but didn't%s\n", RED, END);
			correct = false;
		}
		return correct;
	}

	int my_sign = get_sign(my_result);
	int orig_sign = get_sign(orig_result);

	if (my_sign != orig_sign) {
		dprintf(1, "  %s✗ Sign mismatch: ft=%d, orig=%d%s\n", 
				RED, my_sign, orig_sign, END);
		correct = false;
	}

	if (my_sign != test->expected_sign) {
		dprintf(1, "  %s✗ Wrong sign: expected %d, got %d%s\n",
				RED, test->expected_sign, my_sign, END);
		correct = false;
	}

	if (test->expected_result != 0 && !test->should_crash) {
		if ((my_result > 0) != (test->expected_result > 0)) {
			dprintf(1, "  %s✗ Wrong direction%s\n", RED, END);
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

/** Covers strings that should compare as equal. */
static void test_identical_strings(void) {
	dprintf(1, "\n%s--- Identical Strings ---%s\n", CYAN, END);
	
	StrcmpTestCase tests[] = {
		STRCMP_TEST_CASE_SIGN("Both empty", "", "", 0, false,
							"Empty strings should be equal"),
		
		STRCMP_TEST_CASE_SIGN("Single char", "a", "a", 0, false,
							"Same single character"),
		
		STRCMP_TEST_CASE_SIGN("Simple word", "hello", "hello", 0, false,
							"Same word"),
		
		STRCMP_TEST_CASE_SIGN("With spaces", "hello world", "hello world", 0, false,
							"Same string with spaces"),
		
		STRCMP_TEST_CASE_SIGN("Numbers", "12345", "12345", 0, false,
							"Same numbers"),
		
		STRCMP_TEST_CASE_SIGN("Special chars", "!@#$%", "!@#$%", 0, false,
							"Same special characters"),
		
		STRCMP_TEST_CASE_SIGN("Long string", 
			"This is a very long string for testing",
			"This is a very long string for testing",
			0, false, "Same long string"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_strcmp_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sIdentical strings: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

/** Covers common non-equal string comparisons. */
static void test_different_strings(void) {
	dprintf(1, "\n%s--- Different Strings ---%s\n", CYAN, END);
	
	StrcmpTestCase tests[] = {
		STRCMP_TEST_CASE_SIGN("First char diff", "a", "b", -1, false,
							"'a' < 'b' → negative"),
		
		STRCMP_TEST_CASE_SIGN("First char diff reverse", "b", "a", 1, false,
							"'b' > 'a' → positive"),
		
		STRCMP_TEST_CASE_SIGN("Different length - s1 shorter", "abc", "abcd", -1, false,
							"Shorter string with same prefix is smaller"),
		
		STRCMP_TEST_CASE_SIGN("Different length - s1 longer", "abcd", "abc", 1, false,
							"Longer string with same prefix is larger"),
		
		STRCMP_TEST_CASE_SIGN("Different at position 2", "abx", "aby", -1, false,
							"'x' < 'y' at position 2"),
		
		STRCMP_TEST_CASE_SIGN("Different case - uppercase", "Hello", "hello", -1, false,
							"Uppercase < lowercase in ASCII"),
		
		STRCMP_TEST_CASE_SIGN("Different case - lowercase", "hello", "Hello", 1, false,
							"Lowercase > uppercase in ASCII"),
		
		STRCMP_TEST_CASE_SIGN("Numbers vs letters", "9", "A", -1, false,
							"'9' (57) < 'A' (65)"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_strcmp_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sDifferent strings: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

/** Covers control characters, extended bytes, and UTF-8 byte sequences. */
static void test_special_characters(void) {
	dprintf(1, "\n%s--- Special Characters ---%s\n", CYAN, END);
	
	StrcmpTestCase tests[] = {
		STRCMP_TEST_CASE_SIGN("Null char in string", "abc\0def", "abc\0def", 0, false,
							"Strings with embedded null (should compare up to null)"),
		
		STRCMP_TEST_CASE_SIGN("Control chars - tab", "a\tb", "a b", -1, false,
							"Tab (9) < space (32)"),
		
		STRCMP_TEST_CASE_SIGN("Extended ASCII", "\x80", "\x81", -1, false,
							"Extended ASCII comparison"),
		
		STRCMP_TEST_CASE_SIGN("UTF-8: á vs é", "á", "é", -1, false,
							"á (0xC3 0xA1) vs é (0xC3 0xA9) - compares byte by byte"),
		
		STRCMP_TEST_CASE_SIGN("UTF-8: ñ vs n", "ñ", "n", 1, false,
							"ñ (multi-byte) > n (single byte) - byte comparison"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_strcmp_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sSpecial characters: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

/** Validates exact subtraction results for representative byte pairs. */
static void test_exact_differences(void) {
	dprintf(1, "\n%s--- Exact Differences ---%s\n", CYAN, END);
	
	StrcmpTestCase tests[] = {
		STRCMP_TEST_CASE_EXACT("'a' vs 'b'", "a", "b", -1, false,
							"Exact difference: 'a'(97) - 'b'(98) = -1"),
		
		STRCMP_TEST_CASE_EXACT("'b' vs 'a'", "b", "a", 1, false,
							"Exact difference: 'b'(98) - 'a'(97) = 1"),
		
		STRCMP_TEST_CASE_EXACT("'A' vs 'a'", "A", "a", -32, false,
							"ASCII difference: 'A'(65) - 'a'(97) = -32"),
		
		STRCMP_TEST_CASE_EXACT("'0' vs '9'", "0", "9", -9, false,
							"'0'(48) - '9'(57) = -9"),
		
		STRCMP_TEST_CASE_EXACT("First diff at pos 3", "abcx", "abcy", -1, false,
							"'x'(120) - 'y'(121) = -1"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_strcmp_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sExact differences: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

/** Covers NULL pointers and very long strings. */
static void test_edge_cases(void) {
	dprintf(1, "\n%s--- Edge Cases ---%s\n", CYAN, END);
	
	StrcmpTestCase tests[] = {
		STRCMP_TEST_CASE_SIGN("NULL vs NULL", NULL, NULL, 0, true,
							"Both NULL should crash"),
		
		STRCMP_TEST_CASE_SIGN("NULL vs empty", NULL, "", 0, true,
							"NULL s1 should crash"),
		
		STRCMP_TEST_CASE_SIGN("empty vs NULL", "", NULL, 0, true,
							"NULL s2 should crash"),
		
		STRCMP_TEST_CASE_SIGN("Very long vs itself", 
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
			0, false, "Very long identical strings"),
		
		STRCMP_TEST_CASE_SIGN("One char diff at end", 
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab",
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac",
			-1, false, "Long strings with last char different"),
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	
	for (int i = 0; i < num_tests; i++) {
		if (run_strcmp_test(&tests[i], i)) {
			passed++;
		}
	}
	
	dprintf(1, "\n%sEdge cases: %d/%d passed%s\n", 
			passed == num_tests ? GREEN : RED, passed, num_tests, END);
}

// static void test_performance(void) {
// 	dprintf(1, "\n%s--- Performance Test ---%s\n", CYAN, END);
	
// 	const char *s1 = "This is a test string for performance comparison";
// 	const char *s2 = "This is a test string for performance comparison";
	
// 	clock_t start, end;
// 	volatile int result = 0;
	
// 	start = clock();
// 	for (int i = 0; i < 1000000; i++) {
// 		result += ft_strcmp(s1, s2);
// 	}
// 	end = clock();
// 	double ft_time = ((double)(end - start)) / CLOCKS_PER_SEC;
	
// 	start = clock();
// 	for (int i = 0; i < 1000000; i++) {
// 		result += strcmp(s1, s2);
// 	}
// 	end = clock();
// 	double orig_time = ((double)(end - start)) / CLOCKS_PER_SEC;
	
// 	dprintf(1, "  ft_strcmp: %.3f seconds\n", ft_time);
// 	dprintf(1, "  strcmp:    %.3f seconds\n", orig_time);
// 	dprintf(1, "  Ratio:     %.2fx\n", ft_time / orig_time);
	
// 	if (ft_time <= orig_time * 1.5) {
// 		dprintf(1, "  %s✓ Performance acceptable%s\n", GREEN, END);
// 	} else {
// 		dprintf(1, "  %s✗ Too slow%s\n", RED, END);
// 	}
// }

/** Launches the complete ft_strcmp test suite. */
void inject_data_strcmp(void) {
    dprintf(1, "\n%s========================================%s\n", BLUE, END);
    dprintf(1, "%s         TESTING STRCMP%s\n", BLUE, END);
    dprintf(1, "%s========================================%s\n\n", BLUE, END);
    
    dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
    dprintf(1, "%s      SUITE 1: Identical Strings%s\n", BLUE, END);
    dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
    test_identical_strings();
    
    dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
    dprintf(1, "%s      SUITE 2: Different Strings%s\n", BLUE, END);
    dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
    test_different_strings();
    
    dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
    dprintf(1, "%s      SUITE 3: Special Characters%s\n", BLUE, END);
    dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
    test_special_characters();
    
    dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
    dprintf(1, "%s      SUITE 4: Exact Differences%s\n", BLUE, END);
    dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
    test_exact_differences();
    
    dprintf(1, "\n%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
    dprintf(1, "%s         SUITE 5: Edge Cases%s\n", BLUE, END);
    dprintf(1, "%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n", BLUE, END);
    test_edge_cases();

    dprintf(1, "\n%s========================================%s\n", BLUE, END);
    dprintf(1, "%s           ALL TESTS COMPLETE%s\n", BLUE, END);
    dprintf(1, "%s========================================%s\n\n", BLUE, END);
}
