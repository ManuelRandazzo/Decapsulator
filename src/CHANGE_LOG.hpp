#pragma once


/**
 *  Dear mantainer :
 *  When we wrote this code, only god and
 *  us knew how it worked.
 *  Now, only god knows it!
 *
 *  REMEMBER : if it works and you don't know 
 *             how, don't ever touch it again please !!! 
 *             You've probably summoned the right magical 
 *             creature (or just sold your soul to the Devil)
 *
 *  Therefore, if you are trying to optimize
 *  this routine and it fails (most surely)
 *  please increment this counter as a
 *  warning for the next person :
 */
#define TOTAL_HOURS_WASTED_HERE 272

// @link_di_drive:https://drive.google.com/drive/u/1/folders/1zrbpykBCAI9a7m_Ivwn7NhQyyCF_MRep

#if TOTAL_HOURS_WASTED_HERE > 100
  #define GET_A_LIFE_PAL !!!
#endif

/**
  Qui non viene compilato ma rimane come Tab



╔════════════════════════════════════════════════╗
║           @date	    :	     18-12-2025	         ║
╚════════════════════════════════════════════════╝



╔════════════════════════════════════════════════╗
║          ADDED: (nuove funzionalità)           ║
╚════════════════════════════════════════════════╝
-

╔════════════════════════════════════════════════╗
║         REMOVED: (funzionalità rimosse) 	     ║
╚════════════════════════════════════════════════╝
- 

╔════════════════════════════════════════════════╗
║  CHANGED: (modifiche a funzionalità esistenti) ║
╚════════════════════════════════════════════════╝
- Cambiata radicalmente nuovamente la gestione della classe DRV8825_Decapsulator resa thread safe
  (La vecchia versione è stata backuppata nel Drive)

- Ora alla FINE del setup() NON viene più eliminata la task che gestisce il loop() perchè causava malfunzionamenti del WiFi

╔════════════════════════════════════════════════╗
║      BUGS: TO: FIX: (bug da correggere)        ║
╚════════════════════════════════════════════════╝
- Molti bug tutti relativi alla gestione del @file Contenitori.hpp

- Deprecated legacy della vecchia libreria RMT sulla libreria del driver del decapsulator

╔════════════════════════════════════════════════╗
║               FIXED: (bug corretti)            ║
╚════════════════════════════════════════════════╝
- 

╔════════════════════════════════════════════════╗
║         TODO:  (cose ancora da fare)           ║
╚════════════════════════════════════════════════╝
  - @todo Capire se ha senso portare tutti/molti metodi della Motion @class
    direttamente sulla libreria del DRV8825.
    (Aspettare di capire se il DRV8825 sarà effettivamente scelto come driver)

  - @todo Cambiare la gestione della safety direttamente ciclando una task di safety
    (usare una task normale e non oggetto della @class TaskTypeDef)
    perchè potrebbe essere che l'interrupt venga perso, ricordandosi di impostare la
    priorità massima.
    Ricordarsi di togliere la gestione dei pin nFAULT dal driver perchè non ci saranno interrupt
    (motivo in più per fare tutta la gestione sul file della @class DRV8825) anche perchè non 
    servirà più gestire i mutex o gli spinlocks.

  - @todo Bisogna controllare che vTaskDelete(NULL); (funzione che elimina completamente la task di loop)
    alla fine del setup non causi crash quando gestisce Seriali, WiFi ecc.











*/