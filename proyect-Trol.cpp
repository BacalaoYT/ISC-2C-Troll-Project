//proyectotroll  
#include <iostream>
using namespace std;

// Funcion recursiva que calcula cuantas veces se rompe la cadena
int contarRupturas(int fuerzaHobbit, int eslabones) {
    int capacidadEnano = 2 * fuerzaHobbit;

    // Caso base: Si el segmento es menor o igual a lo que puede cargar un enano, no se rompe
    if (eslabones <= capacidadEnano) {
        return 0;
    }

    // Dividir el segmento en proporcion 2:1
    int partePequena = eslabones / 3;           // 1/3 del total
    int parteGrande = eslabones - partePequena; // 2/3 + sobrantes, si los hay

    // Se rompe una vez, y se suma la recursion de ambos fragmentos
    return 1 + contarRupturas(fuerzaHobbit, parteGrande)
             + contarRupturas(fuerzaHobbit, partePequena);
}

