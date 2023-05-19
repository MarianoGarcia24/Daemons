#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>

int main()
{
    pid_t pid, sid;
    
    // Crear un proceso hijo
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    
    // Si el proceso padre puede crear el proceso hijo, entonces termina
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Inicializar el registro del sistema
    openlog("simple_daemon", LOG_PID, LOG_DAEMON);

    // Bucle principal del daemon
    while (1) {
        // Realizar las tareas del daemon aquí
        syslog(LOG_NOTICE, "El daemon está ejecutándose.");
        sleep(10); // Esperar 10 segundos antes de la siguiente iteración
    }

    // Cerrar el registro del sistema
    closelog();

    exit(EXIT_SUCCESS);
}