//proyecto troll

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

// Simulacion de la funcion contarRupturas
int contarRupturas(int fuerza, int longitud, int profundidad) {
    if (fuerza <= 0) return -1;
    if (profundidad > 1000) return -2; // Proteccion contra recursion infinita
    if (longitud <= fuerza) return 1;
    return 1 + contarRupturas(fuerza, longitud / 2, profundidad + 1);
}

int main() {
    ifstream archivoEntrada("entrada.txt");
    ofstream archivoSalida("salida.txt");
    if (!archivoEntrada.is_open() || !archivoSalida.is_open()) {
        cerr << "No se pudo abrir archivo de entrada o salida." << endl;
        return 1;
    }

    string linea;
    int caso = 1;
    const int LIMITE = 1000000000; // Limite razonable para fuerza y longitud

    while (getline(archivoEntrada, linea)) {
        if (linea.empty() || linea[0] == '#') continue;

        istringstream iss(linea);
        long long fuerza_ll, longitud_ll;
        string extra;
        // Leer fuerza y longitud, validar que sean numeros enteros positivos y no haya basura extra
        if (!(iss >> fuerza_ll >> longitud_ll) || (iss >> extra) ||
            fuerza_ll < 0 || longitud_ll < 0 ||
            fuerza_ll > LIMITE || longitud_ll > LIMITE) {
            string msg = "Caso #" + to_string(caso++) + " - Linea invalida, contiene letras, numeros fuera de rango o datos extra.\n";
            archivoSalida << msg;
            cout << msg;
            continue;
        }

        int fuerza = static_cast<int>(fuerza_ll);
        int longitud = static_cast<int>(longitud_ll);

        if (fuerza == 0 && longitud == 0) break;

        int rupturas = contarRupturas(fuerza, longitud, 0);

        string msg;
        if (rupturas == -1) {
            msg = "Caso #" + to_string(caso) + " - Fuerza invalida (cero o negativa).\n";
        } else if (rupturas == -2) {
            msg = "Caso #" + to_string(caso) + " - Demasiada recursion (posible ciclo infinito).\n";
        } else {
            msg = "Caso #" + to_string(caso) + " - Fuerza: " + to_string(fuerza)
                + ", Longitud: " + to_string(longitud)
                + " => Eslabones rotos: " + to_string(rupturas) + "\n";
        }
        archivoSalida << msg;
        cout << "Entrada: " << fuerza << " " << longitud << " | " << msg;
        caso++;
    }

    archivoEntrada.close();
    archivoSalida.close();
    return 0;
}
