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


/// Libreria per la creazione della Human Machine Interface (HMI) 
/// @link_per_il_setup_della_libreria: https://docs.lvgl.io/master/details/integration/frameworks/platformio.html
//#include "lvgl.h" 
#ifdef LV_USE_TFT_ESPI
  #include <TFT_eSPI.h>
#endif