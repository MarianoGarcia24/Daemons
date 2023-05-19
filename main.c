#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <dirent.h>

void signalHandler(int sig) {
    // Manejar señales aquí
}

int main() {
    // Crear un nuevo proceso
    pid_t pid = fork();

    // Salir si el fork falla
    if (pid < 0) {
        printf("Error al crear el proceso hijo.\n");
        exit(1);
    }

    // Salir del proceso padre
    if (pid > 0) {
        exit(0);
    }

    // Establecer el modo de archivo y directorio
    umask(0);

    // Crear una nueva sesión de grupo
    pid_t sid = setsid();
    if (sid < 0) {
        printf("Error al crear la nueva sesión de grupo.\n");
        exit(1);
    }

    // Cerrar los descriptores de archivo estándar
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Establecer el manejador de señales
    signal(SIGTERM, signalHandler);  // Por ejemplo, manejar SIGTERM

    // Bucle principal del daemon
    while (1) {
        

        sleep(1);  // Ejemplo: Esperar 1 segundo antes de repetir el bucle
    }

    return 0;
}

void makeBackup(){
    DIR *dir;
    struct dirent *entry;

    dir = opendir("./test");



    return 0;
}
