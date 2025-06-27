#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <ctime>
using namespace std;

const int MAX = 10; // Tamaño de la matriz (10x10)
const int CELL_SIZE = 48; // Tamaño (pixeles) de cada celda del laberinto
const int SCREEN_W = MAX * CELL_SIZE + 250; // Ancho total de la ventana gráfica
const int SCREEN_H = MAX * CELL_SIZE + 100; // Alto total de la ventana gráfica

// Definición de los estados que puede tener una celda (con bits para combinar)
enum EstadoCelda {
    NORMAL = 0,
    RECORRIDO = 1 << 0,  // Celda ya visitada en el recorrido
    CHACAL = 1 << 1,     // Celda con "chacal" (valor 0)
    ACTUAL = 1 << 2      // Celda en la posición actual del recorrido
};

// Unión para guardar un dato extra: energía o símbolo (demostración)
union InfoExtra {
    int energia;
    char simbolo;
};

// Colores usados para representar diferentes estados en Allegro
const ALLEGRO_COLOR COLOR_FONDO = al_map_rgb(32, 32, 64);
const ALLEGRO_COLOR COLOR_NORMAL = al_map_rgb(70, 70, 120);
const ALLEGRO_COLOR COLOR_RECORRIDO = al_map_rgb(100, 200, 100);
const ALLEGRO_COLOR COLOR_ACTUAL = al_map_rgb(255, 255, 0);
const ALLEGRO_COLOR COLOR_CHACAL = al_map_rgb(200, 50, 50);
const ALLEGRO_COLOR COLOR_TEXTO = al_map_rgb(255, 255, 255);
const ALLEGRO_COLOR COLOR_BORDE = al_map_rgb(150, 150, 200);
const ALLEGRO_COLOR COLOR_PANEL = al_map_rgb(40, 40, 80);
const ALLEGRO_COLOR COLOR_BOTON = al_map_rgb(200, 50, 50);

// Estructura que guarda la información de cada paso en el recorrido
struct Paso {
    int f, c;       // Fila y columna actuales
    int suma;       // Suma acumulada hasta este paso
    int chacales;   // Conteo de chacales encontrados hasta este paso
};

// Función recursiva que suma el campo 'suma' de todos los pasos hasta el índice dado
int sumaRecursiva(const vector<Paso>& pasos, int idx) {
    if (idx < 0) return 0;  // Caso base: no hay más pasos
    return pasos[idx].suma + sumaRecursiva(pasos, idx - 1);
}

// Función para leer la matriz desde un archivo de texto
bool leerMatriz(const char* filename, int M[MAX][MAX]) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error al abrir el archivo " << filename << endl;
        return false;
    }
    // Lee 10x10 números separados por espacios o saltos de línea
    for (int i = 0; i < MAX; i++)
        for (int j = 0; j < MAX; j++)
            file >> M[i][j];
    file.close();
    return true;
}

// Función que realiza el recorrido diagonal y luego por la última columna
// Registra cada paso con su posición, suma y número de chacales encontrados
vector<Paso> resolverLaberinto(int M[MAX][MAX], int& posF, int& posC, vector<vector<int>>& estados) {
    vector<Paso> pasos;    // Vector para almacenar los pasos del recorrido
    int suma = 0, chacales = 0;
    posF = 0; posC = 0;

    // Recorre la diagonal principal de la matriz
    for (int i = 0; i < MAX; i++) {
        posF = i; posC = i;
        if (M[i][i] == 0) {
            chacales++;                  // Cuenta chacal si celda es 0
            estados[i][i] |= CHACAL;     // Marca celda como chacal
        } else {
            suma += M[i][i];             // Acumula el valor si no es chacal
        }
        estados[i][i] |= RECORRIDO;      // Marca celda como parte del recorrido
        pasos.push_back({posF, posC, suma, chacales}); // Guarda paso actual
        if (chacales == 3) return pasos;  // Termina si se encuentran 3 chacales
    }

    // Recorre la última columna desde abajo hacia arriba (excepto la última fila ya recorrida)
    for (int i = MAX - 2; i >= 0; i--) {
        posF = i; posC = MAX - 1;
        if (M[i][MAX - 1] == 0) {
            chacales++;
            estados[i][MAX - 1] |= CHACAL;
        } else {
            suma += M[i][MAX - 1];
        }
        estados[i][MAX - 1] |= RECORRIDO;
        pasos.push_back({posF, posC, suma, chacales});
        if (chacales == 3) return pasos;
    }
    return pasos;
}

