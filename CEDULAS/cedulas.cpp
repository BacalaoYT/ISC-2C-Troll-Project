#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <filesystem> // Agrega esto
using namespace std;

// Resultado posible de la validacion
enum ResultadoCedula { FORMATO_INVALIDO, CEDULA_VALIDA, CEDULA_INVALIDA };

// Estructura para guardar los datos de una cedula
struct InfoCedula {
    char* numero;            // Cedula (memoria dinamica)
    int suma;                // Suma de productos
    int residuo;             // Modulo 10
    int resultadoEsperado;   // Digito esperado
    ResultadoCedula resultado; // Resultado final
};

// Union para mostrar texto o codigo del resultado
union ResultadoUnion {
    ResultadoCedula codigo;
    const char* texto;
};

// Pide una cedula por consola si el archivo esta vacio
void pedirCedulaPorConsola(string& cedula) {
    cout << "El archivo cedulass.txt esta vacio. Ingrese una cedula: ";
    getline(cin, cedula);
}

// Funcion recursiva para calcular la suma con coeficientes
int sumaRecursiva(const int* ced, const int* coef, int i) {
    if (i == 9) return 0; // Base de la recursion
    int mult = ced[i] * coef[i]; // Multiplica
    if (mult > 9) mult = (mult / 10) + (mult % 10); // Si es doble digito, suma sus cifras
    return mult + sumaRecursiva(ced, coef, i + 1); // Llamado recursivo
}

// Verifica si la cedula es valida segun reglas del Ecuador
bool cedulaValida(int ced[], int& suma, int& residuo, int& result) {
    int provincia = ced[0] * 10 + ced[1]; // Provincia = dos primeros digitos
    if (provincia < 1 || provincia > 24) return false;
    if (ced[2] >= 6) return false;
    int coef[] = {2, 1, 2, 1, 2, 1, 2, 1, 2}; // Coeficientes de validacion
    suma = sumaRecursiva(ced, coef, 0); // Calcula suma recursiva
    residuo = suma % 10; // Modulo 10
    result = (residuo == 0) ? 0 : 10 - residuo; // Digito verificador
    return ced[9] == result; // Compara con ultimo digito
}

int main() {
    // Imprime el directorio actual para depuracion
    cout << "Directorio actual: " << std::filesystem::current_path() << endl;

    FILE* entrada = fopen("cedulass.txt", "r");
    FILE* salida = fopen("resultado.txt", "w");

    if (!entrada) {
        cout << "No se pudo abrir el archivo cedulass.txt" << endl;
        if (salida) fclose(salida);
        return 1;
    }

    // Verifica si el archivo esta vacio
    fseek(entrada, 0, SEEK_END);
    long tam = ftell(entrada);
    if (tam == 0) {
        fclose(entrada);
        string cedula;
        pedirCedulaPorConsola(cedula); // Pide desde consola

        InfoCedula info;
        info.numero = new char[11];
        strncpy(info.numero, cedula.c_str(), 10);
        info.numero[10] = '\0';

        // Validar formato
        if (cedula.length() != 10 || cedula.find_first_not_of("0123456789") != string::npos) {
            info.resultado = FORMATO_INVALIDO;
        } else {
            int ced[10];
            for (int i = 0; i < 10; i++) ced[i] = cedula[i] - '0';
            bool valida = cedulaValida(ced, info.suma, info.residuo, info.resultadoEsperado);
            info.resultado = valida ? CEDULA_VALIDA : CEDULA_INVALIDA;
        }

        ResultadoUnion ru;
        switch (info.resultado) {
            case FORMATO_INVALIDO: ru.texto = "Formato invalido"; break;
            case CEDULA_VALIDA:    ru.texto = "Cedula valida"; break;
            case CEDULA_INVALIDA:  ru.texto = "Cedula invalida"; break;
        }

        // Muestra resultados
        fprintf(salida, "Cedula: %s", info.numero);
        cout << "Cedula: " << info.numero;
        if (info.resultado == FORMATO_INVALIDO) {
            fprintf(salida, "\nResultado: %s\n\n", ru.texto);
            cout << "\nResultado: " << ru.texto << "\n" << endl;
        } else {
            fprintf(salida, "\nSuma: %d, Residuo: %d, Resultado esperado: %d", info.suma, info.residuo, info.resultadoEsperado);
            fprintf(salida, "\nResultado: %s\n\n", ru.texto);

            cout << "\nSuma: " << info.suma << ", Residuo: " << info.residuo << ", Resultado esperado: " << info.resultadoEsperado;
            cout << "\nResultado: " << ru.texto << "\n" << endl;
        }

        delete[] info.numero;
        fclose(salida);
        return 0;
    }

    // Si hay cedulas en el archivo, las procesa una por una
    rewind(entrada);
    char linea[64];
    while (fgets(linea, sizeof(linea), entrada)) {
        // Elimina salto de linea si existe
        size_t len = strlen(linea);
        if (len > 0 && (linea[len - 1] == '\n' || linea[len - 1] == '\r')) linea[len - 1] = '\0';

        InfoCedula info;
        info.numero = new char[11];
        strncpy(info.numero, linea, 10);
        info.numero[10] = '\0';

        // Validacion de formato
        if (strlen(linea) != 10 || strspn(linea, "0123456789") != 10) {
            info.resultado = FORMATO_INVALIDO;
        } else {
            int ced[10];
            for (int i = 0; i < 10; i++) ced[i] = linea[i] - '0';
            bool valida = cedulaValida(ced, info.suma, info.residuo, info.resultadoEsperado);
            info.resultado = valida ? CEDULA_VALIDA : CEDULA_INVALIDA;
        }

        ResultadoUnion ru;
        switch (info.resultado) {
            case FORMATO_INVALIDO: ru.texto = "Formato invalido"; break;
            case CEDULA_VALIDA:    ru.texto = "Cedula valida"; break;
            case CEDULA_INVALIDA:  ru.texto = "Cedula invalida"; break;
        }

        // Muestra y guarda resultados
        fprintf(salida, "Cedula: %s", info.numero);
        cout << "Cedula: " << info.numero;
        if (info.resultado == FORMATO_INVALIDO) {
            fprintf(salida, "\nResultado: %s\n\n", ru.texto);
            cout << "\nResultado: " << ru.texto << "\n" << endl;
        } else {
            fprintf(salida, "\nSuma: %d, Residuo: %d, Resultado esperado: %d", info.suma, info.residuo, info.resultadoEsperado);
            fprintf(salida, "\nResultado: %s\n\n", ru.texto);

            cout << "\nSuma: " << info.suma << ", Residuo: " << info.residuo << ", Resultado esperado: " << info.resultadoEsperado;
            cout << "\nResultado: " << ru.texto << "\n" << endl;
        }

        delete[] info.numero;
    }

    fclose(entrada);
    fclose(salida);
    return 0;
}
// cedulass.txt para pruebas
