#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <cmath>

using namespace std;

// Esta estructura guarda los saltos que hace Mario: cuantos hacia arriba y cuantos hacia abajo
struct Resultado {
    int saltos_arriba;
    int saltos_abajo;
};

// Estructura para las nubes, super simple: posicion x, y y velocidad
struct Nube { float x, y, vel; };

// Estructura para los botones de la interfaz: nombre, posicion y tamaño
struct Boton { std::string nombre; float x, y, w, h; };

// Enum para identificar facilmente cada boton por nombre
enum TipoBoton { SALIR, PAUSA, AVANZAR, RETROCEDER };

// Union para guardar temporalmente un int o un float, solo para mostrar que se puede usar
union Temporal {
    int i;
    float f;
};

// Esta funcion es recursiva y cuenta cuantos muros son mayores que un valor dado
// Es como decir: "¿Cuantos muros son mas altos que X?"
int contar_muros_mayores(const vector<int>& muros, int idx, int valor) {
    if (idx >= muros.size()) return 0;
    return (muros[idx] > valor ? 1 : 0) + contar_muros_mayores(muros, idx + 1, valor);
}

// Esta funcion procesa un escenario y cuenta los saltos de Mario
// Si el muro siguiente es mas alto, suma a saltos_arriba, si es mas bajo, suma a saltos_abajo
Resultado procesarEscenario(const vector<int>& muros) {
    Resultado res = {0, 0};
    for (size_t i = 1; i < muros.size(); ++i) {
        if (muros[i] > muros[i - 1])
            res.saltos_arriba++;
        else if (muros[i] < muros[i - 1])
            res.saltos_abajo++;
    }
    return res;
}

// Esta funcion dibuja el suelo naranja donde se paran los muros
void dibujar_suelo(float ancho, float y_base, float alto = 20) {
    ALLEGRO_COLOR color_suelo = al_map_rgb(255, 140, 0); // Naranja
    al_draw_filled_rectangle(0, y_base - alto, ancho, y_base, color_suelo);
}

// Esta funcion dibuja un boton amarillo con el texto centrado
// El texto siempre queda bien en el centro, no importa el tamaño del boton
void dibujar_boton(float x, float y, float w, float h, const char* texto, ALLEGRO_FONT* font, ALLEGRO_COLOR color) {
    al_draw_filled_rectangle(x, y, x + w, y + h, al_map_rgb(255, 255, 0)); // Amarillo
    al_draw_rectangle(x, y, x + w, y + h, al_map_rgb(100, 100, 100), 2);
    int tx = x + w / 2;
    int ty = y + (h - al_get_font_line_height(font)) / 2;
    al_draw_text(font, color, tx, ty, ALLEGRO_ALIGN_CENTER, texto);
}

// Esta funcion checa si el mouse esta sobre un boton
// Devuelve true si el mouse esta dentro del rectangulo del boton
bool mouse_sobre(float mx, float my, float x, float y, float w, float h) {
    return mx >= x && mx <= x + w && my >= y && my <= y + h;
}

