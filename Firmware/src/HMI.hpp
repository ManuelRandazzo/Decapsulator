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


/// Arduino + LVGL 9.5 + TFT_eSPI
#include "HMI_UI_EEZ/ui.h"
/// Include il file per il Logger
#include "Debug.hpp"
/// Include il file per gli handler delle tasks
#include "tasks_cfg.hpp"
/// Include il file che contiene gli oggetti degli IO del decapsulator
#include "decapsulator_io.hpp"


extern void prgHMITask(void* pvParameters);



#ifdef __cplusplus
extern "C" {
#endif

/// Libreria per la creazione della Human Machine Interface (HMI) 
/// @link_per_il_setup_della_libreria: https://docs.lvgl.io/master/details/integration/frameworks/platformio.html
#include "lvgl.h"

// UI INIT
void ui_init(void);
void ui_tick(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif