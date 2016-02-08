#include <stdio.h>

int main( int argc, char* argv[])
{
	//open the file as a command line argument
	if(argc > 1)
	{
		FILE *doc = fopen( argv[1], "r");
	}
	return 0;
}