#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime>

using namespace std;

// Enum para identificar el tipo de codigo de barras
enum TipoCodigo { EAN8, EAN13, DESCONOCIDO };

// Estructura para guardar la informacion de cada codigo procesado
struct InfoCodigo {
    string original;
    string procesado;
    TipoCodigo tipo;
    string resultado;
    string pais;
};

// Funcion recursiva para contar la cantidad de digitos en un string
int cuentaDigitos(const string& codigo, int idx = 0) {
    if (idx >= codigo.size()) return 0;
    return 1 + cuentaDigitos(codigo, idx + 1);
}

// Determina el pais de origen del producto a partir del prefijo del codigo de barras
string obtenerPais(const string& codigo) {
    if (codigo.length() < 3) return "Desconocido";
    int prefijo = stoi(codigo.substr(0, 3));
    if (prefijo == 380) return "Bulgaria";
    if (prefijo == 539) return "Irlanda";
    if (prefijo == 560) return "Portugal";
    if (prefijo == 759) return "Venezuela";
    if (prefijo == 850) return "Cuba";
    if (prefijo == 890) return "India";
    if (prefijo >= 500 && prefijo <= 509) return "Inglaterra";
    if (prefijo >= 700 && prefijo <= 709) return "Noruega";
    if (prefijo >= 0 && prefijo <= 19) return "EEUU";
    return "Desconocido";
}

// Calcula el digito de control de un codigo EAN-8 o EAN-13 segun el estandar EAN
int calcularControl(const string& codigo, int longitud) {
    int suma = 0;
    for (int i = longitud - 2, pos = 1; i >= 0; --i, ++pos) {
        int digito = codigo[i] - '0';
        if (pos % 2 == 1) {
            suma += digito * 3;
        } else {
            suma += digito * 1;
        }
    }
    int resto = suma % 10;
    if (resto == 0) return 0;
    return 10 - resto;
}

// Agrega ceros a la izquierda del codigo si su longitud es menor a la requerida (8 o 13)
string completarCeros(const string& codigo, int longitud) {
    string resultado = codigo;
    while (resultado.length() < longitud) resultado = "0" + resultado;
    return resultado;
}

// Modifica la funcion para que el archivo de salida sea unico cada vez
void procesaCodigos(vector<InfoCodigo*>& codigos, const string& archivoEntrada, string archivoSalida) {
    // Genera un nombre unico usando la fecha y hora actual
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "salida_%Y%m%d_%H%M%S.txt", ltm);
    archivoSalida = buffer;

    ifstream entrada(archivoEntrada);
    ofstream salida(archivoSalida);
    string linea;
    while (getline(entrada, linea)) {
        linea.erase(remove(linea.begin(), linea.end(), ' '), linea.end());
        if (linea == "0") break;
        if (linea.empty()) continue;

        InfoCodigo* info = new InfoCodigo;
        info->original = linea;
        int digitos = cuentaDigitos(linea);
        if (digitos <= 8) {
            info->procesado = completarCeros(linea, 8);
            info->tipo = EAN8;
        } else {
            info->procesado = completarCeros(linea, 13);
            info->tipo = EAN13;
        }
        int controlEsperado = calcularControl(info->procesado, info->tipo == EAN8 ? 8 : 13);
        int controlReal = info->procesado[(info->tipo == EAN8 ? 8 : 13) - 1] - '0';
        if (controlEsperado == controlReal) {
            info->resultado = "SI";
            info->pais = (info->tipo == EAN13) ? obtenerPais(info->procesado) : "";
        } else {
            info->resultado = "NO";
            info->pais = (info->tipo == EAN13) ? obtenerPais(info->procesado) : "";
        }
        // Guarda el resultado y el pais (o desconocido) en el archivo de salida
        salida << info->resultado << " " << (info->pais.empty() ? "Desconocido" : info->pais) << endl;
        codigos.push_back(info);
    }
    entrada.close();
    salida.close();
}

