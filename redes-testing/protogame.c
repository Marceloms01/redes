

// juego.c
#include <stdio.h>
#include <stdlib.h>
#include "protogame.h"
#include <string.h>
#define NUM_CARTAS 52

void inicializarMazo(Carta mazo[]) {
    const char *palos[] = {"Picas", "Corazones", "Diamantes", "Tréboles"};
    int k = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 1; j <= 13; j++) {
            mazo[k].valor = j;
            strcpy(mazo[k].palo, palos[i]); // Ya no causará segfault
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
void calcularPuntuacion(void *jugador) {
    if (jugador == NULL) {
        printf("Error: jugador nulo.\n");
        return;
    }
    Jugador *j = (Jugador *)jugador; // Evitar múltiples conversiones

    j->puntuacion = 0;
    int ases = 0;

    for (int i = 0; i < j->num_cartas; i++) {
        int valor_real = obtenerValorReal(j->mano[i].valor);
        j->puntuacion += valor_real;
        if (valor_real == 1) {
            ases++;
        }
    }

    // Ajustar la puntuación si hay ases
    while (ases > 0 && j->puntuacion + 10 <= 21) {
        j->puntuacion += 10;
        ases--;
    }
}

// Obtener el valor real de una carta según Blackjack
int obtenerValorReal(int valorCarta) {
    return (valorCarta >= 10) ? 10 : valorCarta; // Las cartas J, Q, K valen 10
}

// Mostrar la mano del jugador
void mostrarMano(void *jugador) {
    if (jugador == NULL) {
        printf("Error: jugador nulo.\n");
        return;
    }
    Jugador *j = (Jugador *)jugador;

    printf("Mano actual:\n");
    for (int i = 0; i < j->num_cartas; i++) {
        mostrarCarta(j->mano[i]);
    }
    printf("Puntuación actual: %d\n", j->puntuacion);
}
