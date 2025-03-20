#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

int main(int argc, char* argv[])
{
	/* To be completed */
	FILE* file=NULL;

	if (argc<2) {
		fprintf(stderr,"Usage: %s <file_name>\n",argv[0]);
		exit(1);
	}

	/* Open file */
	if ((file = fopen(argv[1], "w")) == NULL)
		err(2,"The input file %s could not be opened",argv[1]);


	int i = 2;
	while (i < argc) {
		size_t length = strlen(argv[i])+1;
		if(fwrite(argv[i], sizeof(char), length, file) != length){
			perror("Boo");
			fclose(file);
			return EXIT_FAILURE;
		}
		i++;
	}

	fclose(file);
	return 0;
}
