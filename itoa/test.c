#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char * utoa(int digit, char *buff) {
	while(digit) {
		*--buff = digit % 10 + '0';
		digit/= 10;
	}
	return buff;
}
int main(void) {
	char *buff = malloc(8);
	int digit = 123;
	buff = utoa(digit, &buff[7]);

	printf("%s\n", buff);
	return 0;
}