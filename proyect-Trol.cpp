// proyectotroll 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIMITE_RECURSION 10000

// Funcion recursiva protegida contra desbordamiento
int contarRupturas(int fuerzaHobbit, int eslabones, int profundidad) {
    if (fuerzaHobbit <= 0) return -1;  // fuerza invalida

    if (profundidad > LIMITE_RECURSION) return -2; // evitar recursion infinita

    int capacidadEnano = 2 * fuerzaHobbit;

    if (eslabones <= capacidadEnano) {
        return 0;
    }

    int partePequena = eslabones / 3;
    if (partePequena < 1) partePequena = 1;
    int parteGrande = eslabones - partePequena;

    return 1
        + contarRupturas(fuerzaHobbit, parteGrande, profundidad + 1)
        + contarRupturas(fuerzaHobbit, partePequena, profundidad + 1);
}

int main() {
    FILE *archivoEntrada;
    FILE *archivoSalida;
    FILE *archivoPrevio;
    char respuesta[10];
    char linea[256];
    int fuerza, longitud, caso = 1;

    printf("=== Proyecto Troll  ===\n");

    archivoPrevio = fopen("salida.txt", "r");
    if (archivoPrevio != NULL) {
        printf("Se encontró un archivo de resultados anteriores.\n");
        printf("¿Deseas usarlo? (s/n): ");
        fflush(stdout);
        fgets(respuesta, sizeof(respuesta), stdin);

        if (respuesta[0] == 's' || respuesta[0] == 'S') {
            printf("\n=== Resultados anteriores ===\n");
            rewind(archivoPrevio);
            while (fgets(linea, sizeof(linea), archivoPrevio)) {
                printf("%s", linea);
            }
            fclose(archivoPrevio);
            return 0;
        }
        fclose(archivoPrevio);
    }

    archivoEntrada = fopen("entrada.txt", "r");
    if (archivoEntrada == NULL) {
        printf("Error: no se pudo abrir 'entrada.txt'. Verifica que exista.\n");
        return 1;
    }

    archivoSalida = fopen("salida.txt", "w");
    if (archivoSalida == NULL) {
        printf("Error: no se pudo crear 'salida.txt'.\n");
        fclose(archivoEntrada);
        return 1;
    }

    fprintf(archivoSalida, "=== Resultados del Proyecto Troll ===\n");

    while (fgets(linea, sizeof(linea), archivoEntrada)) {
        // Ignorar líneas vacías o que inicien con #
        if (linea[0] == '\n' || linea[0] == '#') continue;

        int leidos = sscanf(linea, "%d %d", &fuerza, &longitud);
        if (leidos != 2 || fuerza < 0 || longitud < 0) {
            fprintf(archivoSalida, "Caso #%d - Línea invalida o con datos negativos.\n", caso++);
            continue;
        }

        if (fuerza == 0 && longitud == 0) break;

        int rupturas = contarRupturas(fuerza, longitud, 0);

        if (rupturas == -1) {
            fprintf(archivoSalida, "Caso #%d - Fuerza invalida (cero o negativa).\n", caso);
        } else if (rupturas == -2) {
            fprintf(archivoSalida, "Caso #%d - Demasiada recursion (posible ciclo infinito).\n", caso);
        } else {
            fprintf(archivoSalida, "Caso #%d - Fuerza: %d, Longitud: %d => Eslabones rotos: %d\n",
                    caso, fuerza, longitud, rupturas);
        }

        caso++;
    }

    fclose(archivoEntrada);
    fclose(archivoSalida);

    printf("Proceso terminado. Resultados guardados en 'salida.txt'.\n");
    return 0;
}
