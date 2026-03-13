#include <tests.h>

static sigjmp_buf env;

void	segfault_handler(int sig, siginfo_t *info, void *context)
{
	(void)sig;
	(void)info;
	(void)context;
	siglongjmp(env, 1);
}


bool	check_null(size_t (*func)(const char *), const char *param)
{
	struct sigaction sa, old_sa;

	sa.sa_sigaction = segfault_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO | SA_NODEFER;

	if (sigaction(SIGSEGV, &sa, &old_sa) == ERROR) {
		perror(ERR_SIG);
		return false;
	}

	bool	segf = false;

	if (sigsetjmp(env, 1) == 0) {
		func(param);
		segf = false;
	}
	else
		segf = true;

	sigaction(SIGSEGV, &old_sa, NULL);
	return segf;
}

static bool loop_test(const char *data[], int len)
{
	if (!data || len <= 0) {
		dprintf(1, "%s invalid data\n", STR_ERR);
		return false;
	}

	bool	all_ok = true;

	for (int i = 0; i < len; i++) {
		bool	my = check_null(ft_strlen, data[i]);
		bool	orig = check_null(strlen, data[i]);

		const char *tmp = data[i];
		if (!tmp)
			tmp = "NULL";

		if (my == orig)
			dprintf(1, "\t%s Same behavior: param: *%s%s%s*\n", STR_SCC, BLUE, tmp, END);
		else {
			dprintf(1, "\t%s It doesn't behave the same way: param: *%s%s%s*\n", STR_ERR, RED, tmp, END);
			all_ok = false;
		}
		
		if (data[i] == NULL)
			continue ;

		size_t len_my = ft_strlen(data[i]);
		size_t len_orig = strlen(data[i]);

		if (len_my == len_orig)
			dprintf(1, "\t%s The len is the same: param: %s%s%s len_my = %ld len_orig = %ld\n", STR_SCC, BLUE, tmp, END, len_my, len_orig);
		else {
			dprintf(1, "\t%s It does not have the same lens: param: %s%s%s len_my = %ld len_orig = %ld\n", STR_ERR, RED, tmp, END, len_my, len_orig);
			all_ok = false;
		}
	}

	if (all_ok)
		dprintf(1, "\t%s All tests for valid strings passed\n", STR_SCC);
	else
		dprintf(1, "\t%s Some tests for valid strings failed\n", STR_ERR);
	return all_ok;
}

void inject_data_strlen(void) {
	const char *simple_data[] = {"", "a", "hello", "hello world!", NULL};

	dprintf(1, "%s Test simple data\n", STR_INF);
	if (!loop_test(simple_data, 5))
		exit(ERROR);

	const char *null_data[] = {NULL, "", NULL, "", NULL, "", NULL, "", NULL, NULL};
	dprintf(1, "%s Test null data\n", STR_INF);
	if (!loop_test(null_data, 10))
		exit(ERROR);
}