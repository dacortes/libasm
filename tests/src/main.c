#include <tests.h>

int	main(void)
{
	// inject_data_strlen();
	// inject_data_strcpy();
	dprintf(1, "result strcmp=%d\n", strcmp("hola", "holam"));
	dprintf(1, "test ft_strcmp=%i\n", ft_strcmp("hola", "holam"));
	return 0;
}