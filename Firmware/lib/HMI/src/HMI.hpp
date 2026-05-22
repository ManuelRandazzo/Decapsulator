#pragma once
/**
 * This file is part of {{ Decapsulator }}.
 *
 * Developed for the ITS Meccatronico Montebelluna-Castelfranco Veneto 2024-2026 Gruppo 1.
 * This product includes software developed by the Gruppo 1 Project.
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This program is NOT free software: you can NOT redistribute it and/or modify it.
 *
 * The Documentation is made following the Doxygen standard documentation style
 * 
 * @file (scrivi cosa fa sto file)
 * @author Alessio Pisan
 * @attention il codice è stato generato automaticamente grazie al software "SquareLine Studio"
 *
 */


// Arduino + LVGL 9.3 + TFT_eSPI

/// Libreria per la creazione della Human Machine Interface (HMI) 
/// @link_per_il_setup_della_libreria: https://docs.lvgl.io/master/details/integration/frameworks/platformio.html
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <ui.h>
#include "Debug.hpp"
/// Include il file per gli handler delle tasks
#include "tasks_cfg.hpp"
/// Include il file che contiene gli oggetti degli IO del decapsulator
#include "decapsulator_io.hpp"
/// Include il file per gli handler delle tasks
#include "ui_and_backend_cfg.hpp"


extern void prgHMITask(void* pvParameters);



#ifdef __cplusplus
extern "C" {
#endif

#if defined __has_include
#if __has_include("lvgl.h")
#include "lvgl.h"
#elif __has_include("lvgl/lvgl.h")
#include "lvgl/lvgl.h"
#else
#include "lvgl.h"
#endif
#else
#include "lvgl.h"
#endif

#include "ui_helpers.h"
#include "ui_events.h"
#include "ui_theme_manager.h"
#include "ui_themes.h"

///////////////////// SCREENS ////////////////////

#include "ui_Scheramata_Avvio.h"
#include "ui_Schermata_Principale.h"
#include "ui_Schermata_Diagnostica.h"
#include "ui_password.h"
#include "ui_EASTER_EGG.h"
#include "ui_Logger1.h"
#include "ui_JOGGER.h"

///////////////////// VARIABLES ////////////////////

extern lv_anim_t * glow1_Animation(lv_obj_t * TargetObject, int delay);

// EVENTS

extern lv_obj_t * ui____initial_actions0;

// IMAGES AND IMAGE SETS
LV_IMG_DECLARE(ui_img_1415156368);    // assets/ITSMeccatroniconew2-grigio (1).png
LV_IMG_DECLARE(ui_img_foto_sfondo_resized_png);    // assets/foto sfondo_resized.png
LV_IMG_DECLARE(ui_img_1414324677);    // assets/pngegg (1).png
LV_IMG_DECLARE(ui_img_logo_decapsulator_resized_png);    // assets/logo_decapsulator_resized.png

// UI INIT
void ui_init(void);
void ui_destroy(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif