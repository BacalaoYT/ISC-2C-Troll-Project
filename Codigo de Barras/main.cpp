#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <iostream>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <cmath>

using namespace std;

// Definimos los tipos de codigos de barras que vamos a manejar
enum TipoCodigo { EAN8, EAN13, DESCONOCIDO };

// Estructura para almacenar la informacion de cada codigo de barras
struct InfoCodigo {
    char original[32];
    char procesado[16];
    TipoCodigo tipo;
    char resultado[4];
    char pais[32];
};

// Declaramos las funciones que usamos en el codigo
int cuentaDigitos(const char* codigo);
void obtenerPais(const char* codigo, char* pais);
int calcularControl(const char* codigo, int longitud);
void completarCeros(const char* codigo, int longitud, char* resultado);
InfoCodigo* crearInfoCodigo(const char* linea);
void liberarCodigos(InfoCodigo** codigos, int cantidad);
void leerCodigosArchivo(const char* archivoEntrada, InfoCodigo**& codigos, int& cantidad);
void guardarResultadosArchivo(const char* archivoSalida, InfoCodigo** codigos, int cantidad);
void mostrarPantalla(InfoCodigo** codigos, int cantidad);
void dibujarCodigoBarras(const char* codigo, float x, float y, float ancho, float alto, ALLEGRO_COLOR colorBase, ALLEGRO_FONT* fuente);
void dibujarMarco(float x, float y, float ancho, float alto, ALLEGRO_COLOR color);

// MAIN
int main() {
    InfoCodigo** codigos = nullptr;
    int cantidad = 0;
    leerCodigosArchivo("entrada.txt", codigos, cantidad);

    char archivoSalida[64];
    time_t now = time(0);
    tm* ltm = localtime(&now);
    strftime(archivoSalida, 64, "salida_%Y%m%d_%H%M%S.txt", ltm);
    guardarResultadosArchivo(archivoSalida, codigos, cantidad);

    mostrarPantalla(codigos, cantidad);

    liberarCodigos(codigos, cantidad);
    return 0;
}

// FUNCIONES

// Cuenta la cantidad de digitos en un codigo de barras
int cuentaDigitos(const char* codigo) {
    int n = 0;
    while (codigo[n] != '\0') n++;
    return n;
}

// Analizando los primeros 3 digitos del codigo, determina el pais de origen
void obtenerPais(const char* codigo, char* pais) {
    if (cuentaDigitos(codigo) < 3) {
        strcpy(pais, "Desconocido");
        return;
    }
    char prefijoStr[4] = {0};
    for (int i = 0; i < 3; ++i) prefijoStr[i] = codigo[i];
    int prefijo = atoi(prefijoStr);
    if (prefijo == 380) strcpy(pais, "Bulgaria");
    else if (prefijo == 539) strcpy(pais, "Irlanda");
    else if (prefijo == 560) strcpy(pais, "Portugal");
    else if (prefijo == 759) strcpy(pais, "Venezuela");
    else if (prefijo == 850) strcpy(pais, "Cuba");
    else if (prefijo == 890) strcpy(pais, "India");
    else if (prefijo >= 500 && prefijo <= 509) strcpy(pais, "Inglaterra");
    else if (prefijo >= 700 && prefijo <= 709) strcpy(pais, "Noruega");
    else if (prefijo >= 0 && prefijo <= 19) strcpy(pais, "EEUU");
    else strcpy(pais, "Desconocido");
}

// Esta funcion calcula el digito de control de un codigo EAN-8 o EAN-13

int calcularControl(const char* codigo, int longitud) {
    int suma = 0;
    for (int i = longitud - 2, pos = 1; i >= 0; --i, ++pos) {
        int digito = codigo[i] - '0';
        if (pos % 2 == 1) suma += digito * 3;
        else suma += digito * 1;
    }
    int resto = suma % 10;
    if (resto == 0) return 0;
    return 10 - resto;
}

void completarCeros(const char* codigo, int longitud, char* resultado) {
    int len = cuentaDigitos(codigo);
    int ceros = longitud - len;
    for (int i = 0; i < ceros; ++i) resultado[i] = '0';
    for (int i = 0; i < len; ++i) resultado[ceros + i] = codigo[i];
    resultado[longitud] = '\0';
}

InfoCodigo* crearInfoCodigo(const char* linea) {
    InfoCodigo* info = new InfoCodigo;
    int digitos = cuentaDigitos(linea);
    for (int i = 0; i < digitos + 1; ++i) info->original[i] = linea[i];
    if (digitos <= 8) {
        completarCeros(linea, 8, info->procesado);
        info->tipo = EAN8;
    } else {
        completarCeros(linea, 13, info->procesado);
        info->tipo = EAN13;
    }
    int longitud = (info->tipo == EAN8) ? 8 : 13;
    int controlEsperado = calcularControl(info->procesado, longitud);
    int controlReal = info->procesado[longitud - 1] - '0';
    if (controlEsperado == controlReal) {
        strcpy(info->resultado, "SI");
        if (info->tipo == EAN13) obtenerPais(info->procesado, info->pais);
        else info->pais[0] = '\0';
    } else {
        strcpy(info->resultado, "NO");
        if (info->tipo == EAN13) obtenerPais(info->procesado, info->pais);
        else info->pais[0] = '\0';
    }
    return info;
}

void liberarCodigos(InfoCodigo** codigos, int cantidad) {
    for (int i = 0; i < cantidad; ++i) delete codigos[i];
    delete[] codigos;
}

void leerCodigosArchivo(const char* archivoEntrada, InfoCodigo**& codigos, int& cantidad) {
    FILE* entrada = fopen(archivoEntrada, "r");
    if (!entrada) {
        codigos = nullptr;
        cantidad = 0;
        return;
    }
    char linea[32];
    int capacidad = 16;
    cantidad = 0;
    codigos = new InfoCodigo*[capacidad];
    while (fgets(linea, sizeof(linea), entrada)) {
        int len = 0;
        for (int i = 0; linea[i] != '\0'; ++i) {
            if (linea[i] != ' ' && linea[i] != '\n' && linea[i] != '\r')
                linea[len++] = linea[i];
        }
        linea[len] = '\0';
        if (len == 0 || (len == 1 && linea[0] == '0')) break;
        if (cantidad >= capacidad) {
            capacidad *= 2;
            InfoCodigo** temp = new InfoCodigo*[capacidad];
            for (int k = 0; k < cantidad; ++k) temp[k] = codigos[k];
            delete[] codigos;
            codigos = temp;
        }
        codigos[cantidad++] = crearInfoCodigo(linea);
    }
    fclose(entrada);
}

void guardarResultadosArchivo(const char* archivoSalida, InfoCodigo** codigos, int cantidad) {
    FILE* salida = fopen(archivoSalida, "w");
    if (!salida) return;
    for (int i = 0; i < cantidad; ++i) {
        InfoCodigo* info = codigos[i];
        if (info->tipo == EAN13 && cuentaDigitos(info->pais) > 0)
            fprintf(salida, "%s %s\n", info->resultado, info->pais);
        else
            fprintf(salida, "%s Desconocido\n", info->resultado);
    }
    fclose(salida);
}

void mostrarPantalla(InfoCodigo** codigos, int cantidad) {
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

    int actual = 0;
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
            } else if (evento.keyboard.keycode == ALLEGRO_KEY_RIGHT && actual < cantidad - 1) {
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

        if (cantidad == 0) {
            al_draw_text(fuenteGrande, colorTexto, anchoPantalla/2, altoPantalla/2, ALLEGRO_ALIGN_CENTRE, "No hay codigos para mostrar");
        } else {
            InfoCodigo* info = codigos[actual];
            const char* tipoCodigo = (info->tipo == EAN8) ? "EAN-8" : "EAN-13";
            al_draw_textf(fuente, colorTexto, anchoPantalla/2, margen + al_get_font_line_height(fuenteTitulo) + 20, ALLEGRO_ALIGN_CENTRE, 
                         "Codigo: %s (%s)", info->original, tipoCodigo);

            float barraX = margen + 50;
            float barraY = margen + al_get_font_line_height(fuenteTitulo) + 80;
            float barraAncho = anchoPantalla - 2*margen - 100;
            float barraAlto = altoPantalla * 0.25f;
            dibujarCodigoBarras(info->procesado, barraX, barraY, barraAncho, barraAlto, colorPrimario, fuente);

            bool esValido = (info->resultado[0] == 'S' && info->resultado[1] == 'I');
            ALLEGRO_COLOR colorResultado = esValido ? colorExito : colorError;
            
            al_draw_text(fuenteGrande, al_map_rgb(0, 0, 0), anchoPantalla/2 + 2, barraY + barraAlto + 62, ALLEGRO_ALIGN_CENTRE, info->resultado);
            al_draw_text(fuenteGrande, colorResultado, anchoPantalla/2, barraY + barraAlto + 60, ALLEGRO_ALIGN_CENTRE, info->resultado);

            if (info->tipo == EAN13 && esValido && cuentaDigitos(info->pais) > 0) {
                al_draw_textf(fuente, colorTexto, anchoPantalla/2, 
                             barraY + barraAlto + al_get_font_line_height(fuenteGrande) + 80,
                             ALLEGRO_ALIGN_CENTRE, "Pais: %s", info->pais);
            }

            al_draw_textf(fuente, colorTexto, anchoPantalla/2, altoPantalla - margen - al_get_font_line_height(fuente)*2, ALLEGRO_ALIGN_CENTRE, 
                         "← → para cambiar | %d/%d | ESC para salir", actual+1, cantidad);
        }

        al_flip_display();
    }

    if (fuente) al_destroy_font(fuente);
    if (fuenteGrande) al_destroy_font(fuenteGrande);
    if (fuenteTitulo) al_destroy_font(fuenteTitulo);
    if (pantalla) al_destroy_display(pantalla);
    if (eventos) al_destroy_event_queue(eventos);
}

void dibujarCodigoBarras(const char* codigo, float x, float y, float ancho, float alto, ALLEGRO_COLOR colorBase, ALLEGRO_FONT* fuente) {
    float espacio = 2.0f;
    int longitud = cuentaDigitos(codigo);
    float anchoBarra = (ancho / longitud) - espacio;
    float profundidad = 5.0f;
    for (int i = 0; i < longitud; ++i) {
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
