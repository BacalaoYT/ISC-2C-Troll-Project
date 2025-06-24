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

int main() {
    int fuerza, longitud;
    int caso = 1;

    cout << "=== Transporte de cadenas por hobbits y enanos ===\n";
    cout << "Introduce pares de numeros separados por espacio:\n";
    cout << "- Primer número: fuerza de un hobbit (numero maximo de eslabones que puede cargar)\n";
    cout << "- Segundo numero: longitud de la cadena a transportar\n";
    cout << "- Introduce 0 0 para terminar.\n\n";

    while (true) {
        cout << "Caso #" << caso << " - Ingresa fuerza y longitud: ";
        cin >> fuerza >> longitud;

        if (fuerza == 0 && longitud == 0)
            break;

        int rupturas = contarRupturas(fuerza, longitud);

        cout << "Resultado -> Numero de eslabones que deben romperse: " << rupturas << "\n\n";
        caso++;
    }

    cout << "SALIENDO SISTEMA! \n";
    return 0;
}