// Dibuja el codigo de barras en pantalla usando colores y efectos visuales
void dibujarCodigoBarras(const string& codigo, float x, float y, float ancho, float alto, ALLEGRO_COLOR colorBase, ALLEGRO_FONT* fuente) {
    float espacio = 2.0f;
    float anchoBarra = (ancho / codigo.length()) - espacio;
    float profundidad = 5.0f;
    for (size_t i = 0; i < codigo.length(); ++i) {
        int valor = codigo[i] - '0';
        float altura = alto * (0.4f + 0.06f * valor);
        float posX = x + i * (anchoBarra + espacio);
        float posY = y + (alto - altura);
        float h = valor * 36.0f;
        float s = 0.4f;
        float v = 0.9f;
        float c = v * s;
        float xh = h / 60.0f;
        float xColor = c * (1 - fabs(fmod(xh, 2) - 1));
        float m = v - c;
        float r1, g1, b1;
        if (h < 60)      { r1 = c; g1 = xColor; b1 = 0; }
        else if (h < 120){ r1 = xColor; g1 = c; b1 = 0; }
        else if (h < 180){ r1 = 0; g1 = c; b1 = xColor; }
        else if (h < 240){ r1 = 0; g1 = xColor; b1 = c; }
        else if (h < 300){ r1 = xColor; g1 = 0; b1 = c; }
        else             { r1 = c; g1 = 0; b1 = xColor; }
        ALLEGRO_COLOR colorPastel = al_map_rgb_f(r1 + m, g1 + m, b1 + m);
        al_draw_filled_rectangle(posX, posY, posX + anchoBarra, posY + altura, colorPastel);
        ALLEGRO_COLOR sombraLateral = al_map_rgba(0, 0, 0, 80);
        al_draw_filled_rectangle(posX + anchoBarra, posY, posX + anchoBarra + profundidad, posY + altura, sombraLateral);
        ALLEGRO_COLOR sombraInferior = al_map_rgba(0, 0, 0, 120);
        al_draw_filled_rectangle(posX, posY + altura, posX + anchoBarra + profundidad, posY + altura + profundidad, sombraInferior);
        al_draw_textf(fuente, al_map_rgb(0, 0, 0), posX + anchoBarra/2, posY + altura/2 - 8, ALLEGRO_ALIGN_CENTRE, "%d", valor);
    }
}

// Dibuja un marco decorativo alrededor del area principal de la pantalla
void dibujarMarco(float x, float y, float ancho, float alto, ALLEGRO_COLOR color) {
    al_draw_rectangle(x, y, x + ancho, y + alto, color, 4.0f);
    float tamanoEsquina = 20.0f;
    al_draw_line(x, y, x + tamanoEsquina, y, color, 3.0f);
    al_draw_line(x, y, x, y + tamanoEsquina, color, 3.0f);
    al_draw_line(x + ancho, y, x + ancho - tamanoEsquina, y, color, 3.0f);
    al_draw_line(x + ancho, y, x + ancho, y + tamanoEsquina, color, 3.0f);
    al_draw_line(x, y + alto, x + tamanoEsquina, y + alto, color, 3.0f);
    al_draw_line(x, y + alto, x, y + alto - tamanoEsquina, color, 3.0f);
    al_draw_line(x + ancho, y + alto, x + ancho - tamanoEsquina, y + alto, color, 3.0f);
    al_draw_line(x + ancho, y + alto, x + ancho, y + alto - tamanoEsquina, color, 3.0f);
}

