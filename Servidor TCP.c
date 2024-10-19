#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define PORT 2050
#define MAX_CLIENTS 20
#define MAX_BUFFER 250

bool add_users(char *uname, char *pass){
    FILE *f;
    f = fopen("usuarios.txt", "a");
    if(f != NULL){ 
        fprintf(f, "%s %s\n", uname, pass);   
    }
    fclose(f);
    return true;
}

bool check_registered(char *name){
    FILE *f;
    char uname[50], pass[50];
    bool result = false;
    f = fopen("usuarios.txt", "r");
    if(f != NULL){
        while(feof(f) == 0){
            fscanf(f, "%s %s\n", uname, pass);
            if(strcmp(name, uname) == 0){
                result = true;
                break;
            }
        }
    }
    fclose(f);
    return result;
}


char* find_user(char *name) {
    FILE *f;
    char uname[50], pass[50];
    char *result = NULL;
    f = fopen("usuarios.txt", "r");
    if (f != NULL) {
        while (!feof(f)) {
            fscanf(f, "%s %s\n", uname, pass);
            if (strcmp(name, uname) == 0) {
                result = strdup(pass);  // Crear una copia de la contraseña
                break;
            }
        }
        fclose(f);
    }
    return result;
}


// Estructura para representar a los jugadores

typedef struct {
    int socket;
    char username[50];
    char *password;
    int puntuacion;
    int cartas[10];
    int num_cartas;
    bool en_partida;
    bool mi_turno;
    bool plantado;
    bool session;
    bool preparado_para_partida;
    int partida_asociada;

} Jugador;


typedef struct{
    Jugador jugador1;
    Jugador jugador2;
    int id;
    bool is_started;
    
}Partida;

Jugador jugadores[MAX_CLIENTS];
Partida partidas[MAX_CLIENTS/2];

bool check_logged(char *name){
    bool val = false;
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(strcmp(name, jugadores[i].username) == 0 && jugadores[i].session){
            val = true;
        }
    }
    return val;
}

int num_jugadores = 0;
int num_partidas = 0;

// Función para inicializar los jugadores
void inicializar_jugadores() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        jugadores[i].socket = -1;
        jugadores[i].puntuacion = 0;
        jugadores[i].num_cartas = 0;
        jugadores[i].en_partida = false;
        jugadores[i].mi_turno = false;
        jugadores[i].preparado_para_partida = false; 
        jugadores[i].session = false;
        jugadores[i].partida_asociada = 0;
        jugadores[i].plantado = false;

    }
}

Jugador *encontrar_jugador_por_socket(int socket){
    Jugador *val = NULL;
    for(int i = 0; i < MAX_CLIENTS;i++){
        if(jugadores[i].socket == socket){
            val = &jugadores[i];
            break;
        }
    }
    return val;
}

Partida *encontrar_partida_por_id(int id){
    Partida *val = NULL;
    for(int i = 0; i < (MAX_CLIENTS/2); i++){
        if(partidas[i].id == id){
            val = &partidas[i];
        }
    }
    return val;
}

void inicializar_partidas() {
    Jugador jugador;
    jugador.socket = -1;
    jugador.puntuacion = 0;
    jugador.num_cartas = 0;
    jugador.en_partida = false;
    jugador.mi_turno = false;
    jugador.preparado_para_partida = false; 
    jugador.session = false;
    for (int i = 0; i < MAX_CLIENTS/2; i++) {
        partidas[i].jugador1 = jugador;
        partidas[i].jugador2 = jugador;
        partidas[i].is_started = false;
        partidas[i].id = i+1;

    }
}

void iniciar_partidas(){
    for(int i = 0; i<MAX_CLIENTS/2;i++){

    }
}

// Función para repartir una carta
int repartir_carta() {
    return rand() % 10 + 1;  // Simulamos cartas del 1 al 10
}

