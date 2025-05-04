#include <iostream>
using namespace std;
 
#define MAXLETRA 21
 
const char letrasPermitidas[MAXLETRA] = { // Definimos el abecedario solo con CONSONANTES
    'B', 'C', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M',
    'N', 'P', 'Q', 'R', 'S', 'T', 'V', 'W', 'X', 'Y', 'Z'
};
 
struct Matriculas{ // Hacemos un incremento para el caracter nulo que se agrega
    char numeros[5];
    char letras[4];
 
} matricula;
 
//Esta funcion analiza la posicion de la letra dentro de  "Letras Permitidas"
int posicionLetra(char letra){
     for (int i = 0; i < MAXLETRA; i++){
        if (letrasPermitidas[i] == letra)
        return i;
    }
    return -1; /*Al retronar -1 damos a entender que no se encontro un valor,
                 si colocamos el 1 puede interpretarse que el valor esta en esa posicion*/
}

// Usamos esta funcion recursiva para ir recorriendo las letras
void avanzarLetras(char letras[4], int pos) {
    if (pos < 0) { // Caso base
        return;
    }

    int indice = posicionLetra(letras[pos]);
    if (indice < MAXLETRA - 1) {
        letras[pos] = letrasPermitidas[indice + 1];  // avanzamos esta posición
    } else {
        letras[pos] = letrasPermitidas[0];  // reiniciamos esta posición
        avanzarLetras(letras, pos - 1);  // avanzamos a la izquierda
    }
}

void avanzarNumeros(char numeros[5]) {
    int pos = 3; // Iniciamos en el 3 ya que es el mas a la derecha (El arreglo inicia en la pos 0)

    while (pos >= 0) {
        if (numeros[pos] < '9') {
            numeros[pos]++;  // Hacemo el incremento del digito en el que estamos
            return;          // Salimos
        } else {
            numeros[pos] = '0';  // Reiniciamos este digito a 0
            pos--;               // Nos recorremos a la izquierda
        }
    }

    /*En este punto si era 9999 se habra reiniciado a 0000, por lo que pasamos a 
    incrementar las letras*/

    avanzarLetras(matricula.letras, 2);  //(2 es la ultima posicion de las letras)
}

int main(){
 
    cout << "Ingrese la matricula de la forma (XXXX ABC): ";
    cin >> matricula.numeros >> matricula.letras;
 
}