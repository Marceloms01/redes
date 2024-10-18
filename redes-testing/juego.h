#ifndef JUEGO_H
#define JUEGO_H
#include <stdbool.h>
#define NUM_CARTAS 52

// Estructura para representar una carta
typedef struct {
    int valor;  // 2-10 para cartas numéricas, 11 para J, 12 para Q, 13 para K, 1 para A
    char *palo; // Picas, Corazones, Diamantes, Tréboles
} Carta;

// Estructura para representar un jugador
typedef struct {
    int socket;
    int puntuacion;
    int num_cartas;
    char username[50];
      int cartas[10];
    char *password;
    bool en_partida;
    bool mi_turno;
    bool preparado_para_partida;
    bool session;
    Carta mano[11];  // Añade el campo mano aquí
} Jugador;

// Prototipos de las funciones del juego
void inicializarMazo(Carta mazo[]);
void barajar(Carta mazo[]);
Carta robarCarta(Carta mazo[], int *indice);
void calcularPuntuacion(Jugador *jugador);
void mostrarCarta(Carta carta);
void mostrarMano(Jugador jugador);

#endif