// Función para manejar la lógica del juego
void manejar_comando(int i, char* buffer) {
    Jugador* jugador = &jugadores[i];

    // Comando: REGISTRO
    if(strncmp(buffer, "REGISTRO", 8) == 0){
        char nickname[50], password[50];
        sscanf(buffer, "REGISTRO -u %s -p %s", nickname, password);
        printf("Usuario %s contraseña %s\n", nickname, password);
        if(!check_registered(nickname)){
            add_users(nickname, password);
            send(jugador->socket, "+Ok. Registro correcto\n", strlen("+Ok. Registro correcto\n"), 0);
        }else{
            send(jugador->socket, "-Err. Registro incorrecto\n", strlen("-Err. Registro incorrecto\n"), 0);
        }
    }

    // Comando: USUARIO
    if (strncmp(buffer, "USUARIO", 7) == 0) {
        char nickname[50], *password;
        sscanf(buffer, "USUARIO %s", nickname);
        password = find_user(nickname);
        if(password != NULL){
            sscanf(buffer, "USUARIO %s", jugador->username);
            jugador->password = password; 
            printf("Usuario %s conectado\n", nickname);
            send(jugador->socket, "+Ok. Usuario correcto\n", strlen("+Ok. Usuario correcto\n"), 0);

        }else{
            send(jugador->socket, "-Err. Usuario incorrecto\n", strlen("-Err. Usuario correcto\n"), 0);
        }
    }

    // Comando: PASSWORD 
    else if (strncmp(buffer, "PASSWORD", 8) == 0) {
        char password[50];
        sscanf(buffer, "PASSWORD %s", password);

        // Compara la contraseña ingresada con la contraseña almacenada
        if (jugador->password != NULL && strcmp(password, jugador->password) == 0) {
            jugador->session = true;
            send(jugador->socket, "+Ok. Usuario validado\n", strlen("+Ok. Usuario validado\n"), 0);
        } else {
            send(jugador->socket, "-Err. Usuario no validado\n", strlen("-Err. Usuario no validado\n"), 0);
        }
    }


    // Comando: INICIAR-PARTIDA
    else if (strcmp(buffer, "INICIAR-PARTIDA\n") == 0 && !jugador->preparado_para_partida && !jugador->en_partida) {
        if (!jugador->session) {
            send(jugador->socket, "-Err. Debes iniciar sesión antes de comenzar una partida\n", strlen("-Err. Debes iniciar sesión antes de comenzar una partida\n"), 0);
            return;
        }
        jugador->preparado_para_partida = true;
        jugador->puntuacion = 0;
        jugador->num_cartas = 0;
        for (int i = 0; i<MAX_CLIENTS/2; i++) {
            if(!partidas[i].is_started){
                if(partidas[i].jugador1.socket == -1){
                    partidas[i].jugador1.socket = jugador->socket;
                    jugador->partida_asociada = partidas[i].id;
                    send(jugador->socket, "+Ok. Esperando a otro jugador.\n", strlen("+Ok. Esperando a otro jugador.\n"), 0);

                    break;
                }else{
                    partidas[i].jugador2.socket = jugador->socket;
                    jugador->partida_asociada = partidas[i].id;
                    partidas[i].is_started = true;
                    jugador->en_partida = true;
                    Jugador *jug1temp = encontrar_jugador_por_socket(partidas[i].jugador1.socket);
                    jug1temp->en_partida = true;
                    jug1temp->mi_turno = true;
                    send(jugador->socket, "+Ok. Partida encontrada, espera tu turno.\n", strlen("+Ok. Partida encontrada, espera tu turno.\n"), 0);
                    send(partidas[i].jugador1.socket, "+Ok. Partida encontrada, es tu turno.\n", strlen("+Ok. Partida encontrada, es tu turno.\n"), 0);
                    break;
                }
            }
        }
    }

    // Comando: PEDIR-CARTA
    else if (strcmp(buffer, "PEDIR-CARTA\n") == 0 && jugador->mi_turno && jugador->en_partida && !jugador->plantado) {
        int carta = repartir_carta();
        Partida *partida_actual = encontrar_partida_por_id(jugador->partida_asociada);
        int socket_contrincante = -1;
        Jugador *contrincante = NULL;
        if(jugador->socket == partida_actual->jugador1.socket){
            socket_contrincante = partida_actual->jugador2.socket;
        }else{
            socket_contrincante = partida_actual->jugador1.socket;
        }
        jugador->cartas[jugador->num_cartas++] = carta;
        jugador->puntuacion += carta;

        char mensaje[MAX_BUFFER];
        sprintf(mensaje, "+Ok. Carta recibida: %d. Puntuación: %d\n", carta, jugador->puntuacion);
        send(jugador->socket, mensaje, strlen(mensaje), 0);
        contrincante = encontrar_jugador_por_socket(socket_contrincante);
        contrincante->mi_turno = true;
        if(!contrincante->plantado){
            send(jugador->socket, "+Ok. Turno del otro jugador\n", strlen("+Ok. Turno del otro jugador\n"), 0);
            send(socket_contrincante, "+Ok. Turno de partida\n", strlen("+Ok. Turno de partida\n"), 0);
        }else{
            send(jugador->socket, "+Ok. Turno de partida\n", strlen("+Ok. Turno de partida\n"), 0);
        }

        if(jugador->puntuacion == 21){
            send(jugador->socket, "+Ok. Has sacado 21. Has ganado la partida\n", strlen("+Ok. Has sacado 21. Has ganado la partida\n"), 0);
            send(socket_contrincante, "+Ok. Tu contrincante ha sacado de 21. Has perdido la partida\n", strlen("+Ok. Tu contrincante ha sacado de 21. Has perdido la partida\n"), 0);
            contrincante->en_partida = false;
            contrincante->mi_turno = false;
            jugador->en_partida = false;
            jugador->mi_turno = false;
            jugador->plantado = false;
            contrincante->plantado = false;
            jugador->preparado_para_partida = false;
            contrincante->preparado_para_partida = false;
            partida_actual->is_started = false;
            jugador->partida_asociada = -1;
            contrincante-> partida_asociada = -1;
            send(jugador->socket, "+Ok. Partida finalizada\n", strlen("+Ok. Partida finalizada\n"), 0);
            send(contrincante->socket, "+Ok. Partida finalizada\n", strlen("+Ok. Partida finalizada\n"), 0);
            partida_actual->jugador1.socket = -1;
            partida_actual->jugador2.socket = -1;
            partida_actual->is_started = false;
        }

        if (jugador->puntuacion > 21) {
            send(jugador->socket, "+Ok. Te has pasado de 21. Has perdido la partida\n", strlen("+Ok. Te has pasado de 21. Has perdido la partida\n"), 0);
            send(socket_contrincante, "+Ok. Tu contrincante se ha pasado de 21. Has ganado la partida\n", strlen("+Ok. Tu contrincante se ha pasado de 21. Has ganado la partida\n"), 0);
            contrincante->en_partida = false;
            contrincante->mi_turno = false;
            jugador->en_partida = false;
            jugador->mi_turno = false;
            jugador->plantado = false;
            contrincante->plantado = false;
            jugador->preparado_para_partida = false;
            contrincante->preparado_para_partida = false;
            partida_actual->is_started = false;
            jugador->partida_asociada = -1;
            contrincante-> partida_asociada = -1;
            send(jugador->socket, "+Ok. Partida finalizada\n", strlen("+Ok. Partida finalizada\n"), 0);
            send(contrincante->socket, "+Ok. Partida finalizada\n", strlen("+Ok. Partida finalizada\n"), 0);
            partida_actual->jugador1.socket = -1;
            partida_actual->jugador2.socket = -1;
            partida_actual->is_started = false;

        }
        if(!contrincante->plantado){
            jugador->mi_turno = false;
            contrincante->mi_turno = true;
            }
        
    } else if (strcmp(buffer, "PEDIR-CARTA\n") == 0) {
        send(jugador->socket, "-Err. No es tu turno o no estás en una partida\n", strlen("-Err. No es tu turno o no estás en una partida\n"), 0);
    }
    // Comando: PLANTARME
    else if (strcmp(buffer, "PLANTARME\n") == 0) {
        if(jugador->mi_turno && jugador->en_partida){
            send(jugador->socket, "+Ok. Te has plantado\n", strlen("+Ok. Te has plantado\n"), 0);
            jugador->mi_turno = false;
            jugador->plantado = true;
            Partida *partida_actual = encontrar_partida_por_id(jugador->partida_asociada);
            int socket_contrincante = -1;
            if(jugador->socket == partida_actual->jugador1.socket){
                socket_contrincante = partida_actual->jugador2.socket;
            }else{
                socket_contrincante = partida_actual->jugador1.socket;
            }
            Jugador *contrincante = encontrar_jugador_por_socket(socket_contrincante);
            if(!contrincante->plantado){
                contrincante->mi_turno = true;
                send(socket_contrincante, "+Ok. Turno de partida\n", strlen("+Ok. Turno de partida\n"), 0);
            }

            if(contrincante->plantado && jugador->plantado){
                if(contrincante->puntuacion > jugador->puntuacion){
                    send(jugador->socket, "+Ok. Has perdido\n", strlen("+Ok. Has perdido\n"), 0);
                    send(contrincante->socket, "+Ok. Has ganado\n", strlen("+Ok. Has ganado\n"), 0);
                }else if(contrincante->puntuacion < jugador->puntuacion){
                    send(jugador->socket, "+Ok. Has ganado\n", strlen("+Ok. Has ganado\n"), 0);
                    send(contrincante->socket, "+Ok. Has perdido\n", strlen("+Ok. Has perdido\n"), 0);
                }else{
                    send(jugador->socket, "+Ok. Empate\n", strlen("+Ok. Empate\n"), 0);
                    send(contrincante->socket, "+Ok. Empate\n", strlen("+Ok. Empate\n"), 0);
                }
                contrincante->en_partida = false;
                contrincante->mi_turno = false;
                jugador->en_partida = false;
                jugador->mi_turno = false;
                jugador->plantado = false;
                contrincante->plantado = false;
                jugador->preparado_para_partida = false;
                contrincante->preparado_para_partida = false;
                partida_actual->is_started = false;
                jugador->partida_asociada = -1;
                contrincante-> partida_asociada = -1;
                send(jugador->socket, "+Ok. Partida finalizada\n", strlen("+Ok. Partida finalizada\n"), 0);
                send(contrincante->socket, "+Ok. Partida finalizada\n", strlen("+Ok. Partida finalizada\n"), 0);
                partida_actual->jugador1.socket = -1;
                partida_actual->jugador2.socket = -1;
                partida_actual->is_started = false;
            }

        } else{
            send(jugador->socket, "-Err. No es tu turno o no estás en una partida\n", strlen("-Err. No es tu turno o no estás en una partida\n"), 0);
        }
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
        if(jugador->en_partida == true){
            Partida *partida_actual = encontrar_partida_por_id(jugador->partida_asociada);
            Jugador *contrincante = NULL;
            int socket_contrincante = -1;
            if(jugador->socket == partida_actual->jugador1.socket){
                socket_contrincante = partida_actual->jugador2.socket;
            }else{
                socket_contrincante = partida_actual->jugador1.socket;
            }
            contrincante = encontrar_jugador_por_socket(socket_contrincante);
            contrincante->en_partida = false;
            contrincante->mi_turno = false;
            jugador->en_partida = false;
            jugador->mi_turno = false;
            jugador->plantado = false;
            contrincante->plantado = false;
            jugador->preparado_para_partida = false;
            contrincante->preparado_para_partida = false;
            partida_actual->is_started = false;
            jugador->partida_asociada = -1;
            contrincante-> partida_asociada = -1;
            partida_actual->jugador1.socket = -1;
            partida_actual->jugador2.socket = -1;
            partida_actual->is_started = false;
            send(jugador->socket, "+Ok. Has salido de la partida\n", strlen("+Ok. Has salido de la partida\n"), 0);
            send(contrincante->socket, "+Ok. Tu contrincante ha salido de la partida\n", strlen("+Ok. Tu contrincante ha salido de la partida\n"), 0);
            send(jugador->socket, "+Ok. Partida finalizada\n", strlen("+Ok. Partida finalizada\n"), 0);
            send(contrincante->socket, "+Ok. Partida finalizada\n", strlen("+Ok. Partida finalizada\n"), 0);
        } else{
            send(jugador->socket, "-Err. No estas en una partida\n", strlen("-Err. No estas en una partida\n"), 0);
        }
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

    // Inicializar jugadores y partidas
    inicializar_partidas();
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
                    jugadores[i].socket = -1;
                    num_jugadores--;
                }
            }
        }
    }

    close(sd);
    return 0;
}
