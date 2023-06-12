#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
// #define sourcePath "./test"
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <syslog.h>

void copyFile(char* sourcePath, char* destPath) {
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

void copyDirectory(char* sourceDir, char* destDir) {
    DIR* dir;
    struct dirent* entry;

    // Crear el directorio de destino si no existe
    mkdir(destDir,0777);

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

int isModified(struct dirent* entry,DIR* sourcePathDir,char* actualPath,time_t modifiedTimeBackup){
    time_t modifiedTimeEntry;
    struct stat st;

    do{
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        char path[256];
        snprintf(path, sizeof(path), "%s/%s", actualPath, entry->d_name);
        if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)){
            DIR* actualDir = opendir(path);
            entry = readdir(actualDir);
            if (entry != NULL)
                if (isModified(entry,actualDir,path,modifiedTimeBackup))
                    return 1;
        }
        modifiedTimeEntry = st.st_mtime;
    } while((entry = readdir(sourcePathDir)) != NULL && modifiedTimeEntry <= modifiedTimeBackup);

    if (modifiedTimeEntry > modifiedTimeBackup)
        return 1;
    else
        return 0;
    
}

int runDeamon(char* sourcePath, char* destinationPath){

    struct stat fileStat;
    
    if (stat(sourcePath,&fileStat) == -1){
        return 0;
    }

    time_t modifiedTime = fileStat.st_mtime;
   

    DIR *dir;

    dir = opendir(destinationPath);
    if (dir == NULL){
        mkdir(destinationPath,0777);
        copyDirectory(sourcePath,destinationPath);
        return 1;
    }
    else{
        //obtenemos timestamp de carpeta Backup
        struct stat fileStatBackup;

        if (stat(destinationPath,&fileStatBackup) == -1){
            return 0;
        }
        time_t modifiedTimeBackup = fileStatBackup.st_mtime;
        struct dirent* entryBckp;
        entryBckp = readdir(dir);
        if (entryBckp != NULL){
            struct stat st;
            char sourcePath[256];
            snprintf(sourcePath, sizeof(sourcePath), "%s/%s", destinationPath, entryBckp->d_name);
            while(stat(sourcePath, &st) == 0 && S_ISDIR(st.st_mode)){
                entryBckp = readdir(dir);
                snprintf(sourcePath, sizeof(sourcePath), "%s/%s", destinationPath, entryBckp->d_name);
            }
            modifiedTimeBackup = st.st_mtime;
        }
        //obtenemos timestamp de carpeta Backup

        if (modifiedTime > modifiedTimeBackup){
           copyDirectory(sourcePath,destinationPath);
           return 1;
        }
        else{
            DIR * sourcePathDir;
            struct dirent* entry;
            struct stat st;
            sourcePathDir = opendir(sourcePath);
            entry = readdir(sourcePathDir);
            time_t modifiedTimeEntry;

            if (isModified(entry,sourcePathDir,sourcePath,modifiedTimeBackup)){
                copyDirectory(sourcePath,destinationPath);
            	return 1;
            }
            else
            	return 0;
                       
        }
    }
}




int main(int argc, char* argv[]) {
    char* sourcePath = argv[1];
    char* destinationPath = argv[2];
    // Crear un nuevo proceso
    pid_t padre = getpid();
    pid_t pid = fork();
    printf("Proceso creado\n");
    // Salir si el fork falla

    if (pid < 0) {
        printf("Error al crear el proceso hijo.\n");
        exit(1);
    }

    //Salir del proceso padre
    if (pid > 0) {
        exit(0);
    }

    // Establecer el modo de archivo y directorio
    openlog("backupd", LOG_PID, LOG_DAEMON);


    // Cerrar los descriptores de archivo estándar
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);


    // Bucle principal del daemon

    while (1) {
        if (runDeamon(sourcePath, destinationPath)){
        	//el Daemon hizo una copia de seguridad.
        	syslog(LOG_NOTICE, "Se ha realizado una nueva copia de seguridad...");
        }
        else{
        }
        sleep(5);  // Ejemplo: Esperar 1 segundo antes de repetir el bucle
    }

    closelog();
	
    exit(EXIT_SUCCESS);
}
