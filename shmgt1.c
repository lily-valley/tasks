#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#define P_ID 3
#define SH_FILE "shmgt.txt"
    int main(int argc,char** argv,char* envp[]){
    int result;
    char name[]="shmgt.fifo";
    (void)umask(0);
    if (mknod (name,S_IFIFO | 0666,0)<0){
        printf("Can\'t create FIFO\n");
        exit(-1);
    }
    result = fork();
    if (result > 0){
        int out = dup(1);     //создаем копию файлового дескриптора
        if (out < 0){
            printf("Can't duplicate stdout\n");
        }
        int e_cl = close(1);
        if (e_cl < 0){
            printf("Can't close stdin\n");
            exit(-1);
        }
        int e_op = open(name,O_WRONLY);
        if (e_op != 1){
            dup2(out,1);
            printf("Can't open FIFO\n");
            exit(-1);
        }
        int e_ex = execlp("/bin/ps","/bin/ps",argv[1],NULL);
        if (e_ex < 0){
            dup2(out,1);
            printf("Error with exec\n");
            exit(-1);
        }
    }
    if (result == 0){
        int fd = open(name,O_RDONLY);
        if (fd < 0){
            printf("Can't open FIFO\n");
            exit(-1);
        }
        const int tot = 256;
        char  *rcvd, *rcurr,*rbuf;
        int cha = sizeof(char);
        rcvd = rcurr = malloc(tot+cha);
        int full = tot;
        int i = read(fd,rcvd,full);
        while (i > 0){
            rcurr += i;
            full -= i;
            if (full == 0){
                rbuf = realloc(rcvd,rcurr-rcvd+tot+1);
                if (rbuf != NULL) rcvd=rbuf;
                else{
                    printf("Error with memory extending\n");
                    printf("Some information was wasted\n");
                    i=-1;
                }
                full = tot;
            }
            if (i > 0) i=read(fd,rcurr,full);
        }
        *rcurr = '\0';
        int length = rcurr-rcvd+cha;
        printf("Child has received this from parent: size=%d bytes\n",length);
        printf("%s\n",rcvd);
        int key = ftok(SH_FILE,P_ID);
        if (key < 0){
            printf("Can't generate a key for shared memory\n");
            exit(-1);
        }
        (void)umask(0);
        int mem = shmget(key,length,0666|IPC_CREAT);
        if (mem < 0){
            printf("errno=%d\n",errno);
            printf("Can't create shared memory segment\n");
            exit(-1);
        }
        char *att = shmat(mem,NULL,0);
        if (att < 0){
            printf("Can't attach shared memory\n");
            exit(-1);
        }
        strcpy(att,rcvd);
        int detach = shmdt(att);
        if (detach < 0){
            printf("Can't detach shared memory\n");
            exit(-1);
        }
        int e_ex=execlp("/bin/rm","/bin/rm",name,NULL);
        if (e_ex < 0){
            printf("Can't delete FIFO file\n");
            exit(-1);
        }
    }
    if (result < 0){
        printf("Can't create a child\n");
        exit(-1);
    }
return(0);
}
