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
 * @file Questo file contiene gli extern degli handler e le configurazioni di tutte le tasks
 * 
 * @author Manuel Randazzo
 */
#include "FreeRTOS.h"


#pragma region (TASKS HANDLERS)

/// @brief Handler della task del programma principale (MAIN)
extern TaskHandle_t MainPrgHandler;

/// @brief Handler della task del programma della Human Machine Interface (HMI)
extern TaskHandle_t HMIPrgHandler;

/// @brief Handler della task del programma di pilotaggio manuale dei motori (JOG)
extern TaskHandle_t JogMotoriPrgHandler;

#pragma endregion (TASKS HANDLERS)













#pragma region (TASKS ENUMS)

/**
 *  @brief enum dell'Heap occupato dalle singole tasks (RAM del runtime)
 *
 *  @note heap occupato realmente --> realHeap = taskHeapSize * 4   (ex. 2048 * 4 = 8196 = 8KB)
 *  @note cercare di non occupare troppo spazio dell'Heap. Utilizzare esp_get_free_heap_size() 
 *        per sapere l'heap rimanente e uxTaskGetStackHighWaterMark(NULL) per sapere quello della task in cui viene chiamato. 
 *        MAX 512KB
 *
 *  @attention source stack_overflow: "Debug exception reason: Stack canary watchpoint triggered (task X) " nella console indica uno stack overflow della task X, aumentare lo stack
 *
 *  
 *  @determinazione_heap:
 *                        1. Aggiungi una funziona che restituisce l'Heap rimanente.
 *
 *                        2. Esegui la task in condizioni realistiche.
 *
 *                        3. Monitora quanta heap rimane con la funzione messa prima.
 *
 *                        4. Aggiungi un margine di sicurezza +20~50%.
 * 
 */
enum taskHeapSize_t
{
    OTA_heap        = 2048, // 2kiB
    HMI_heap        = 4096, // 4kiB
    JogMotori_heap  = 4096, // 4kiB 
    MainPrg_heap    = 6656, // 6.5kiB
};

/**
 *  @brief enum delle priorità delle singole tasks
 *
 *  @note più alto è il valore più la priorità è alta
 *  @note cercare di non fare troppe task con la stessa priorità (anche se vengono ugualmente eseguite)
 */
enum taskPriority_t
{
    OTA_priority        = 10,
    HMI_priority        = 5,
    JogMotori_priority  = 7,
    MainPrg_priority    = 7,
};

/**
 *  @brief enum dei delay delle singole tasks
 */
enum taskDelays_t
{
    MainPrg_delay    = 5,
    JogMotori_delay  = 5,
    HMI_delay        = 100,
    OTA_delay        = 200,
};

#pragma endregion (TASKS ENUMS)