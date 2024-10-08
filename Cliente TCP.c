#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool jugando = false;
bool miTurno = false;

int main(int argc, char const *argv[])
{
    // Comprobar que se ha llamado correctamente
    if (argc < 2)
    {
        printf("-ERR. Faltan argumentos\n");
        printf("Ejecuta el programa de la siguiente manera:\n");
        printf("./clienteTCP <Server's Ip>\n");
        return -1;
    }

    // Descriptores de socket y buffer de datos
    int sd;
    struct sockaddr_in sockname;
    char buffer[250];
    socklen_t len_sockname;
    fd_set readfds, auxfds;
    int salida;
    int fin = 0;

    // Se abre el socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1)
    {
        perror("–ERR. No se puede abrir el socket cliente\n");
        exit(1);
    }

    // Rellenar los campos de la estructura con la IP del servidor
    sockname.sin_family = AF_INET;
    sockname.sin_port = htons(2050); // Puerto del servidor
    sockname.sin_addr.s_addr = inet_addr(argv[1]);

    // Solicitar la conexión con el servidor
    len_sockname = sizeof(sockname);
    if (connect(sd, (struct sockaddr *)&sockname, len_sockname) == -1)
    {
        perror("–ERR. No se puede conectar al servidor");
        exit(1);
    }

    // Inicializar las estructuras de los sets
    FD_ZERO(&auxfds);
    FD_ZERO(&readfds);

    FD_SET(0, &readfds);  // Para la entrada por teclado
    FD_SET(sd, &readfds); // Para la comunicación con el servidor

    printf("Bienvenido al Blackjack online.\n");
    printf("Inicie sesión para jugar:\n");
    printf(" - USUARIO suNombre\n");

    // Bucle principal
    do
    {
        auxfds = readfds;
        salida = select(sd + 1, &auxfds, NULL, NULL, NULL);

        // Mensaje desde el servidor
        if (FD_ISSET(sd, &auxfds))
        {
            bzero(buffer, sizeof(buffer));
            recv(sd, buffer, sizeof(buffer), 0);

            printf("%s", buffer);

            // Control de finalización del programa
            if (strcmp(buffer, "–ERR. Demasiados clientes conectados\n") == 0)
                fin = 1;

            if (strcmp(buffer, "–ERR. Desconectado por el servidor\n") == 0)
                fin = 1;

            if (strcmp(buffer, "+Ok. Usuario correcto\n") == 0)
            {
                printf("Introduzca ahora su contraseña así:\n");
                printf("   PASSWORD suContraseña\n");
            }

            if (strcmp(buffer, "+Ok. Usuario validado\n") == 0)
            {
                printf("Para iniciar una partida, introduzca:\n");
                printf("   INICIAR-PARTIDA\n");
            }

            if (strncmp(buffer, "+Ok. Empieza la partida.", strlen("+Ok. Empieza la partida.")) == 0)
            {
                jugando = true;
                printf("Opciones durante su turno:\n");
                printf(" - PEDIR-CARTA\n");
                printf(" - PLANTARME\n");
                printf(" - PUNTUACION\n");
                printf(" - SALIR\n");
            }

            // Control del turno
            if (strcmp(buffer, "+Ok. Turno de partida\n") == 0)
            {
                miTurno = true;
                printf("Es tu turno, ¿qué deseas hacer?\n");
            }

            if (strcmp(buffer, "+Ok. Turno del otro jugador\n") == 0)
            {
                miTurno = false;
                printf("Esperando el turno del otro jugador...\n");
            }

            if (strcmp(buffer, "+Ok. Partida finalizada\n") == 0)
            {
                jugando = false;
                miTurno = false;
                printf("La partida ha finalizado.\n");
            }

            if (strcmp(buffer, "+Ok. Desconexión procesada.\n") == 0)
                fin = 1;
        }

        // Entrada del usuario por teclado
        if (FD_ISSET(0, &auxfds))
        {
            bzero(buffer, sizeof(buffer));
            fgets(buffer, sizeof(buffer), stdin);

            // Validar el estado del juego
            if (strncmp(buffer, "PASSWORD", strlen("PASSWORD")) == 0 && !jugando)
            {
                printf("-Err. Debes iniciar sesión antes de realizar otras acciones\n");
            }
            else if (strncmp(buffer, "PEDIR-CARTA", strlen("PEDIR-CARTA")) == 0 && (!jugando || !miTurno))
            {
                printf("-Err. Debes estar en una partida y ser tu turno para pedir carta\n");
            }
            else if (strncmp(buffer, "PLANTARME", strlen("PLANTARME")) == 0 && (!jugando || !miTurno))
            {
                printf("-Err. Debes estar en una partida y ser tu turno para plantarte\n");
            }
            else if (strncmp(buffer, "PUNTUACION", strlen("PUNTUACION")) == 0 && !jugando)
            {
                printf("-Err. Debes estar en una partida para ver tu puntuación\n");
            }
            else if (strncmp(buffer, "SALIR", strlen("SALIR")) == 0 && !jugando)
            {
                printf("-Err. No estás en una partida\n");
            }
            else
            {
                // Enviar comando al servidor
                send(sd, buffer, sizeof(buffer), 0);
            }
        }
    } while (fin == 0);

    close(sd); // Cerrar el socket

    return 0;
}
