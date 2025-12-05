#pragma once
/*
/// Include la libreria custom per la gestione delle task
#include "Tasks.hpp"
/// Include la classe di gestione dell'HX711 la quale è ottimizzata per FreeRTOS
#include "HX711.h" /// https://github.com/bogde/HX711/blob/master
/// Oggetto HX711
HX711 Balance;

/// ATTENZIONE: verificare l'utilità di queste task
TaskTypeDef Contenitori;

/// USER:
#define LOADCELL_DOUT_PIN  3
#define LOADCELL_SCK_PIN  2
/// Quanti cicli di lettura fa per fare una media tra i valori misurati
#define READING_CYCLES_FOR_AVG 15
/// Delay tra una lettura e un'altra, nel mezzo il sensore va in power down
/// Questo è praticamente il delay con cui viene schedulata la task
#define BALANCE_DELAY_BETWEEN_READINGS 0


/// INTERNAL:
#define CONVERSION_FACTOR 453.592;// Converte da libbre a grammi
#define BASE_SETPOINT 500.0
/// Valore misurato in fase di calibrazione del peso dei due contenitori
/// @warning questi verranno trasformati in costanti di precompilazione
float SCALE_FACTOR_CHANNEL_A = -7050.0;
float SCALE_FACTOR_CHANNEL_B = -7050.0;
float DEBUG_SETPOINT_CH_A = BASE_SETPOINT;
float DEBUG_SETPOINT_CH_B = BASE_SETPOINT;
float acc = 1.0; // Accuratezza
*/
/**
 ╔════════════════════════════════════════════════════════════════════════════════════════╗
 ║                         ATTENZIONE: Come Fare La Calibrazione                          ║
 ╠════════════════════════════════════════════════════════════════════════════════════════╣
 ║     La calibrazione va fatta una sola volta con dei pesi noti.                         ║
 ║     Perchè questo sia vero, però, bisogna seguire questi punti.                        ║
 ║     Nel caso in cui uno di questi accorgimenti non ci sia bisogna                      ║
 ║     ripetere la calibrazione.                                                          ║
 ║                                                                                        ║
 ║         1) Lo stesso sensore HX711 e la stessa cella di carico vengono usati           ║
 ║                                                                                        ║
 ║         2) L'alimentazione è stabile (stesso voltaggio ogni volta).                    ║
 ║                                                                                        ║
 ║         3) La struttura fisica non cambia il piatto non viene rimosso o riposizionato  ║
 ║                                                                                        ║
 ║         4) Il sensore non subisce urti, deformazioni o variazioni meccaniche.          ║
 ║                                                                                        ║
 ║         5) La temperatura ambientale non varia troppo, oppure si accetta               ║
 ║            una certa tolleranza di errore.                                             ║
 ╚════════════════════════════════════════════════════════════════════════════════════════╝
 */
/*
void BalanceSetup()
{
  Balance.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);


  /// DEBUG:
  const char* topicChA = "BalanceCalibration/SetPoint (grams)/CH A";
  const char* topicChB = "BalanceCalibration/SetPoint (grams)/CH B";
  const char* topicAcc = "BalanceCalibration/Accuracy";
  /// DEBUG: setta un topic in cui sottoscriversi per overridare la variabile
  DebugOverrideVar(topicChA, &DEBUG_SETPOINT_CH_A);
  /// DEBUG: setta un topic in cui sottoscriversi per overridare la variabile
  DebugOverrideVar(topicChB, &DEBUG_SETPOINT_CH_B);
  /// DEBUG: setta un topic in cui sottoscriversi per overridare la variabile
  DebugOverrideVar(topicChB, &DEBUG_SETPOINT_CH_B);
  LogDebug("Auto Calibration", "Togliere tutto dalle celle di carico.\
                                \nImpostare un setpoint in grammi a uno di questi MQTT topic e appoggiare un peso noto (Max 1000g):\
                                \n%s    ---> Setpoint attuale : %.4f [g],\
                                \n%s    ---> Setpoint attuale : %.4f [g]\
                                \n%s                 ---> Accuratezza attuale : +-%.4f",
                                topicChA, DEBUG_SETPOINT_CH_A, topicChB, DEBUG_SETPOINT_CH_B, topicAcc, acc);  
}

void BalanceLoop()
{
  static float ValChA;
  static float ValChB;


  /// @warning Il loop viene chiamato ad ogni ciclo della task(BALANCE_DELAY_BETWEEN_READINGS)

  /// Accende la bilancia
  Balance.power_up();
  /// Ottiene il valore sulla cella di carico sul canale A (scelto dal gain)
  Balance.set_gain(128);
  Balance.set_scale(SCALE_FACTOR_CHANNEL_A);
  ValChA = Balance.get_units(READING_CYCLES_FOR_AVG) * CONVERSION_FACTOR;// Converte anche da libbre a grammi
  /// Ottiene il valore sulla cella di carico sul canale B (scelto dal gain)
  Balance.set_gain(32);
  Balance.set_scale(SCALE_FACTOR_CHANNEL_B);
  ValChB = Balance.get_units(READING_CYCLES_FOR_AVG) * CONVERSION_FACTOR;// Converte anche da libbre a grammi
  /// Spegne la bilancia per il delay specificato
  Balance.power_down();
*/

  /**
   * @section Calibrazione Automatica
   * @todo fare in modo che vi sia un controllo dell'errore e quindi 
   *       avere la calibrazione il più accurata possibile
   */
/*  static bool CalibDoneChA = false;
  static bool CalibDoneChB = false;

  /// Se qualcuno cambia i setpoint allora viene fatta la calibrazione automatica finchè entrambe non si completano
  if((DEBUG_SETPOINT_CH_A != BASE_SETPOINT || DEBUG_SETPOINT_CH_B != BASE_SETPOINT) && !(CalibDoneChA || CalibDoneChB))
  {
    if(ValChA < (DEBUG_SETPOINT_CH_A - acc))
      SCALE_FACTOR_CHANNEL_A += 0.1;
    else if(ValChA > (DEBUG_SETPOINT_CH_A + acc))
      SCALE_FACTOR_CHANNEL_A -= 0.1;
    else if(!CalibDoneChA)
    {
      CalibDoneChA = true;
      LogInfo("Calib CH A", "Calibrazione canale A completata");
    }
    
    if(ValChB < (DEBUG_SETPOINT_CH_B - acc))
      SCALE_FACTOR_CHANNEL_B += 0.1;
    else if(ValChB > (DEBUG_SETPOINT_CH_B + acc))
      SCALE_FACTOR_CHANNEL_B -= 0.1;
    else if(!CalibDoneChB)
    {
      CalibDoneChB = true;
      LogInfo("Calib CH B", "Calibrazione canale B completata");
    }
  }
}

*/









