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
 * @file Questo file contiene le definizioni degli handler
 * 
 * @author Manuel Randazzo
 */

#include "tasks_cfg.hpp"


#pragma region (TASKS HANDLERS)

/// @brief Handler della task del programma principale (MAIN)
TaskHandle_t MainPrgHandler;

/// @brief Handler della task del programma della Human Machine Interface (HMI)
TaskHandle_t HMIPrgHandler;

/// @brief Handler della task del programma di pilotaggio manuale dei motori (JOG)
TaskHandle_t JogMotoriPrgHandler;

/// @brief Handler della task del logger in Seriale, MQTT e SD
TaskHandle_t LoggerHandler;

/// @brief Handler della task dell'autokill
TaskHandle_t AutokillHandler;

#pragma endregion (TASKS HANDLERS)