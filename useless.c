#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define NBYTES 16
#define MAXDIGITS 10

int search_for_min(int *arr, int n, int min){
    int result, i;
    int maxtime = 1000;
    for(i = 0; i < n; i++){
        if ((arr[i] > min) && (arr[i] < maxtime)){
            result = i;
            maxtime = arr[i];
        }
    }
    return result;
}
int main(int argc, char** argv, char* envp[]){
    char *path;
    path = getenv("SHELL");
    time_t launch_time = time(NULL);
    char *text = NULL;
    int fd;
    (void)umask(0);
    if ((fd = open("./command.txt", O_RDONLY, 0666)) < 0){
        printf("Cannot open a file\n");
        exit(EXIT_FAILURE);
    }
    size_t size;
    char buffer[NBYTES];
    int total = 1;
    while (size = read(fd, buffer, NBYTES)){
        if (size < 0){
            printf("Cannot read the file\n");
            exit(EXIT_FAILURE);
        }
        total = total + size;
        text = (char*)realloc(text, total*sizeof(char)); //check the size
        strncat(text, buffer, size);
    }
    if (close(fd) < 0){
        printf("Cannot close FIFO\n");
        exit(EXIT_FAILURE);
    }
    int length = strlen(text);
    int i, j, tmp = 0, count = 0, flag = 0;
    int strings = 0;
    int *delay = NULL;
    char **command;
    char arr_tmp[MAXDIGITS];
    for (i = 0; i < length; i++){
        if ((text[i] == '\n') || (text[i] == EOF)){
            strings++;
        }
    }
    command = (char**)malloc(strings*sizeof(char*));
    for (i = 0; i < length; i++){
         if ((text[i] == ' ') && (flag == 0)){
             for (j = tmp; j < i; j++)
                 arr_tmp[j - tmp] = text[j];
             arr_tmp[j - tmp] = 0;
             tmp = i;
             char *end;
             delay = (int*)realloc(delay, (count + 1)*sizeof(int));
             if ((delay[count] = strtol(arr_tmp, &end, 10)) == 0){
                 if (errno == EINVAL){
                     printf("Invalid number %d: check the input\n", count + 1);
                     exit(EXIT_FAILURE);
                 }
                 if (errno == ERANGE){
                     printf("The number %d is out of range\n", count + 1);
                     exit(EXIT_FAILURE);
                 }
             }
             flag++;
         }
         if (text[i] == '\n'){
             command[count] = (char*)malloc((i - tmp)*sizeof(char));
             for (j = tmp; j < i; j++)
                 command[count][j - tmp] = text[j];
             command[count][j - tmp] = 0;
             tmp = i;
             flag = 0;
             count++;
         }
    }
    j = 0;
    for (i = 0; i < count; i++){
        if (i == 0)
            j = search_for_min(delay, count, 0);
        else j = search_for_min(delay, count, delay[j]);
        int result = fork();
        if (result < 0){
            printf("Cannot fork\n");
            exit(-1);
        }
        else if (result == 0){
            time_t delta = time(NULL) - launch_time;
            if (sleep(delay[j] - delta) > 0){
                printf("Cannot sleep\n");
                exit(-1);
            }
            if (execl(path, "bash", "-c", command[j], NULL) < 0){
                printf("Cannot execute command %d\n", j);
                exit(-1);
            }
        }
    }
    free(text);
    free(delay);
    free(command);
    return 0;
}
