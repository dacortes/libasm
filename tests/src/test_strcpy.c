#include <tests.h>

static bool check_null(char *(*func)(char *, const char *), char *dst, const char *src)
{
	struct sigaction sa_segv, sa_abrt;
	struct sigaction old_sa_segv, old_sa_abrt;

	// memset(&sa_segv, 0, sizeof(sa_segv));
	sa_segv.sa_sigaction = handle_sigsegv;
	sigemptyset(&sa_segv.sa_mask);
	sa_segv.sa_flags = SA_SIGINFO | SA_NODEFER;
	
	// memset(&sa_abrt, 0, sizeof(sa_abrt));
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
		func(dst, src);
		had_error = false;
	} else 
		had_error = true;
	
	sigaction(SIGSEGV, &old_sa_segv, NULL);
	sigaction(SIGABRT, &old_sa_abrt, NULL);
	
	return had_error;
}

bool loop_test(void)
{
	bool all_ok = true;

	char *dst = malloc(4096);
	const char *src = calloc(2, 4096);
	memset(src, 'a', (4096 * 2) - 1);
	check_null(strcpy, dst, src);

	return all_ok;
}

void inject_data_strcpy(void)
{
	// loop_test();
	// char dst[4];

	// dprintf(1, "\t result %s\n", ft_strcpy(dst, "mundo"));
	loop_test();
}