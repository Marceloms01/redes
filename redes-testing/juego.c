#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "juego.h"
#define NUM_CARTAS 52


// Prototipos de funciones
void jugarTurno(Jugador *jugador, Carta mazo[], int *indice);
int obtenerValorReal(int valorCarta);



// Inicializar mazo con valores y palos
void inicializarMazo(Carta mazo[]) {
    char *palos[] = {"Picas", "Corazones", "Diamantes", "Tréboles"};
    int k = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 1; j <= 13; j++) {
            mazo[k].valor = j;
            mazo[k].palo = palos[i];
            k++;
        }
    }
}

// Barajar el mazo
void barajar(Carta mazo[]) {
    for (int i = 0; i < NUM_CARTAS; i++) {
        int r = rand() % NUM_CARTAS;
        Carta temp = mazo[i];
        mazo[i] = mazo[r];
        mazo[r] = temp;
    }
}

// Robar una carta del mazo
Carta robarCarta(Carta mazo[], int *indice) {
    return mazo[(*indice)++];
}

// Mostrar una carta
void mostrarCarta(Carta carta) {
    const char *nombres_cartas[] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
    printf("%s de %s\n", nombres_cartas[carta.valor - 1], carta.palo);
}

// Calcular la puntuación del jugador
void calcularPuntuacion(Jugador *jugador) {
    jugador->puntuacion = 0;
    int ases = 0;

    for (int i = 0; i < jugador->num_cartas; i++) {
        int valor_real = obtenerValorReal(jugador->mano[i].valor);
        jugador->puntuacion += valor_real;
        if (valor_real == 1) {
            ases++;
        }
    }

    // Si hay ases y la puntuación es baja, convertir un As de 1 a 11
    while (ases > 0 && jugador->puntuacion + 10 <= 21) {
        jugador->puntuacion += 10;
        ases--;
    }
}

// Obtener el valor real de una carta según Blackjack
int obtenerValorReal(int valorCarta) {
    if (valorCarta >= 10) {
        return 10;  // Las cartas J, Q, K valen 10
    }
    return valorCarta;  // Las cartas numéricas valen su número
}

// Mostrar la mano del jugador
void mostrarMano(Jugador jugador) {
    printf("Mano actual:\n");
    for (int i = 0; i < jugador.num_cartas; i++) {
        mostrarCarta(jugador.mano[i]);
    }
    printf("Puntuación actual: %d\n", jugador.puntuacion);
}

// Jugar el turno de un jugador
void jugarTurno(Jugador *jugador, Carta mazo[], int *indice) {
    char opcion;
    while (jugador->puntuacion < 21) {
        printf("¿Quieres pedir una carta (p) o plantarte (s)? ");
        scanf(" %c", &opcion);
        if (opcion == 's') {
            break;
        } else if (opcion == 'p') {
            jugador->mano[jugador->num_cartas++] = robarCarta(mazo, indice);
            calcularPuntuacion(jugador);
            mostrarMano(*jugador);
        }
    }
}