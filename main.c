

#include <stdio.h>

int test_function();

int main(void)
{
	int result = test_function();
	printf("Result from assembly: %d\n", result);
	return 0;
}
