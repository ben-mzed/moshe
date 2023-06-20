#include "Header.h"
#include <stdio.h>

#ifdef _TESTS

bool AddNumbers_UT(void)
{
	int num1 = 10;
	int num2 = 20;
	int result = AddNumbers(num1, num2);
	if ((num1 + num2) != result)
	{
		return false;
	}
	
	return true;
}

int main(void)
{
	if (!AddNumbers_UT())
	{
		printf("UT error");
	}
	return 0;
}
#endif