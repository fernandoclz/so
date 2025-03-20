#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <limits.h>


/* Forward declaration */
int get_size_dir(char *fname, size_t *blocks);

/* Gets in the blocks buffer the size of file fname using lstat. If fname is a
 * directory get_size_dir is called to add the size of its contents.
 */
int get_size(char *fname, size_t *blocks)
{
	struct stat buf;
	if(lstat(fname, &buf) == -1){
		printf("Error: get_size\n");
		exit(-1);
	}
	*blocks += (size_t)(buf.st_size+511)/512;
	return 0;
}


/* Gets the total number of blocks occupied by all the files in a directory. If
 * a contained file is a directory a recursive call to get_size_dir is
 * performed. Entries . and .. are conveniently ignored.
 */
int get_size_dir(char *dname, size_t *blocks)
{
	DIR * dir;
	struct dirent *direntp;
	int ret = 0;
	if((dir = opendir(dname))==NULL){
		printf("No se puede abrir el directorio\n");
		return -1;
	}

	while((direntp = readdir(dir)) != NULL){
		if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0) {
            continue;  // Ignorar . y ..
        }
		char path[PATH_MAX];
		snprintf(path, sizeof(path), "%s/%s", dname, direntp->d_name);

		struct stat buff;
		if((lstat(path, &buff)) == -1){
			printf("Error: get_size_dir\n");
			closedir(dir);
			return -1;
		}
		else{
			if(S_ISDIR(buff.st_mode)){
				if (get_size_dir(path, blocks) == -1) {
                closedir(dir);
                return -1;
            }

			}
			else{
				if (get_size(path, blocks) == -1) {
                closedir(dir);
                return -1;
				}
			}
		}
	}

	closedir(dir);
	return ret;
}

/* Processes all the files in the command line calling get_size on them to
 * obtain the number of 512 B blocks they occupy and prints the total size in
 * kilobytes on the standard output
 */
int main(int argc, char *argv[])
{
	if(argc < 2){
		printf("Could not process %s", argv[0]);
		exit(-1);
	}

	for(int i = 1; i < argc; i++) {

	struct stat buf;
	char* fichero_entrada;
	fichero_entrada = argv[i];

	if((lstat(fichero_entrada, &buf)) == -1){
		printf("No se puedo hacer lstat en %d\n", i);
		continue;
	}

	size_t bloques = 0;

	if(S_ISDIR(buf.st_mode)){
		if(get_size_dir(fichero_entrada, &bloques) == -1){
			return EXIT_FAILURE;
		}
	}
	else{
		if (get_size(fichero_entrada, &bloques) == -1) {
            return EXIT_FAILURE;
        }
	}

	size_t kilobytes = (bloques+1)/2;

	printf("%zuK\t%s\n", kilobytes, fichero_entrada);
	}
	return 0;
}
