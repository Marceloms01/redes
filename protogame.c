#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_CARTAS 52

// Estructura para representar una carta
typedef struct {
    int valor;  // 2-10 para cartas numéricas, 11 para J, 12 para Q, 13 para K, 1 para A
    char *palo; // Picas, Corazones, Diamantes, Tréboles
} Carta;

// Estructura para representar un jugador
typedef struct {
    Carta mano[11]; // Hasta un máximo de 11 cartas (en teoría puedes tener más, pero es raro)
    int num_cartas;
    int puntuacion;
} Jugador;

// Prototipos de funciones
void inicializarMazo(Carta mazo[]);
void barajar(Carta mazo[]);
Carta robarCarta(Carta mazo[], int *indice);
void mostrarCarta(Carta carta);
void calcularPuntuacion(Jugador *jugador);
void jugarTurno(Jugador *jugador, Carta mazo[], int *indice);
int obtenerValorReal(int valorCarta);
void mostrarMano(Jugador jugador);

int main() {
    // Inicializar mazo y jugadores
    Carta mazo[NUM_CARTAS];
    Jugador jugador1 = {{0}, 0, 0};
    Jugador jugador2 = {{0}, 0, 0};

    int indice_mazo = 0;

    srand(time(0));  // Semilla para generar números aleatorios

    inicializarMazo(mazo);
    barajar(mazo);

    printf("=== Comienza el Blackjack entre dos jugadores ===\n");

    // Repartir dos cartas a cada jugador
    jugador1.mano[jugador1.num_cartas++] = robarCarta(mazo, &indice_mazo);
    jugador1.mano[jugador1.num_cartas++] = robarCarta(mazo, &indice_mazo);
    calcularPuntuacion(&jugador1);

    jugador2.mano[jugador2.num_cartas++] = robarCarta(mazo, &indice_mazo);
    jugador2.mano[jugador2.num_cartas++] = robarCarta(mazo, &indice_mazo);
    calcularPuntuacion(&jugador2);

    // Turno del jugador 1
    printf("\nTurno del Jugador 1:\n");
    mostrarMano(jugador1);
    jugarTurno(&jugador1, mazo, &indice_mazo);

    // Turno del jugador 2
    printf("\nTurno del Jugador 2:\n");
    mostrarMano(jugador2);
    jugarTurno(&jugador2, mazo, &indice_mazo);

    // Determinar el ganador
    if (jugador1.puntuacion > 21 && jugador2.puntuacion > 21) {
        printf("\nAmbos jugadores se pasaron de 21. ¡Es un empate!\n");
    } else if (jugador1.puntuacion > 21) {
        printf("\nJugador 1 se pasó de 21. Jugador 2 gana.\n");
    } else if (jugador2.puntuacion > 21) {
        printf("\nJugador 2 se pasó de 21. Jugador 1 gana.\n");
    } else if (jugador1.puntuacion > jugador2.puntuacion) {
        printf("\nJugador 1 gana con %d puntos.\n", jugador1.puntuacion);
    } else if (jugador2.puntuacion > jugador1.puntuacion) {
        printf("\nJugador 2 gana con %d puntos.\n", jugador2.puntuacion);
    } else {
        printf("\n¡Es un empate con %d puntos cada uno!\n", jugador1.puntuacion);
    }

    return 0;
}

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