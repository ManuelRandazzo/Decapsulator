#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: titoli schermata iniziale
lv_style_t *get_style_titoli_schermata_iniziale_MAIN_DEFAULT();
void add_style_titoli_schermata_iniziale(lv_obj_t *obj);
void remove_style_titoli_schermata_iniziale(lv_obj_t *obj);

// Style: Pulsante Rosso
lv_style_t *get_style_pulsante_rosso_MAIN_DEFAULT();
void add_style_pulsante_rosso(lv_obj_t *obj);
void remove_style_pulsante_rosso(lv_obj_t *obj);

// Style: puls jog
lv_style_t *get_style_puls_jog_MAIN_DEFAULT();
lv_style_t *get_style_puls_jog_MAIN_DISABLED();
lv_style_t *get_style_puls_jog_MAIN_PRESSED();
void add_style_puls_jog(lv_obj_t *obj);
void remove_style_puls_jog(lv_obj_t *obj);

// Style: puls rosso verde
lv_style_t *get_style_puls_rosso_verde_MAIN_DEFAULT();
lv_style_t *get_style_puls_rosso_verde_MAIN_CHECKED();
void add_style_puls_rosso_verde(lv_obj_t *obj);
void remove_style_puls_rosso_verde(lv_obj_t *obj);

// Style: Stato Sensori
lv_style_t *get_style_stato_sensori_MAIN_DEFAULT();
lv_style_t *get_style_stato_sensori_MAIN_CHECKED();
void add_style_stato_sensori(lv_obj_t *obj);
void remove_style_stato_sensori(lv_obj_t *obj);

// Style: giallo
lv_style_t *get_style_giallo_MAIN_CHECKED();
void add_style_giallo(lv_obj_t *obj);
void remove_style_giallo(lv_obj_t *obj);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/