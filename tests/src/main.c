#include <tests.h>

int	main(void)
{
	inject_data_strlen();
	inject_data_strcpy();
	inject_data_strcmp();
	// dprintf(1, "my = %d -- orig = %d\n", ft_strcmp("", ""), strcmp("", ""));
	// dprintf(1, "my = %d -- orig = %d\n", ft_strcmp("abc\0def", "abc\0def"), ft_strcmp("abc\0def", "abc\0def"));
	return 0;
}