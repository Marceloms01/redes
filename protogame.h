// juego.h
#ifndef PROTOGAME_H
#define PROTOGAME_H
#include <stdbool.h>
#include <string.h>
#define NUM_CARTAS 52

typedef struct {
    int valor;
    char nombre;
    char palo[10];  // Tamaño suficiente para "Corazones", "Diamantes", etc.
} Carta;

typedef struct {
    int socket;
    char username[50];
    char *password;
    int puntuacion;
    int cartas[10];
    Carta mano[11];
    int num_cartas;
    bool en_partida;
    bool mi_turno;
    bool plantado;
    bool session;
    bool preparado_para_partida;
    int partida_asociada;

} Jugador;

// Prototipos de funciones del juego
void inicializarMazo(Carta mazo[]);
void barajar(Carta mazo[]);
Carta robarCarta(Carta mazo[], int *indice);
void mostrarCarta(Carta carta);
void calcularPuntuacion(void *jugador);  // Se espera que el servidor pase un puntero a su estructura de jugador
int obtenerValorReal(int valorCarta);
void mostrarMano(void *jugador);  // Se espera que el servidor pase un puntero a su estructura de jugador

#endif // JUEGO_H
