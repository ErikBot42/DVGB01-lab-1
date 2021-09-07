#include <stdio.h>

int *pBuffer;

int main() 
{
	int n;
	printf("Number of consumers:");
	scanf("%d", &n);

	int iBufferSize;
	printf("Buffer size:");
	scanf("%d", &iBufferSize);

	int buffer[iBufferSize];
	pBuffer = buffer;


}


