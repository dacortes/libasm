/**
 * @file main.c
 * @brief Entry point for the libasm validation executable.
 */

#include <tests.h>

/**
 * @brief Executes every test suite exposed by the repository.
 *
 * @return 0 after all suites have been launched.
 */
int	main(void)
{
	inject_data_strlen();
	inject_data_strcpy();
	inject_data_strcmp();
	inject_data_write();
	inject_data_read();
	inject_data_strdup();
	return 0;
}
