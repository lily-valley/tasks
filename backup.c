#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
using namespace std;
int cp(const char *patha,const char* pathb, struct stat statA, struct stat statB){
    int a, b;
    a=open(patha, O_RDONLY);
    b=open(pathb, O_WRONLY | O_CREAT);
    char buf;
    int i=0, j;
    int protect = statA.st_mode;
    i=read(a, (void*)&buf, sizeof(char));
    while (i!=0){
        j=write(b, (void*)&buf, sizeof(char));
        if (j==0){perror(pathb);}
        i=read(a, (void*)&buf, sizeof(char));
    }
    close(a);
    close(b);
    chmod(pathb, protect);
    if (S_ISREG(statB.st_mode)){
        int w=fork();
        if (w==0){execl("/bin/gzip", "gzip", pathb, NULL);}
    }
    return 0;
}
int mandarin (const char * patha, const char * pathb ){
    DIR *a, *b; // первая и вторая дир
    a = opendir (patha);
    b = opendir (pathb);
    int i=0; // индикатор для меня есть ли файл в а
    int child;
    struct dirent *entrya, *entryb;
    string path; // для удаления
    entryb = readdir(b);
    while (entryb != NULL) {
        string nameB(entryb->d_name);
        string h;
        char chek[3];
        int size=nameB.size();
        if (size>3){
            nameB.copy(chek, 3, size-3);
            if (strcmp(chek, ".gz")==0){
                h=nameB.substr(0, size-3);
                nameB = h;
            }
        }
        if(strcmp(nameB.c_str(), ".")!=0 && strcmp(nameB.c_str(), "..")!=0 ){
            entrya = readdir(a);
            i=0;
            while (entrya!=NULL){ // просмотр дир а на наличие такого файла
                string zzzz(entrya->d_name);
                if (strcmp(zzzz.c_str(),nameB.c_str())==0)
                i=1;
                entrya = readdir(a);
            }
            if (i==0){
                path = pathb;
                path = path + "/" + entryb->d_name;
                int q=unlink( path.c_str());
            }
            closedir(a);
            a = opendir(patha);
            i=0;
        }
        entryb=readdir(b);
    }
    closedir(a);
    return 0;
}

int svetina(const char * path, const char * path2 ){
    DIR *a, *b,*c; //указывает на директории, которые открыли
    a = opendir (path);
    b = opendir (path2); // b для директории
    struct dirent *entry; //указатель на то, что возвращает readdir
    struct stat f, p; // структура, где будет информация о файле f в первой дир, p во второй дир
    string pf=path; //путь объекта
    string ff=path2; // путь до файла во второй папке
    string r, d; // для дир (1 и 2)
    int i; // для проверки открылся или нет файл
    int status; // для waitpid (третий форк)
    entry = readdir(a);
    while (entry!=NULL){
        if (strcmp(entry->d_name, ".")!=0 && (strcmp(entry->d_name, "..")!=0)){ //проверка, что это не точка и не точка точка //FIXME: аналогично
            pf = path;
            pf = pf +"/"+ entry->d_name; // для первой дир
            stat(pf.c_str(), &f); // в f теперь много полезной информации
            if (S_ISREG(f.st_mode)){ // првоерка, не обычный ли это файл и его вывод, вывод времени изменения
                ff = path2;
                ff = ff + "/" + entry->d_name + ".gz"; // для второй дир. адрес файла, который хотим открыть
                i=open(ff.c_str(), O_RDONLY);// теперь можем уже что-нибудь понять
                if (i!=-1){
                    stat(ff.c_str(), &p); // если файл нормально открылся
                    if (f.st_mtime >= p.st_mtime){ // файл открылся, проверка на время
                        int q = unlink(ff.c_str());
                        ff=path2;
                        ff=ff+"/"+entry->d_name;
                        cp(pf.c_str(), ff.c_str(), f, p);
                    }
                }
                else { // если не открылся - создаём
                    ff=path2;
                    ff=ff+"/"+entry->d_name;
                    cp (pf.c_str(), ff.c_str(), f, p);
                }
                close (i);
            }
            if (S_ISDIR(f.st_mode)){ // проверка, не директрория ли это
                d=ff+"/"+entry->d_name;
                c=opendir (d.c_str()); // попытка открыть дир. с соответсвующим название во второй дир
                if (c!=NULL){
                    svetina(pf.c_str(),d.c_str());
                }
                else{
                    int child3 = fork();
                    if(child3 ==0){
                        mkdir (d.c_str(), f.st_mode);
                    }
                    if(child3 != 0){
                        waitpid(child3, &status, 0);
                        if(WIFEXITED(status));
                        svetina(pf.c_str(),d.c_str());
                    }
                }
            }
        }
        entry = readdir(a);
        pf = path;
    }
    mandarin(path, path2);
    return 0;
}
int main(int argc, char ** argv){
    svetina(argv[1], argv[2]);
    return 0;
}
