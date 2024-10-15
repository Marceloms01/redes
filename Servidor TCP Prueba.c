#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdlib.h>

#define PORT 2060
#define MAX_CLIENTS 30
#define MAX_BUFFER 250

// Estructura para representar a los jugadores
bool find_user(char *username){
    FILE *f;
    char uname[50], pass[50];
    f = fopen("usuarios.txt", "r");
    if(f != NULL){
        while(feof(f) == 0){
            fscanf(f, "%s %s\n", uname, pass);
            printf("%s %s\n", uname, pass);
        }
    }
    fclose(f);
    return false;
}

bool add_user(char *uname, char *pass){
    FILE *f;
    f = fopen("usuarios.txt", "a");
    if(f != NULL){ 
        fprintf(f, "%s %s\n", uname, pass);   
    }
    fclose(f);
    return true;
}

typedef struct {
    int socket;
    char username[50];
    char password[50];
    int puntuacion;
    int cartas[10];
    int num_cartas;
    bool en_partida;
    bool mi_turno;
} Jugador;

Jugador jugadores[MAX_CLIENTS];
int num_jugadores = 0;
FILE *regisro_usuarios;

// Función para inicializar los jugadores
void inicializar_jugadores() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        jugadores[i].socket = -1;
        jugadores[i].puntuacion = 0;
        jugadores[i].num_cartas = 0;
        jugadores[i].en_partida = false;
        jugadores[i].mi_turno = false;
    }
}

// Función para repartir una carta
int repartir_carta() {
    return rand() % 10 + 1;  // Simulamos cartas del 1 al 10
}

// Función para manejar la lógica del juego
void manejar_comando(int i, char* buffer) {
    Jugador* jugador = &jugadores[i];

    // Comando: USUARIO

     if(strstr(buffer, "REGISTRO") != NULL){
        send(jugador->socket, "+Ok. Registro correcto\n", strlen("+Ok. Registro correcto\n"), 0);
    } else if (strncmp(buffer, "USUARIO", 7) == 0) {
        char uname[50];
        sscanf(buffer, "USUARIO %s", uname);
        printf("Usuario %s conectado\n", jugador->username);
        send(jugador->socket, "+Ok. Usuario correcto\n", strlen("+Ok. Usuario correcto\n"), 0);
    } else {
        send(jugador->socket, "-Err. Usuario incorrecto\n", strlen("-Err. Usuario incorrecto\n"), 0);
    }

    // Comando: PASSWORD (No se valida, es un ejemplo)
     if (strncmp(buffer, "PASSWORD", 8) == 0) {
        send(jugador->socket, "+Ok. Usuario validado\n", strlen("+Ok. Usuario validado\n"), 0);
    } else {
            send(jugador->socket, "-Err. Usuario no validado\n", strlen("-Err. Usuario no validado\n"), 0);
        }
    


    // Comando: INICIAR-PARTIDA
    if (strcmp(buffer, "INICIAR-PARTIDA\n") == 0) {
        jugador->en_partida = true;
        jugador->puntuacion = 0;
        jugador->num_cartas = 0;
        jugador->mi_turno = true;
        send(jugador->socket, "+Ok. Empieza la partida. Es tu turno\n", strlen("+Ok. Empieza la partida. Es tu turno\n"), 0);
    }

    // Comando: PEDIR-CARTA
    else if (strcmp(buffer, "PEDIR-CARTA\n") == 0 && jugador->mi_turno && jugador->en_partida) {
        int carta = repartir_carta();
        jugador->cartas[jugador->num_cartas++] = carta;
        jugador->puntuacion += carta;

        char mensaje[MAX_BUFFER];
        sprintf(mensaje, "+Ok. Carta recibida: %d. Puntuación: %d\n", carta, jugador->puntuacion);
        send(jugador->socket, mensaje, strlen(mensaje), 0);

        if (jugador->puntuacion > 21) {
            send(jugador->socket, "+Ok. Te has pasado de 21. Has perdido la partida\n", strlen("+Ok. Te has pasado de 21. Has perdido la partida\n"), 0);
            jugador->en_partida = false;
            jugador->mi_turno = false;
        } else {
            jugador->mi_turno = false;
            // Aquí se puede pasar el turno a otro jugador
        }
         } else if (strcmp(buffer, "PEDIR-CARTA\n") == 0) {
        send(jugador->socket, "-Err. No es tu turno o no estás en una partida\n", strlen("-Err. No es tu turno o no estás en una partida\n"), 0);
    }

    // Comando: PLANTARME
    else if (strcmp(buffer, "PLANTARME\n") == 0 && jugador->mi_turno && jugador->en_partida) {
        jugador->mi_turno = false;
        send(jugador->socket, "+Ok. Te has plantado\n", strlen("+Ok. Te has plantado\n"), 0);
        // Aquí se puede pasar el turno a otro jugador
        } else if (strcmp(buffer, "PLANTARME\n") == 0) {
        send(jugador->socket, "-Err. No es tu turno o no estás en una partida\n", strlen("-Err. No es tu turno o no estás en una partida\n"), 0);
    }

    // Comando: PUNTUACION
    else if (strcmp(buffer, "PUNTUACION\n") == 0 && jugador->en_partida) {
        char mensaje[MAX_BUFFER];
        sprintf(mensaje, "+Ok. Tu puntuación actual es: %d\n", jugador->puntuacion);
        send(jugador->socket, mensaje, strlen(mensaje), 0);
    } else if (strcmp(buffer, "PUNTUACION\n") == 0) {
        send(jugador->socket, "-Err. No estás en una partida\n", strlen("-Err. No estás en una partida\n"), 0);
    }

    // Comando: SALIR
    else if (strcmp(buffer, "SALIR\n") == 0) {
        jugador->en_partida = false;
        jugador->mi_turno = false;
        send(jugador->socket, "+Ok. Has salido de la partida\n", strlen("+Ok. Has salido de la partida\n"), 0);
    }
    
    // Comando no reconocido
     else {
        send(jugador->socket, "-Err. Comando no reconocido\n", strlen("-Err. Comando no reconocido\n"), 0);
    }
}

