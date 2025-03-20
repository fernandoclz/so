#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(void)
{
    int fd1,fd2,i,pos;
    char c;
    char buffer[6];

    fd1 = open("output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    for (i=1; i < 10; i++) {
        
        if (fork() == 0) {
            /* Child */
            fd2 = open("output.txt", O_WRONLY);
            pos = 5 + (i*10);
            lseek(fd2, pos, SEEK_SET);
            sprintf(buffer, "%d", i*11111);
            write(fd2, buffer, 5);
            close(fd2);
            exit(0);
        } else {
            /* Parent */
            fd2 = open("output.txt", O_WRONLY);
            pos = i*10;
            lseek(fd2, pos, SEEK_SET);
            write(fd2, "00000", 5);
            close(fd2);
        }
    }

	//wait for all childs to finish
    while (wait(NULL) != -1);

    lseek(fd1, 0, SEEK_SET);
    printf("File contents are:\n");
    while (read(fd1, &c, 1) > 0)
        printf("%c", (char) c);
    printf("\n");
    close(fd1);
    exit(0);
}
