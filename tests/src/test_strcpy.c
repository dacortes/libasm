#include <tests.h>

static bool loop_test(void)
{
	// char dest[3];
	// char src[] = "hello";  // 5 chars + null = 6 bytes
	char dest[10];
	char src[] = {'H','i'};
	
	dprintf(1, "\t result: %s errno: %d\n", strcpy(dest, src), errno);
	//superposicion de memoria
	char buffer[20] = "Hello World";
	dprintf(1, "\t result: %s errno: %d\n", strcpy(buffer + 2, buffer), errno);
	return true;
}

void inject_data_strcpy(void)
{
	loop_test();
	dprintf(1, "\t result %s\n", ft_strcpy("hola", "mundo"));
}