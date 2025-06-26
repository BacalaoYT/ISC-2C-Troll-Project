#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

// Enumeracion para el resultado del caso
enum EstadoCaso { CASO_VALIDO, FUERZA_INVALIDA, DEMASIADA_RECURSION, LINEA_INVALIDA };

// Estructura para almacenar la informacion de cada caso
struct InfoCaso {
    int fuerza;
    int longitud;
    int rupturas;
    EstadoCaso estado;
    char* mensaje; // memoria dinamica para el mensaje
};

// Union para mostrar el resultado como texto o como codigo
union ResultadoUnion {
    EstadoCaso codigo;
    const char* texto;
};

// Funcion recursiva para contar rupturas
int contarRupturas(int fuerza, int longitud, int contador) {
    if (fuerza <= 0) return -1; // Fuerza invalida
    if (contador > 1000) return -2; // Demasiada recursion
    if (longitud <= fuerza) return 1; // Caso base
    return 1 + contarRupturas(fuerza, longitud / 2, contador + 1); // Llamada recursiva
}

int main() {
    ifstream archivoEntrada("entrada.txt"); // Archivo de entrada
    ofstream archivoSalida("salida.txt");   // Archivo de salida

    if (!archivoEntrada.is_open() || !archivoSalida.is_open()) {
        cerr << "No se pudo abrir archivo de entrada o salida." << endl;
        return 1;
    }

    string linea;
    int caso = 1;
    const int LIMITE = 1000000000; // Limite para los valores

    while (getline(archivoEntrada, linea)) {
        if (linea.empty() || linea[0] == '#') continue; // Ignora lineas vacias o comentarios

        istringstream iss(linea);
        long long fuerza_ll, longitud_ll;
        string extra;
        InfoCaso info;
        info.mensaje = nullptr;

        // Validar entrada: deben ser dos numeros positivos y sin basura extra
        if (!(iss >> fuerza_ll >> longitud_ll) || (iss >> extra) ||
            fuerza_ll < 0 || longitud_ll < 0 ||
            fuerza_ll > LIMITE || longitud_ll > LIMITE) {
            info.estado = LINEA_INVALIDA;
            ResultadoUnion ru;
            ru.texto = "Linea invalida, contiene letras, numeros fuera de rango o datos extra.";
            string msg = "Caso #" + to_string(caso++) + " - " + ru.texto + "\n";
            info.mensaje = new char[msg.size() + 1];
            strcpy(info.mensaje, msg.c_str());
            archivoSalida << info.mensaje; // Escribe mensaje en archivo
            cout << info.mensaje;          // Muestra mensaje en pantalla
            delete[] info.mensaje;
            continue;
        }

        info.fuerza = static_cast<int>(fuerza_ll);
        info.longitud = static_cast<int>(longitud_ll);

        if (info.fuerza == 0 && info.longitud == 0) break; // Fin de datos

        // Llama funcion recursiva para calcular rupturas
        info.rupturas = contarRupturas(info.fuerza, info.longitud, 0);

        ResultadoUnion ru;
        string msg;

        // Determina el estado y mensaje segun el resultado
        if (info.rupturas == -1) {
            info.estado = FUERZA_INVALIDA;
            ru.texto = "Fuerza invalida (cero o negativa).";
            msg = "Caso #" + to_string(caso) + " - " + ru.texto + "\n";
        } else if (info.rupturas == -2) {
            info.estado = DEMASIADA_RECURSION;
            ru.texto = "Demasiada recursion (posible ciclo infinito).";
            msg = "Caso #" + to_string(caso) + " - " + ru.texto + "\n";
        } else {
            info.estado = CASO_VALIDO;
            ru.texto = "Eslabones rotos: ";
            msg = "Caso #" + to_string(caso) + " - Fuerza: " + to_string(info.fuerza)
                + ", Longitud: " + to_string(info.longitud)
                + " => " + ru.texto + to_string(info.rupturas) + "\n";
        }

        // Reserva memoria para el mensaje y lo copia
        info.mensaje = new char[msg.size() + 1];
        strcpy(info.mensaje, msg.c_str());

        archivoSalida << "Entrada: " << info.fuerza << " " << info.longitud << " | " << info.mensaje; // Guarda en archivo
        cout << "Entrada: " << info.fuerza << " " << info.longitud << " | " << info.mensaje;         // Muestra en pantalla

        delete[] info.mensaje; // Libera memoria
        caso++;
    }

    archivoEntrada.close();
    archivoSalida.close();
    return 0;
}

// Puedes poner casos en entrada.txt, ejemplo: 5// Puedes poner casos en entrada.txt, ejemplo: 5
