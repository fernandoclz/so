#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

void readFile(int file){
	char array[512];
	int leido;
	while((leido = read(file, array, 512)) > 0){
		printf("%s",array);
	}
	close(file);
}

int main(int argc, char *argv[])
{
	char *fichero_entrada;
	int opt, bytes = 0;
	int isE = -1;
	if(argc < 2){
		printf("Faltan argumentos para %s\n", argv[0]);
	}

	while((opt = getopt(argc, argv, "n:e")) != -1){
		switch("%s",opt){
			case 'n':
				bytes = atoi(optarg);
			break;
			case 'e':
				isE = 1;
			break;
			default:
			break;
		}
	}

	fichero_entrada = argv[optind];

	int file;
	if((file = open(fichero_entrada, O_RDONLY)) == -1){
		printf("No se pudo abrir el fichero %s\n", argv[optind]);
	}
	else{

		if(isE == -1 && bytes != 0){
			lseek(file, bytes, SEEK_SET);
		}
		else if(isE == 1 && bytes != 0){
			lseek(file, -bytes, SEEK_END);
		}

		readFile(file);
	}
	return 0;
}