// Función para dibujar una celda estilo 8-bit con color y valor
void dibujarCelda8Bit(int x, int y, ALLEGRO_COLOR color, int valor, ALLEGRO_FONT* font) {
    al_draw_filled_rectangle(x, y, x + CELL_SIZE, y + CELL_SIZE, color); // Fondo color
    al_draw_rectangle(x, y, x + CELL_SIZE, y + CELL_SIZE, COLOR_BORDE, 2); // Borde celda
    // Sombras para efecto 3D
    al_draw_line(x+1, y+1, x+CELL_SIZE-1, y+1, al_map_rgba(0,0,0,60), 1);
    al_draw_line(x+1, y+1, x+1, y+CELL_SIZE-1, al_map_rgba(0,0,0,60), 1);

    // Dibuja "X" si valor es cero (chacal), sino el número
    if (valor == 0)
        al_draw_text(font, COLOR_TEXTO, x + CELL_SIZE/2, y + CELL_SIZE/2 - 8, ALLEGRO_ALIGN_CENTRE, "X");
    else
        al_draw_textf(font, COLOR_TEXTO, x + CELL_SIZE/2, y + CELL_SIZE/2 - 8, ALLEGRO_ALIGN_CENTRE, "%d", valor);
}

// Dibuja todo el laberinto con colores según estado, además del panel lateral informativo
void dibujarLaberinto8Bit(int M[MAX][MAX], vector<vector<int>>& estados, int pasoIdx, Paso paso, ALLEGRO_FONT* font) {
    al_clear_to_color(COLOR_FONDO); // Limpia fondo

    // Dibuja fondo del laberinto
    al_draw_filled_rectangle(20, 20, 20 + MAX*CELL_SIZE, 20 + MAX*CELL_SIZE, al_map_rgb(20,20,50));

    // Dibuja cada celda con color según su estado
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            ALLEGRO_COLOR color = COLOR_NORMAL;
            if (estados[i][j] & CHACAL) color = COLOR_CHACAL;
            else if (estados[i][j] & RECORRIDO) color = COLOR_RECORRIDO;
            if (i == paso.f && j == paso.c) color = COLOR_ACTUAL;
            dibujarCelda8Bit(20 + j*CELL_SIZE, 20 + i*CELL_SIZE, color, M[i][j], font);
        }
    }

    // Dibuja panel lateral derecho con fondo y borde
    al_draw_filled_rectangle(MAX*CELL_SIZE + 40, 20, SCREEN_W - 20, SCREEN_H - 20, COLOR_PANEL);
    al_draw_rectangle(MAX*CELL_SIZE + 40, 20, SCREEN_W - 20, SCREEN_H - 20, COLOR_BORDE, 3);

    // Titulos y lineas decorativas
    al_draw_text(font, COLOR_TEXTO, MAX*CELL_SIZE + 60, 40, 0, "LABERINTO 8-BIT");
    al_draw_line(MAX*CELL_SIZE + 60, 70, SCREEN_W - 40, 70, COLOR_BORDE, 2);

    // Informacion actual del recorrido
    al_draw_textf(font, COLOR_TEXTO, MAX*CELL_SIZE + 60, 90, 0, "PASO: %d/%d", pasoIdx+1, MAX*2-1);
    al_draw_textf(font, COLOR_TEXTO, MAX*CELL_SIZE + 60, 120, 0, "POSICION: (%d,%d)", paso.f, paso.c);
    al_draw_textf(font, COLOR_TEXTO, MAX*CELL_SIZE + 60, 150, 0, "SUMA: %d", paso.suma);
    al_draw_textf(font, COLOR_TEXTO, MAX*CELL_SIZE + 60, 180, 0, "CHACALES: %d/3", paso.chacales);

    // Leyenda de colores para entender el mapa
    al_draw_text(font, COLOR_TEXTO, MAX*CELL_SIZE + 60, 220, 0, "DATOS :");
    dibujarCelda8Bit(MAX*CELL_SIZE + 60, 250, COLOR_ACTUAL, 0, font);
    al_draw_text(font, COLOR_TEXTO, MAX*CELL_SIZE + 60 + CELL_SIZE + 10, 250 + CELL_SIZE/2 - 8, 0, "ACTUAL");
    dibujarCelda8Bit(MAX*CELL_SIZE + 60, 290, COLOR_RECORRIDO, 1, font);
    al_draw_text(font, COLOR_TEXTO, MAX*CELL_SIZE + 60 + CELL_SIZE + 10, 290 + CELL_SIZE/2 - 8, 0, "RECORRIDO");
    dibujarCelda8Bit(MAX*CELL_SIZE + 60, 330, COLOR_CHACAL, 0, font);
    al_draw_text(font, COLOR_TEXTO, MAX*CELL_SIZE + 60 + CELL_SIZE + 10, 330 + CELL_SIZE/2 - 8, 0, "CHACAL");
    dibujarCelda8Bit(MAX*CELL_SIZE + 60, 370, COLOR_NORMAL, 1, font);
    al_draw_text(font, COLOR_TEXTO, MAX*CELL_SIZE + 60 + CELL_SIZE + 10, 370 + CELL_SIZE/2 - 8, 0, "NORMAL");

    // Controles del programa
    al_draw_text(font, COLOR_TEXTO, MAX*CELL_SIZE + 60, 420, 0, "CONTROLES:");
    al_draw_text(font, COLOR_TEXTO, MAX*CELL_SIZE + 60, 450, 0, "ESPACIO: SIGUIENTE");
    al_draw_text(font, COLOR_TEXTO, MAX*CELL_SIZE + 60, 480, 0, "PULSA X PARA SALIR");

    // Boton grafico para salir
    al_draw_filled_rectangle(SCREEN_W - 60, 20, SCREEN_W - 20, 60, COLOR_BOTON);
    al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_W - 40, 30, ALLEGRO_ALIGN_CENTER, "X");

    al_flip_display(); // Actualiza la pantalla con todo lo dibujado
}