int main() {
    al_init();
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_primitives_addon();
    al_init_image_addon();
    al_install_keyboard();

    ALLEGRO_MONITOR_INFO info;
    al_get_monitor_info(0, &info);
    int anchoPantalla = info.x2 - info.x1;
    int altoPantalla = info.y2 - info.y1;

    ALLEGRO_DISPLAY* pantalla = al_create_display(anchoPantalla, altoPantalla);
    al_set_window_title(pantalla, "Verificador de Codigos de Barras EAN");

    ALLEGRO_FONT* fuente = al_load_ttf_font("arial.ttf", anchoPantalla/50, 0);
    if (!fuente) fuente = al_create_builtin_font();
    ALLEGRO_FONT* fuenteGrande = al_load_ttf_font("arial.ttf", anchoPantalla/28, 0);
    if (!fuenteGrande) fuenteGrande = al_create_builtin_font();
    ALLEGRO_FONT* fuenteTitulo = al_load_ttf_font("arial.ttf", anchoPantalla/18, 0);
    if (!fuenteTitulo) fuenteTitulo = al_create_builtin_font();

    ALLEGRO_COLOR colorFondo = al_map_rgb(230, 240, 255);
    ALLEGRO_COLOR colorTexto = al_map_rgb(30, 30, 50);
    ALLEGRO_COLOR colorPrimario = al_map_rgb(100, 150, 255);
    ALLEGRO_COLOR colorExito = al_map_rgb(100, 255, 150);
    ALLEGRO_COLOR colorError = al_map_rgb(255, 100, 100);

    vector<InfoCodigo*> codigos;
    procesaCodigos(codigos, "entrada.txt", "salida.txt");

    size_t actual = 0;
    bool ejecutando = true;
    ALLEGRO_EVENT_QUEUE* eventos = al_create_event_queue();
    al_register_event_source(eventos, al_get_display_event_source(pantalla));
    al_register_event_source(eventos, al_get_keyboard_event_source());

    while (ejecutando) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(eventos, &evento);

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            ejecutando = false;
        } else if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                ejecutando = false;
            } else if (evento.keyboard.keycode == ALLEGRO_KEY_RIGHT && actual < codigos.size() - 1) {
                actual++;
            } else if (evento.keyboard.keycode == ALLEGRO_KEY_LEFT && actual > 0) {
                actual--;
            }
        }

        al_clear_to_color(colorFondo);

        float margen = anchoPantalla * 0.05f;
        dibujarMarco(margen, margen, anchoPantalla - 2*margen, altoPantalla - 2*margen, colorPrimario);

        al_draw_text(fuenteTitulo, al_map_rgb(0, 0, 0), anchoPantalla/2 + 2, margen + 2, ALLEGRO_ALIGN_CENTRE, "VERIFICADOR EAN");
        al_draw_text(fuenteTitulo, colorPrimario, anchoPantalla/2, margen, ALLEGRO_ALIGN_CENTRE, "VERIFICADOR EAN");

        if (codigos.empty()) {
            al_draw_text(fuenteGrande, colorTexto, anchoPantalla/2, altoPantalla/2, ALLEGRO_ALIGN_CENTRE, "No hay codigos para mostrar");
        } else {
            InfoCodigo* info = codigos[actual];
            string tipoCodigo = (info->tipo == EAN8) ? "EAN-8" : "EAN-13";
            al_draw_textf(fuente, colorTexto, anchoPantalla/2, margen + al_get_font_line_height(fuenteTitulo) + 20, ALLEGRO_ALIGN_CENTRE, 
                         "Codigo: %s (%s)", info->original.c_str(), tipoCodigo.c_str());

            float barraX = margen + 50;
            float barraY = margen + al_get_font_line_height(fuenteTitulo) + 80;
            float barraAncho = anchoPantalla - 2*margen - 100;
            float barraAlto = altoPantalla * 0.25f;
            dibujarCodigoBarras(info->procesado, barraX, barraY, barraAncho, barraAlto, colorPrimario, fuente);

            bool esValido = info->resultado == "SI";
            ALLEGRO_COLOR colorResultado = esValido ? colorExito : colorError;
            
            al_draw_text(fuenteGrande, al_map_rgb(0, 0, 0), anchoPantalla/2 + 2, barraY + barraAlto + 62, ALLEGRO_ALIGN_CENTRE, info->resultado.c_str());
            al_draw_text(fuenteGrande, colorResultado, anchoPantalla/2, barraY + barraAlto + 60, ALLEGRO_ALIGN_CENTRE, info->resultado.c_str());

            if (info->tipo == EAN13 && esValido && !info->pais.empty()) {
                al_draw_textf(fuente, colorTexto, anchoPantalla/2, 
                             barraY + barraAlto + al_get_font_line_height(fuenteGrande) + 80,
                             ALLEGRO_ALIGN_CENTRE, "Pais: %s", info->pais.c_str());
            }

            al_draw_textf(fuente, colorTexto, anchoPantalla/2, altoPantalla - margen - al_get_font_line_height(fuente)*2, ALLEGRO_ALIGN_CENTRE, 
                         "← → para cambiar | %d/%d | ESC para salir", int(actual+1), int(codigos.size()));
        }

        al_flip_display();
    }

    for (auto ptr : codigos) delete ptr;
    if (fuente) al_destroy_font(fuente);
    if (fuenteGrande) al_destroy_font(fuenteGrande);
    if (fuenteTitulo) al_destroy_font(fuenteTitulo);
    if (pantalla) al_destroy_display(pantalla);
    if (eventos) al_destroy_event_queue(eventos);

    return 0;
}