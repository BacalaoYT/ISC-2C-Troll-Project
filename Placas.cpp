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
 
 int main(){
 
     cout << "Ingrese la matricula de la forma (XXXX ABC): ";
     cin >> matricula.numeros >> matricula.letras;
 
 }