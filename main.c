#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#define ROOT "./test"
#include <time.h>
#include <dirent.h>

void signalHandler(int sig) {
    // Manejar señales aquí
}

void copyFile(const char* sourcePath, const char* destPath) {
    FILE* sourceFile = fopen(sourcePath, "rb");
    FILE* destFile = fopen(destPath, "wb");

    if (sourceFile == NULL || destFile == NULL) {
        return;
    }

    int ch;
    while ((ch = fgetc(sourceFile)) != EOF) {
        fputc(ch, destFile);
    }

    fclose(sourceFile);
    fclose(destFile);
}

void copyDirectory(const char* sourceDir, const char* destDir) {
    DIR* dir;
    struct dirent* entry;

    // Crear el directorio de destino si no existe
    mkdir(destDir);

    // Abrir el directorio de origen
    dir = opendir(sourceDir);
    if (dir == NULL) {
        return;
    }

    // Recorrer cada entrada del directorio
    while ((entry = readdir(dir)) != NULL) {
        // Ignorar las entradas "." y ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construir las rutas completas de origen y destino
        char sourcePath[256];
        char destPath[256];
        snprintf(sourcePath, sizeof(sourcePath), "%s/%s", sourceDir, entry->d_name);
        snprintf(destPath, sizeof(destPath), "%s/%s", destDir, entry->d_name);

        struct stat st;
        if (stat(sourcePath, &st) == 0 && S_ISDIR(st.st_mode)) {
            // Copiar el directorio de manera recursiva
            copyDirectory(sourcePath, destPath);
        } else {
            // Copiar el archivo
            copyFile(sourcePath, destPath);
        }
    }

    closedir(dir);
}


int runDeamon(){

    struct stat fileStat;
    
    if (stat(ROOT,&fileStat) == -1){
        return 1;
    }

    time_t modifiedTime = fileStat.st_mtime;
    
    char *backUpRoute = "./.test_backup";

    DIR *dir;

    dir = opendir(backUpRoute);
    if (dir == NULL){
        mkdir(backUpRoute);
        copyDirectory(ROOT,backUpRoute);
    }
    else{
        struct stat fileStatBackup;

        if (stat(backUpRoute,&fileStatBackup) == -1){
            return 1;
        }

        time_t modifiedTimeBackup = fileStatBackup.st_mtime;

        if (modifiedTime > modifiedTimeBackup){
           copyDirectory(ROOT,backUpRoute);
        }
        else{
            DIR * rootDir;
            struct dirent* entry;
            rootDir = opendir(ROOT);
            entry = readdir(rootDir);
            time_t modifiedTimeEntry;

            do{
                char path[256];
                snprintf(path, sizeof(rootDir), "%s/%s", rootDir, entry->d_name);
                struct stat st;

                stat(path,&st);
                modifiedTimeEntry = st.st_mtime;

            } while((entry = readdir(rootDir)) != NULL && modifiedTimeEntry <= modifiedTimeBackup);
            
            if (entry != NULL)
                copyDirectory(ROOT,backUpRoute);
                       
        }
    }

    return 0;

}


int main() {
    // // Crear un nuevo proceso
    // pid_t pid = fork();

    // // Salir si el fork falla
    // if (pid < 0) {
    //     printf("Error al crear el proceso hijo.\n");
    //     exit(1);
    // }

    // // Salir del proceso padre
    // if (pid > 0) {
    //     exit(0);
    // }

    // // Establecer el modo de archivo y directorio
    // umask(0);

    // // Crear una nueva sesión de grupo
    // pid_t sid = setsid();
    // if (sid < 0) {
    //     printf("Error al crear la nueva sesión de grupo.\n");
    //     exit(1);
    // }

    // // Cerrar los descriptores de archivo estándar
    // close(STDIN_FILENO);
    // close(STDOUT_FILENO);
    // close(STDERR_FILENO);

    // // Establecer el manejador de señales
    // signal(SIGTERM, signalHandler);  // Por ejemplo, manejar SIGTERM

    // // Bucle principal del daemon
    while (1) {
        runDeamon();

        sleep(5);  // Ejemplo: Esperar 1 segundo antes de repetir el bucle
    }

    return 0;
}