int main() {
    int sd, new_sd, salida;
    struct sockaddr_in sockname, from;
    char buffer[MAX_BUFFER];
    socklen_t from_len;
    fd_set readfds, auxfds;
    int max_sd;

    // Inicializar jugadores
    inicializar_jugadores();

    // Abrir el socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        perror("No se puede abrir el socket");
        exit(1);
    }

    // Configurar el servidor
    sockname.sin_family = AF_INET;
    sockname.sin_port = htons(PORT);
    sockname.sin_addr.s_addr = INADDR_ANY;

    // Enlazar el socket al puerto
    if (bind(sd, (struct sockaddr *)&sockname, sizeof(sockname)) == -1) {
        perror("Error en el bind");
        exit(1);
    }

    // Escuchar conexiones
    listen(sd, MAX_CLIENTS);

    // Inicializar sets
    FD_ZERO(&readfds);
    FD_SET(sd, &readfds);
    max_sd = sd;

    printf("Servidor TCP iniciado. Esperando conexiones...\n");

    while (1) {
        auxfds = readfds;
        salida = select(max_sd + 1, &auxfds, NULL, NULL, NULL);

        // Nuevas conexiones
        if (FD_ISSET(sd, &auxfds)) {
            from_len = sizeof(from);
            new_sd = accept(sd, (struct sockaddr *)&from, &from_len);

            if (new_sd == -1) {
                perror("Error aceptando conexión");
            } else {
                if (num_jugadores < MAX_CLIENTS) {
                    jugadores[num_jugadores].socket = new_sd;
                    FD_SET(new_sd, &readfds);
                    if (new_sd > max_sd) {
                        max_sd = new_sd;
                    }
                    num_jugadores++;
                    printf("Nuevo jugador conectado. Total jugadores: %d\n", num_jugadores);
                    send(new_sd, "+Ok. Conexión establecida. Esperando comandos...\n", strlen("+Ok. Conexión establecida. Esperando comandos...\n"), 0);
                } else {
                    send(new_sd, "–ERR. Demasiados clientes conectados\n", strlen("–ERR. Demasiados clientes conectados\n"), 0);
                    close(new_sd);
                }
            }
        }

        // Gestión de mensajes de los clientes
        for (int i = 0; i < num_jugadores; i++) {
            if (jugadores[i].socket != -1 && FD_ISSET(jugadores[i].socket, &auxfds)) {
                bzero(buffer, sizeof(buffer));
                int bytes = recv(jugadores[i].socket, buffer, sizeof(buffer), 0);

                if (bytes > 0) {
                    manejar_comando(i, buffer);
                } else {
                    printf("Jugador desconectado\n");
                    close(jugadores[i].socket);
                    FD_CLR(jugadores[i].socket, &readfds);
                    num_jugadores--;
                    jugadores[i].socket = -1;
                }
            }
        }
    }

    close(sd);
    return 0;
}
