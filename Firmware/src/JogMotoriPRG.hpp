#pragma once

// -*- Gruppo 1-FreeRTOS-C++

/*
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
 */

/// Inlcude il file per la gestione del debugger
#include "Debug.hpp"
/// Include il file per gli handler delle tasks
#include "tasks_cfg.hpp"
/// Include il file che contiene gli oggetti degli IO del decapsulator
#include "decapsulator_io.hpp"
/// Include il file per gli handler delle tasks
#include "ui_and_backend_cfg.hpp"
/// Include la libreria custom del motion
#include "MotionControl.hpp"
/// Include la libreria per la gestione dei pin e del debounce
#include "DebouncePinHandler.hpp"
/// Include il programma principale e alcune funzioni già dichiarate nel file
#include "DecapsulatorPRG.hpp"




#pragma region (EXTERNS)

/// @brief PROGRAMMA JOG DEI MOTORI
extern void prgJogMotoriTask(void *pvParameters);

#pragma endregion (EXTERNS)