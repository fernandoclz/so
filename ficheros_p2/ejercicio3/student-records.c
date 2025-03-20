#include <stdio.h>
#include <unistd.h> /* for getopt() */
#include <stdlib.h> /* for EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h>
#include "defs.h"


/* Assume lines in the text file are no larger than 100 chars */
#define MAXLEN_LINE_FILE 100

student_t *leerFichero(char *path, int *num){
	FILE* fichero;
	student_t *estudiantes; 
	char line[MAX_PASSWD_LINE+1];
	char* lineptr;
	char* token;
	token_id_t token_id;
	if((fichero=fopen(path, "r"))==NULL){
		printf("The input file %s could not be opened",path);
		perror(NULL);
		return NULL;
	}
	int valor;
	if (fscanf(fichero, "%d", &valor) != 1) {
        printf("Error al leer el número de estudiantes.\n");
        fclose(fichero);
        return NULL;
    }

	*num = valor;
	estudiantes=malloc(sizeof(student_t)*valor);
	memset(estudiantes,0,sizeof(student_t)*valor);
	fgets(line, MAX_PASSWD_LINE+1, fichero);
	int i = 0;
	while(fgets(line, MAX_PASSWD_LINE + 1, fichero) != NULL){
		lineptr = line;
		token_id = STUDENT_ID_IDX;
		student_t *cur_student=&estudiantes[i];
		while((token = strsep(&lineptr, ":"))!=NULL){
			switch(token_id){
				case STUDENT_ID_IDX:
					sscanf(token,"%d",&cur_student->student_id);
					break;
				case NIF_IDX:
					strcpy(cur_student->NIF, token);
					break;
				case FIRST_NAME_IDX:
					cur_student->first_name = strdup(token);
					break;
				case LAST_NAME_IDX:
					token[strlen(token)-1]='\0';
					cur_student->last_name = strdup(token);
					break;
				default:
					break;
			}
			token_id++;
		}
		if(token_id!=NR_FIELDS_STUDENT){
			printf("Could not process all tokens from line %d \n", i+1);
			continue;
		}
		i++;
	}
	fclose(fichero);
	return estudiantes;
}

static void free_entries(student_t* entries, int nr_entries)
{
	int i=0;
	student_t* entry;

	for (i=0; i<nr_entries; i++) {
		entry=&entries[i]; /* Point to current entry */
		free(entry->first_name);
		free(entry->last_name);
	}

	free(entries);
}

int print_text_file(char *path)
{
	/* To be completed (part A) */
	int num = 0;
	student_t* estudiantes = leerFichero(path, &num);
	if(estudiantes == NULL){
		printf("Error en la lectura");
		return -1;
	}
	for(int i = 0; i < num; i++){
		printf("[Entry #%d]\n", i);
		printf("\tstudent_id=%d\n", estudiantes[i].student_id);
		printf("\tNIF=%s\n", estudiantes[i].NIF);
		printf("\tfirst_name=%s\n", estudiantes[i].first_name);
		printf("\tlast_name=%s\n", estudiantes[i].last_name);
	}
	free_entries(estudiantes, num);
	return 0;
}

int print_binary_file(char *path)
{
	/* To be completed  (part B) */
	FILE *file = NULL;
	if ((file = fopen(path, "r")) == NULL){
		printf("The input file %s could not be opened",path);
		return -1;
	}

	int num;
	student_t *estudiantes;
	fseek(file, 0, SEEK_SET);

	fread(&num, sizeof(int), 1, file);

	estudiantes=malloc(sizeof(student_t)*num);
	memset(estudiantes,0,sizeof(student_t)*num);

	int it = 0;
	while(it < num){
		fread(&estudiantes[it].student_id, sizeof(int), 1, file);
		fread(&estudiantes[it].NIF, sizeof(char), MAX_CHARS_NIF+1 , file);
		int cont = 0;
		char temp;
		while(fread(&temp, sizeof(char), 1, file) == 1 && temp != '\0'){
			cont++;
		}
		cont++;
		fseek(file, -cont, SEEK_CUR);
		estudiantes[it].first_name = malloc(sizeof(char)*cont);
		int leidof = fread(estudiantes[it].first_name, sizeof(char), cont, file);
		if(leidof < cont){
			printf("Mal leídof\n");
			return -1;
		}
		cont = 0;
		while(fread(&temp, sizeof(char), 1, file) == 1 && temp != '\0'){
			cont++;
		}
		if(cont == 0){
			return -1;
		}
		cont++;
		fseek(file, -cont, SEEK_CUR);
		estudiantes[it].last_name = malloc(sizeof(char)*cont);
		int leido = fread(estudiantes[it].last_name, sizeof(char), cont, file);
		if(leido < cont){
			printf("Mal leído\n");
			return -1;
		}

		printf("[Entry #%d]\n", it);
		printf("\tstudent_id=%d\n", estudiantes[it].student_id);
		printf("\tNIF=%s\n", estudiantes[it].NIF);
		printf("\tfirst_name=%s\n", estudiantes[it].first_name);
		printf("\tlast_name=%s\n", estudiantes[it].last_name);

		it++;
	}
	
	return 0;
}


int write_binary_file(char *input_file, char *output_file)
{
	int num = 0;
	FILE *file = NULL;
	student_t *estudiantes = leerFichero(input_file, &num);

	if ((file = fopen(output_file, "w")) == NULL)
		printf("The input file %s could not be opened",output_file);

	fwrite(&num, sizeof(int), 1, file);
	int i = 0;
	while (i < num) {
		fwrite(&estudiantes[i].student_id, sizeof(int), 1, file);
		fwrite(estudiantes[i].NIF, sizeof(char), strlen(estudiantes[i].NIF)+1, file);
		int lengthf = strlen(estudiantes[i].first_name)+1;
		fwrite(estudiantes[i].first_name, sizeof(char), lengthf, file);
		int lengthl = strlen(estudiantes[i].last_name)+1;
		fwrite(estudiantes[i].last_name, sizeof(char), lengthl, file);
		i++;
	}
	free_entries(estudiantes, num);
	fclose(file);
	return 0;
}

int main(int argc, char *argv[])
{
	int ret_code, opt;
	struct options options;

	/* Initialize default values for options */
	options.input_file = NULL;
	options.output_file = NULL;
	options.action = NONE_ACT;
	ret_code = 0;

	/* Parse command-line options (incomplete code!) */
	while ((opt = getopt(argc, argv, "hpi:o:b")) != -1)
	{
		switch (opt)
		{
		case 'h':
			fprintf(stderr, "Usage: %s [ -h | -p | -i file | -o <output_file> | -b ]\n", argv[0]);
			exit(EXIT_SUCCESS);
		case 'i':
			options.input_file = optarg;
			break;
		case 'p':
			options.action = PRINT_TEXT_ACT;
			break;
		case 'o':
			options.output_file = optarg;
			options.action = WRITE_BINARY_ACT;
			break;
		case 'b':
			options.action = PRINT_BINARY_ACT;
			break;
		/**
		 **  To be completed ...
		 **/

		default:
			exit(EXIT_FAILURE);
		}
	}

	if (options.input_file == NULL)
	{
		fprintf(stderr, "Must specify one record file as an argument of -i\n");
		exit(EXIT_FAILURE);
	}

	switch (options.action)
	{
	case NONE_ACT:
		fprintf(stderr, "Must indicate one of the following options: -p, -o, -b \n");
		ret_code = EXIT_FAILURE;
		break;
	case PRINT_TEXT_ACT:
		/* Part A */
		ret_code = print_text_file(options.input_file);
		break;
	case WRITE_BINARY_ACT:
		/* Part B */
		ret_code = write_binary_file(options.input_file, options.output_file);
		break;
	case PRINT_BINARY_ACT:
		/* Part C */
		ret_code = print_binary_file(options.input_file);
		break;
	default:
		break;
	}
	exit(ret_code);
}
