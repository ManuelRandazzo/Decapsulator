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
/// Include una classe per rilevare i Rising Trigger
#include "R_TRIG.hpp"
/// Include una classe per rilevare i Falling Trigger
#include "F_TRIG.hpp"

/// @brief PROGRAMMA PRINCIPALE
extern void prgDecapsulatorTask(void *pvParameters);

typedef enum __sequence__ : uint8_t
{
    EMERGENCY_STATE,
    CONTAINER_FULL,
    TIMEOUT_STATE,
    MACHINE_STARTUP_STATE,
    PUNZONE_STARTUP_STATE,
    TAMBURO_STARTUP_STATE,
    QUIETE_STATE,
    SERVO_LOADER_OPEN_STATE,
    SERVO_LOADER_CLOSE_STATE,
    REACH_NEXT_STATION_STATE,
    PUNCHER_DOWN_FAST_STATE,
    PUNCHER_DOWN_SLOW_STATE,
    PUNCHER_UP_FAST_STATE,
} Sequence_t;

/// Gestione della sequenza del movimento del Decapsulator
extern Sequence_t sequenza;