int main() {
    // Aqui empieza todo: inicializamos Allegro y los addons para graficos, fuentes, etc
    al_init();
    al_install_keyboard();
    al_install_mouse();
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_primitives_addon();
    al_init_image_addon();

    // Creamos la ventana de 800x600 y preparamos todo para dibujar
    int ancho = 800, alto = 600;
    ALLEGRO_DISPLAY* display = al_create_display(ancho, alto);
    al_set_window_title(display, "Mario Saltarin");
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60);
    ALLEGRO_FONT* font = al_load_ttf_font("C:\\Windows\\Fonts\\arial.ttf", 20, 0);
    if (!font) {
        cerr << "Error cargando fuente." << endl;
        return -1;
    }

    // Registramos las fuentes de eventos para teclado, mouse, ventana y timer
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_mouse_event_source());

    // Colores para muros y texto
    ALLEGRO_COLOR color_muro = al_map_rgb(255, 0, 0);
    ALLEGRO_COLOR color_texto = al_map_rgb(0, 0, 0);

    // Cargamos las imagenes de muro y mario (si existen)
    ALLEGRO_BITMAP* img_muro = al_load_bitmap("muro.png");
    ALLEGRO_BITMAP* img_mario = al_load_bitmap("mario.png");

    if (!img_mario) {
        cerr << "Error: No se pudo cargar 'mario.png'." << endl;
        return -1;
    }

    // --- Memoria dinamica para escenarios y resultados ---
    // Aqui usamos new para crear arreglos dinamicos de escenarios y resultados
    int casos;
    ifstream archivo_entrada("entrada.txt");
    if (!archivo_entrada.is_open()) {
        cerr << "No se pudo abrir entrada.txt" << endl;
        return -1;
    }
    archivo_entrada >> casos;
    vector<int>* escenarios = new vector<int>[casos];
    Resultado* resultados = new Resultado[casos];

    // Creamos un archivo de salida unico usando la fecha y hora
    time_t now = time(0);
    tm* ltm = localtime(&now);
    stringstream nombre_salida;
    nombre_salida << "salida_"
        << 1900 + ltm->tm_year
        << (1 + ltm->tm_mon)
        << ltm->tm_mday << "_"
        << ltm->tm_hour
        << ltm->tm_min
        << ltm->tm_sec << ".txt";
    ofstream archivo_salida(nombre_salida.str());

    // Leemos los escenarios desde el archivo y calculamos los resultados
    for (int c = 0; c < casos; ++c) {
        int num_muros;
        archivo_entrada >> num_muros;
        escenarios[c].resize(num_muros);
        for (int i = 0; i < num_muros; ++i) archivo_entrada >> escenarios[c][i];
        resultados[c] = procesarEscenario(escenarios[c]);
        archivo_salida << resultados[c].saltos_arriba << " " << resultados[c].saltos_abajo << endl;
    }
    archivo_entrada.close();
    archivo_salida.close();

    // Creamos unas nubes para que el fondo se vea mas cool
    vector<Nube> nubes = {
        {100, 100, 0.5f},
        {300, 150, 0.3f},
        {600, 80, 0.4f},
        {400, 200, 0.2f}
    };

    // Definimos los botones de arriba: salir, pausa, avanzar y retroceder
    float by = 10;
    vector<Boton> botones = {
        {"Salir",      10,  by, 70, 28},
        {"Pausa",      90,  by, 80, 28},
        {"Avanzar",    180, by, 90, 28},
        {"Retroceder", 280, by, 110, 28}
    };

    // Variables para el juego: posicion de Mario, caso actual, etc
    size_t caso_actual = 0, posicion_muro = 0;
    float mario_x = 50 + 25, mario_y = alto - escenarios[0][0] * 20 - 35;
    float destino_x = mario_x, destino_y = mario_y;
    bool en_salto = false, salir = false, mario_pausa = false;
    ALLEGRO_EVENT ev;

    al_start_timer(timer);

    // Aqui empieza el bucle principal del juego
    while (!salir) {
        al_wait_for_event(queue, &ev);

        // Si cierran la ventana, salimos
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            salir = true;

        // Si hacen click en un boton, revisamos cual fue
        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            float mx = ev.mouse.x, my = ev.mouse.y;
            // Salir
            if (mouse_sobre(mx, my, botones[SALIR].x, botones[SALIR].y, botones[SALIR].w, botones[SALIR].h)) salir = true;
            // Pausa
            if (mouse_sobre(mx, my, botones[PAUSA].x, botones[PAUSA].y, botones[PAUSA].w, botones[PAUSA].h)) mario_pausa = !mario_pausa;
            // Avanzar muro
            if (mouse_sobre(mx, my, botones[AVANZAR].x, botones[AVANZAR].y, botones[AVANZAR].w, botones[AVANZAR].h)) {
                if (posicion_muro < escenarios[caso_actual].size()-1) {
                    posicion_muro++;
                    mario_x = 50 + posicion_muro * 80 + 25;
                    mario_y = alto - escenarios[caso_actual][posicion_muro] * 20 - 35;
                }
            }
            // Retroceder muro o caso
            if (mouse_sobre(mx, my, botones[RETROCEDER].x, botones[RETROCEDER].y, botones[RETROCEDER].w, botones[RETROCEDER].h)) {
                if (posicion_muro > 0) {
                    posicion_muro--;
                    mario_x = 50 + posicion_muro * 80 + 25;
                    mario_y = alto - escenarios[caso_actual][posicion_muro] * 20 - 35;
                }
                else if (caso_actual > 0) {
                    caso_actual--;
                    posicion_muro = escenarios[caso_actual].size() - 1;
                    mario_x = 50 + posicion_muro * 80 + 25;
                    mario_y = alto - escenarios[caso_actual][posicion_muro] * 20 - 35;
                }
            }
        }

        // Aqui se dibuja todo en pantalla cada frame
        if (ev.type == ALLEGRO_EVENT_TIMER) {
            al_clear_to_color(al_map_rgb(80, 180, 255));

            // Dibujamos las nubes moviendose
            for (auto& nube : nubes) {
                al_draw_filled_circle(nube.x, nube.y, 18, al_map_rgb(255,255,255));
                al_draw_filled_circle(nube.x+20, nube.y+5, 15, al_map_rgb(255,255,255));
                al_draw_filled_circle(nube.x-18, nube.y+8, 13, al_map_rgb(255,255,255));
                al_draw_filled_circle(nube.x+10, nube.y-10, 10, al_map_rgb(255,255,255));
                al_draw_circle(nube.x, nube.y, 18, al_map_rgb(220,220,220), 2);
                al_draw_circle(nube.x+20, nube.y+5, 15, al_map_rgb(220,220,220), 2);
                al_draw_circle(nube.x-18, nube.y+8, 13, al_map_rgb(220,220,220), 2);
                al_draw_circle(nube.x+10, nube.y-10, 10, al_map_rgb(220,220,220), 2);
                nube.x += nube.vel;
                if (nube.x > ancho+50) nube.x = -40;
            }

            // Dibujamos el suelo naranja
            dibujar_suelo(ancho, alto);

            // Dibujamos todos los muros del escenario actual
            const vector<int>& muros = escenarios[caso_actual];
            for (size_t i = 0; i < muros.size(); ++i) {
                float muro_x = 50 + i * 80;
                float muro_y = alto - muros[i] * 20;
                if (img_muro) {
                    al_draw_scaled_bitmap(img_muro, 0, 0, al_get_bitmap_width(img_muro), al_get_bitmap_height(img_muro),
                        muro_x, muro_y, 50, muros[i] * 20, 0);
                } else {
                    al_draw_filled_rectangle(muro_x, muro_y, muro_x + 50, alto, color_muro);
                }
            }

            // Dibujamos a Mario centrado en el muro actual
            al_draw_scaled_bitmap(img_mario, 0, 0, al_get_bitmap_width(img_mario), al_get_bitmap_height(img_mario),
                mario_x - 15, mario_y - 5, 30, 30, 0);

            // Mostramos el texto con los saltos y cuantos muros son altos (usando la funcion recursiva)
            stringstream ss;
            ss << "Caso " << (caso_actual + 1) << " | Arriba: " << resultados[caso_actual].saltos_arriba
                << " Abajo: " << resultados[caso_actual].saltos_abajo
                << " | Muros altos: " << contar_muros_mayores(escenarios[caso_actual], 0, 3);
            al_draw_text(font, color_texto, ancho / 2, 50, ALLEGRO_ALIGN_CENTER, ss.str().c_str());

            // Dibujamos los botones de arriba
            for (const auto& b : botones) {
                dibujar_boton(b.x, b.y, b.w, b.h, b.nombre.c_str(), font, al_map_rgb(0,0,0));
            }

            al_flip_display();

            // Movimiento automatico de Mario (solo si no esta en pausa)
            if (!mario_pausa && !en_salto && posicion_muro < muros.size() - 1) {
                destino_x = 50 + (posicion_muro + 1) * 80 + 25;
                destino_y = alto - muros[posicion_muro + 1] * 20 - 35;
                en_salto = true;
            }
            if (!mario_pausa && en_salto) {
                float dx = (destino_x - mario_x) * 0.1f;
                float dy = (destino_y - mario_y) * 0.1f;
                mario_x += dx;
                mario_y += dy;
                if (abs(destino_x - mario_x) < 1 && abs(destino_y - mario_y) < 1) {
                    mario_x = destino_x;
                    mario_y = destino_y;
                    en_salto = false;
                    posicion_muro++;
                }
            }
            // Si Mario termina el escenario, pasa al siguiente caso
            if (posicion_muro >= muros.size() - 1) {
                caso_actual++;
                if (caso_actual >= (size_t)casos) salir = true;
                else {
                    posicion_muro = 0;
                    mario_x = 50 + 25;
                    mario_y = alto - escenarios[caso_actual][0] * 20 - 35;
                }
            }
        }
    }

    // Al final, liberamos toda la memoria dinamica y destruimos los recursos de Allegro
    delete[] escenarios;
    delete[] resultados;
    al_destroy_font(font);
    al_destroy_display(display);
    al_destroy_event_queue(queue);
    al_destroy_timer(timer);
    if (img_muro) al_destroy_bitmap(img_muro);
    if (img_mario) al_destroy_bitmap(img_mario);

    return 0;
}
