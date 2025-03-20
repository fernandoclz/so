#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void copy(int fdo, int fdd)
{	
	int leido;
	if(fdo==-1 || fdd==-1){
		printf("Error");
		return;
	}
	char array[512];
	while((leido = read(fdo, array, 512)) >0){
		write(fdd, array, leido);
	}
	close(fdo);
	close(fdd);
}

int main(int argc, char *argv[])
{
	int opt;
	char *fichero_entrada = NULL;
	char *fichero_salida = NULL;

	if(argc != 3){
		fprintf(stderr,"Usage: %s <file_name>\n",argv[0]);
		exit(1);
	}

	fichero_entrada = argv[1];
	fichero_salida = argv[2];
	
	if (fichero_entrada == NULL)
	{
		fprintf(stderr, "Must specify one file for argv[1]\n");
		exit(EXIT_FAILURE);
	}
	if (fichero_entrada == NULL)
	{
		fprintf(stderr, "Must specify one file for argv[2]\n");
		exit(EXIT_FAILURE);
	}
	int i, j;
	i = open(fichero_entrada, O_RDONLY);
	j = open(fichero_salida, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	copy(i, j);
	return 0;
}
