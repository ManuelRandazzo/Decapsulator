#include "styles.h"
#include "images.h"
#include "fonts.h"

#include "ui.h"
#include "screens.h"

//
// Style: titoli schermata iniziale
//

void init_style_titoli_schermata_iniziale_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(0xffffff));
};

lv_style_t *get_style_titoli_schermata_iniziale_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_titoli_schermata_iniziale_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_titoli_schermata_iniziale(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_titoli_schermata_iniziale_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_titoli_schermata_iniziale(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_titoli_schermata_iniziale_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: Pulsante Rosso
//

void init_style_pulsante_rosso_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xb80505));
};

lv_style_t *get_style_pulsante_rosso_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_pulsante_rosso_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_pulsante_rosso(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_pulsante_rosso_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_pulsante_rosso(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_pulsante_rosso_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: puls jog
//

void init_style_puls_jog_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0x2196f3));
    lv_style_set_text_font(style, &lv_font_montserrat_16);
};

lv_style_t *get_style_puls_jog_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_puls_jog_MAIN_DEFAULT(style);
    }
    return style;
};

void init_style_puls_jog_MAIN_DISABLED(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0x777777));
    lv_style_set_text_font(style, &lv_font_montserrat_16);
};

lv_style_t *get_style_puls_jog_MAIN_DISABLED() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_puls_jog_MAIN_DISABLED(style);
    }
    return style;
};

void init_style_puls_jog_MAIN_PRESSED(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(0x000000));
    lv_style_set_bg_color(style, lv_color_hex(0xff8800));
    lv_style_set_text_font(style, &lv_font_montserrat_16);
};

lv_style_t *get_style_puls_jog_MAIN_PRESSED() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_puls_jog_MAIN_PRESSED(style);
    }
    return style;
};

void add_style_puls_jog(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_puls_jog_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_puls_jog_MAIN_DISABLED(), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_add_style(obj, get_style_puls_jog_MAIN_PRESSED(), LV_PART_MAIN | LV_STATE_PRESSED);
};

void remove_style_puls_jog(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_puls_jog_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_puls_jog_MAIN_DISABLED(), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_remove_style(obj, get_style_puls_jog_MAIN_PRESSED(), LV_PART_MAIN | LV_STATE_PRESSED);
};

//
// Style: puls rosso verde
//

void init_style_puls_rosso_verde_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0x00cd17));
    lv_style_set_text_font(style, &lv_font_montserrat_16);
};

lv_style_t *get_style_puls_rosso_verde_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_puls_rosso_verde_MAIN_DEFAULT(style);
    }
    return style;
};

void init_style_puls_rosso_verde_MAIN_CHECKED(lv_style_t *style) {
    lv_style_set_text_font(style, &lv_font_montserrat_16);
    lv_style_set_bg_color(style, lv_color_hex(0xf44336));
};

lv_style_t *get_style_puls_rosso_verde_MAIN_CHECKED() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_puls_rosso_verde_MAIN_CHECKED(style);
    }
    return style;
};

void add_style_puls_rosso_verde(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_puls_rosso_verde_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_puls_rosso_verde_MAIN_CHECKED(), LV_PART_MAIN | LV_STATE_CHECKED);
};

void remove_style_puls_rosso_verde(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_puls_rosso_verde_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_puls_rosso_verde_MAIN_CHECKED(), LV_PART_MAIN | LV_STATE_CHECKED);
};

//
// Style: Stato Sensori
//

void init_style_stato_sensori_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xdc143c));
    lv_style_set_border_width(style, 0);
    lv_style_set_radius(style, 2);
};

lv_style_t *get_style_stato_sensori_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_stato_sensori_MAIN_DEFAULT(style);
    }
    return style;
};

void init_style_stato_sensori_MAIN_CHECKED(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0x7fffd4));
};

lv_style_t *get_style_stato_sensori_MAIN_CHECKED() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_stato_sensori_MAIN_CHECKED(style);
    }
    return style;
};

void add_style_stato_sensori(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_stato_sensori_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_stato_sensori_MAIN_CHECKED(), LV_PART_MAIN | LV_STATE_CHECKED);
};

void remove_style_stato_sensori(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_stato_sensori_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_stato_sensori_MAIN_CHECKED(), LV_PART_MAIN | LV_STATE_CHECKED);
};

//
// Style: giallo
//

void init_style_giallo_MAIN_CHECKED(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xe7f321));
    lv_style_set_text_color(style, lv_color_hex(0x000000));
};

lv_style_t *get_style_giallo_MAIN_CHECKED() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_giallo_MAIN_CHECKED(style);
    }
    return style;
};

void add_style_giallo(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_giallo_MAIN_CHECKED(), LV_PART_MAIN | LV_STATE_CHECKED);
};

void remove_style_giallo(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_giallo_MAIN_CHECKED(), LV_PART_MAIN | LV_STATE_CHECKED);
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_titoli_schermata_iniziale,
        add_style_pulsante_rosso,
        add_style_puls_jog,
        add_style_puls_rosso_verde,
        add_style_stato_sensori,
        add_style_giallo,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_titoli_schermata_iniziale,
        remove_style_pulsante_rosso,
        remove_style_puls_jog,
        remove_style_puls_rosso_verde,
        remove_style_stato_sensori,
        remove_style_giallo,
    };
    remove_style_funcs[styleIndex](obj);
}