// Función para generar un nombre único con fecha y hora para el archivo de salida
string generarNombreArchivo() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buffer[64];
    sprintf(buffer, "resultado_%04d%02d%02d_%02d%02d%02d.txt",
        1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday,
        ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    return string(buffer);
}

int main() {
    // Inicialización de Allegro y addons necesarios
    if (!al_init()) { cerr << "Error al inicializar Allegro" << endl; return -1; }
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_primitives_addon();
    al_init_image_addon();
    al_install_keyboard();
    al_install_mouse();

    // Crear ventana con ancho y alto definidos
    ALLEGRO_DISPLAY* display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display) { cerr << "Error al crear la ventana" << endl; return -1; }

    // Cargar fuente para dibujar texto (prueba con fuente alternativa)
    ALLEGRO_FONT* font = al_load_ttf_font("PressStart2P.ttf", 12, 0);
    if (!font) font = al_load_ttf_font("arial.ttf", 10, 0);

    int M[MAX][MAX];
    if (!leerMatriz("laberinto.txt", M)) return -1;  // Lee matriz de archivo, si falla termina

    // Vector 2D para almacenar estados de cada celda (normal, recorrido, chacal, actual)
    vector<vector<int>> estados(MAX, vector<int>(MAX, 0));
    int posF, posC;
    // Resolver recorrido y obtener pasos
    vector<Paso> pasos = resolverLaberinto(M, posF, posC, estados);

    // Crear cola de eventos para teclado, ratón y ventana
    ALLEGRO_EVENT_QUEUE* event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_display_event_source(display));

    int pasoIdx = 0;          // Índice del paso actual mostrado
    bool running = true;      // Controla el loop principal

    // Dibuja el laberinto con el primer paso
    dibujarLaberinto8Bit(M, estados, pasoIdx, pasos[pasoIdx], font);

    // Loop principal para manejo de eventos
    while (running) {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);

        if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (event.keyboard.keycode == ALLEGRO_KEY_SPACE && pasoIdx < (int)pasos.size() - 1) {
                pasoIdx++; // Avanza al siguiente paso con espacio
                dibujarLaberinto8Bit(M, estados, pasoIdx, pasos[pasoIdx], font);
            }
            if (event.keyboard.keycode == ALLEGRO_KEY_X) running = false; // Salir con X
        }
        // Salir si se hace click en el botón rojo X
        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
            event.mouse.x >= SCREEN_W - 60 && event.mouse.x <= SCREEN_W - 20 &&
            event.mouse.y >= 20 && event.mouse.y <= 60)
            running = false;

        // Salir al cerrar ventana
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) running = false;
    }

    // Guardar resultados en archivo con nombre único
    string nombreArchivo = generarNombreArchivo();
    ofstream out(nombreArchivo);
    out << "Posicion final: (" << pasos.back().f << ", " << pasos.back().c << ")\n";
    out << "Suma acumulada: " << pasos.back().suma << endl;
    out << "Chacales encontrados: " << pasos.back().chacales << endl;

    // Uso de unión para mostrar energía (como ejemplo)
    InfoExtra extra;
    extra.energia = pasos.back().suma;
    out << "Energia (via union): " << extra.energia << endl;

    // Suma total calculada con función recursiva
    int sumaTotal = sumaRecursiva(pasos, pasos.size() - 1);
    out << "Suma recursiva total: " << sumaTotal << endl;
    out.close();

    cout << "Resultado guardado en: " << nombreArchivo << endl;

    // Liberar recursos Allegro antes de salir
    al_destroy_font(font);
    al_destroy_event_queue(event_queue);
    al_destroy_display(display);
    return 0;
}
// g++ laberinto.cpp -o laberinto -lallegro -lallegro_font -lallegro_ttf -lallegro_primitives -lallegro_image -std=c++17
