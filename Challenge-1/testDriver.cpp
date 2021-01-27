#include <iostream>
#include <string>

#include "lfsr.h"

using namespace std;

/************* Utility ***************/
void print_data(unsigned char* data, int len)
{
	if(len == 0)
	{
		printf("\n---\n");
	}
	else if(data == NULL) // and len isn't 0
	{
		fprintf(stderr, "\nERROR: cannot print data, null value with non-zero length\n");
	}
	else if(len < 0)
	{
		fprintf(stderr, "\nERROR: cannot print data, negative length\n");
	}
	else 
	{	
		printf("\n");
		for(int i = 0; i < len; i++)
		{
			printf("%x ", data[i]);
		}
		printf("\n");
	}

}

int main() 
{
	int dataLength = 5;
	unsigned char* data = (unsigned char*)malloc(dataLength*sizeof(unsigned char));
	data[0] = 'a';
	data[1] = 'p';
	data[2] = 'p';
	data[3] = 'l';
	data[4] = 'e';

	unsigned int key = 0x12345678;

	printf("Original Data:\t");
	print_data(data, dataLength);
	printf("\n");

	data = Crypt(data, dataLength, key);

	printf("Encrypt Data:\t");
	print_data(data, dataLength);
	printf("\n");

	data = Crypt(data, dataLength, key);
	printf("Back again:\t");
	print_data(data, dataLength);

	free(data);

	return 0;
}

/*
int main()
{
	unsigned int a = 256;
	unsigned char* b = (unsigned char*)&a;
	unsigned char* c = (unsigned char*)malloc(4*1);
	for(int i = 0; i < 4; i++)
	{
		c[i] = b[3-i];
	}
	print_data(b, 4);

	return 0;
}
*/
