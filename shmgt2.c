#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>

#define P_ID 3
#define SH_FILE "shmgt.txt"
int main(){
    int key=ftok(SH_FILE,P_ID);                             //получаем ключ к уже созданной разделяемой памяти
    if (key<0){
        printf("Can't generate a key for shared memory\n");
        exit(-1);
    }
    (void)umask(0);
    int mem = shmget(key,1,0666);                          //получаем доступ только к этому сегменту памяти, но только на первый элемент
    if (mem<0){
        printf("Can't get access to shared memory\n");    //(это не важно, т.к. shmget возвращает указатель, а нам теперь не надо знать длину данных,
        exit(-1);
    }                                                    //полученных в первой программе
    char *att = shmat(mem,NULL,0);                       //включаем эту область в наше адресное пространство
    if (att < 0){
        printf("Can't attach shared memory\n");
        exit(-1);
    }
    printf("%s\n",att);                                  //печатаем из разделяемой памяти по нашему указателю att
    int e_dt = shmdt(att);                               //удаляем разделяемую память из адресного пространства
    if (e_dt < 0){
        printf("Can't detach shared memory\n");
        exit(-1);
    }
    int mem_rem = shmctl(mem,IPC_RMID,NULL);             //удаляем разделяемую память из системы
    if (mem_rem == 0) printf("Shared memory has been deleted successfully\n");
    else{
        printf("Can't delete shared memory\n");
        printf("errno=%d\n",errno);
        exit(-1);
    }
    return 0;
}
