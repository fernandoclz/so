#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor
 *
 * The loadstr() function must allocate memory from the heap to store
 * the contents of the string read from the FILE.
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc())
 *
 * Returns: !=NULL if success, NULL if error
 */
char *loadstr(FILE *file)
{
	/* To be completed */

	int cont=0;

	while(getc(file) != '\0'){
		cont++;
	}

	if(cont == 0){
		return NULL;
	}
	cont++;

	fseek(file, -cont, SEEK_CUR);	//mover puntero al principio

	char *polopolo = malloc((cont)*sizeof(char)); 	//reserva memoria

	size_t leido = fread(polopolo, sizeof(char), cont, file);

	if(leido < cont){
		free(polopolo);
		return NULL;
	}
	
	return polopolo;	
}

int main(int argc, char* argv[]) {
	FILE* file=NULL;

	if (argc!=2) {
		fprintf(stderr,"Usage: %s <file_name>\n",argv[0]);
		exit(1);
	}

	/* Open file */
	if ((file = fopen(argv[1], "r")) == NULL)
		err(2,"The input file %s could not be opened",argv[1]);

	char *str = NULL;
	do {
		/* Print char to stdout */
		str = loadstr(file);
		if(!str)
			break;

		printf("%s\n", str);
	} while(str != NULL);
	free(str);
	fclose(file);
	return 0;
}
