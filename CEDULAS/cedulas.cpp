#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
using namespace std;

// Enumeracion para el resultado de la validacion
enum ResultadoCedula { FORMATO_INVALIDO, CEDULA_VALIDA, CEDULA_INVALIDA };

// Estructura para almacenar informacion de la cedula
struct InfoCedula {
    char* numero; // memoria dinamica para la cedula
    int suma;
    int residuo;
    int resultadoEsperado;
    ResultadoCedula resultado;
};

// Union para mostrar el resultado como texto o como codigo
union ResultadoUnion {
    ResultadoCedula codigo;
    const char* texto;
};

// Funcion para pedir una cedula por consola si el archivo esta vacio
void pedirCedulaPorConsola(string& cedula) {
    cout << "El archivo cedula.txt esta vacio. Ingrese una cedula: ";
    getline(cin, cedula);
}

// Funcion recursiva para calcular la suma de los productos de los primeros 9 digitos
int sumaRecursiva(const int* ced, const int* coef, int i) {
    if (i == 9) return 0;
    int mult = ced[i] * coef[i];
    if (mult > 9) mult = (mult / 10) + (mult % 10);
    return mult + sumaRecursiva(ced, coef, i + 1);
}

// Funcion que valida la cedula y calcula suma, residuo y digito esperado
bool cedulaValida(int ced[], int& suma, int& residuo, int& result) {
    int provincia = ced[0] * 10 + ced[1]; // Dos primeros digitos: provincia
    if (provincia < 1 || provincia > 24) return false; // Provincia valida
    if (ced[2] >= 6) return false; // Tercer digito menor que 6
    int coef[] = {2, 1, 2, 1, 2, 1, 2, 1, 2}; // Coeficientes para calculo
    suma = sumaRecursiva(ced, coef, 0); // Uso de funcion recursiva
    residuo = suma % 10; // Residuo de la suma
    result = (residuo == 0) ? 0 : 10 - residuo; // Digito verificador esperado
    return ced[9] == result; // Compara con el ultimo digito
}

int main() {
    ifstream entrada("cedula.txt", ios::binary); // Archivo de entrada
    ofstream salida("resultado.txt"); // Archivo de salida

    if (!entrada) { // Si no se puede abrir el archivo de entrada
        cout << "No se pudo abrir el archivo cedula.txt" << endl;
        return 1;
    }

    // Verifica si el archivo esta vacio
    entrada.seekg(0, ios::end);
    if (entrada.tellg() == 0) {
        entrada.close();
        string cedula;
        pedirCedulaPorConsola(cedula); // Pide cedula por consola

        InfoCedula info;
        info.numero = new char[11];
        strncpy(info.numero, cedula.c_str(), 10);
        info.numero[10] = '\0';

        // Valida formato de la cedula ingresada
        if (cedula.length() != 10 || cedula.find_first_not_of("0123456789") != string::npos) {
            info.resultado = FORMATO_INVALIDO;
        } else {
            int ced[10];
            for (int i = 0; i < 10; i++) ced[i] = cedula[i] - '0'; // Convierte a enteros
            bool valida = cedulaValida(ced, info.suma, info.residuo, info.resultadoEsperado); // Valida cedula
            info.resultado = valida ? CEDULA_VALIDA : CEDULA_INVALIDA;
        }

        ResultadoUnion ru;
        switch (info.resultado) {
            case FORMATO_INVALIDO: ru.texto = "Formato invalido"; break;
            case CEDULA_VALIDA:    ru.texto = "Cedula valida"; break;
            case CEDULA_INVALIDA:  ru.texto = "Cedula invalida"; break;
        }

        salida << "Cedula: " << info.numero;
        cout << "Cedula: " << info.numero;
        if (info.resultado == FORMATO_INVALIDO) {
            salida << "\nResultado: " << ru.texto << "\n" << endl;
            cout << "\nResultado: " << ru.texto << "\n" << endl;
        } else {
            salida << "\nSuma: " << info.suma << ", Residuo: " << info.residuo << ", Resultado esperado: " << info.resultadoEsperado;
            cout << "\nSuma: " << info.suma << ", Residuo: " << info.residuo << ", Resultado esperado: " << info.resultadoEsperado;
            salida << "\nResultado: " << ru.texto << "\n" << endl;
            cout << "\nResultado: " << ru.texto << "\n" << endl;
        }
        delete[] info.numero;
        salida.close();
        return 0;
    }

    // Si el archivo no esta vacio, procesa cada linea
    entrada.clear();
    entrada.seekg(0, ios::beg);
    string linea;
    while (getline(entrada, linea)) {
        InfoCedula info;
        info.numero = new char[11];
        strncpy(info.numero, linea.c_str(), 10);
        info.numero[10] = '\0';

        // Valida formato de la linea
        if (linea.length() != 10 || linea.find_first_not_of("0123456789") != string::npos) {
            info.resultado = FORMATO_INVALIDO;
        } else {
            int ced[10];
            for (int i = 0; i < 10; i++) ced[i] = linea[i] - '0'; // Convierte a enteros
            bool valida = cedulaValida(ced, info.suma, info.residuo, info.resultadoEsperado); // Valida cedula
            info.resultado = valida ? CEDULA_VALIDA : CEDULA_INVALIDA;
        }

        ResultadoUnion ru;
        switch (info.resultado) {
            case FORMATO_INVALIDO: ru.texto = "Formato invalido"; break;
            case CEDULA_VALIDA:    ru.texto = "Cedula valida"; break;
            case CEDULA_INVALIDA:  ru.texto = "Cedula invalida"; break;
        }

        salida << "Cedula: " << info.numero;
        cout << "Cedula: " << info.numero;
        if (info.resultado == FORMATO_INVALIDO) {
            salida << "\nResultado: " << ru.texto << "\n" << endl;
            cout << "\nResultado: " << ru.texto << "\n" << endl;
        } else {
            salida << "\nSuma: " << info.suma << ", Residuo: " << info.residuo << ", Resultado esperado: " << info.resultadoEsperado;
            cout << "\nSuma: " << info.suma << ", Residuo: " << info.residuo << ", Resultado esperado: " << info.resultadoEsperado;
            salida << "\nResultado: " << ru.texto << "\n" << endl;
            cout << "\nResultado: " << ru.texto << "\n" << endl;
        }
        delete[] info.numero;
    }

    entrada.close();
    salida.close();
    return 0;
}
// Ahora puedes poner una cedula por linea, como 1717171717,