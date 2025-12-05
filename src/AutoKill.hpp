#pragma once

/// Include la libreria custom per la gestione delle Task
#include "Tasks.hpp"
/// Include la libreria custom per la gestione degli Interrupt
#include "Interrupts.hpp"
/// Include la libreria custom per il Debug
#include "Debug.hpp"
/// Include la libreria custom per la gestione dell'SD
#include "SavingFilesSD.hpp"


SaveToFile SD_Card;


/// Inizializza l'SD Card
bool InitSD_Card(SD_ByteUnit_t ByteUnit = MiB, uint32_t TimeoutMs = 3000, bool restart_if_timeout_occurred = true)
{
  uint32_t StartTimeout = millis();

  /// Inizializza la SD card
  while(!SD_Card.Init(SD_SCK, SD_MISO, SD_MOSI, SD_CS))
  {
    /// Se passa troppo tempo blocca la funzione
    if(millis() - StartTimeout >= TimeoutMs)
    {
      if(restart_if_timeout_occurred)
      {
        LogError("SD Card Initializzation", 
                 "SD Card initializzation failed!\
                  \nRebooting ESP32 in 3 seconds...");
        delay(3000);
        ESP.restart();
      }
      else
        LogError("SD Card Initializzation", 
                "SD Card initializzation failed!\
                 \nATTENZIONE: I dati non verranno salvati in caso di reset,\
                 \n            spegnimento o perdita di corrente\n. \
                 \n            Continuo con il resto del programma...");

      return false;
    }
  }
  
  LogInfo("SD Card Init Infos", 
          "Printing Infos about your SD Card \
          \nTotal space: %.6f%s, \
          \nFree  space: %.6f%s, \
          \nUsed  space: %.6f%s", 
          SD_Card.totalMemory(ByteUnit), SD_Card.byteUnitStr(ByteUnit),  // Dimensione totale
          SD_Card.freeMemory(ByteUnit),  SD_Card.byteUnitStr(ByteUnit),  // Dimensione ancora disponibile nella SD
          SD_Card.usedMemory(ByteUnit),  SD_Card.byteUnitStr(ByteUnit)); // Dimensione usata nella SD

  return true;
}

/// ATTENZIONE: Questa funzione va modificata in base alle variabili che bisogna leggere dal file
void ReadAllStoredValues()
{
  #ifdef LOG_ACTIVE
    /// Viene salvato il tempo in cui inizia la funzione
    uint32_t timeStart = micros();
  #endif





  #ifdef LOG_ACTIVE
    /// Viene salvato il tempo in cui inizia la funzione
    uint32_t timeEnd = micros();
    /// Stampa il tempo impiegato per salvare i dati
    LogInfo("WriteAllVariables", "Tempo esecuzione WriteAllVariables : %dus ~= %dms", timeEnd-timeStart, (timeEnd - timeStart) / 1000);
  #endif
}

/// ATTENZIONE: Questa funzione va modificata in base alle variabili che bisogna scrivere nel file
bool WriteAllValues()
{
  #ifdef LOG_ACTIVE
    /// Viene salvato il tempo in cui inizia la funzione
    uint32_t timeStart = micros();
  #endif



  /// Variabile dei percorsi file
  String FilePath;
  /// @commenti: servono solo quando si apre fisicamente il file e si legge. Non sono strettamente neccessari
  String comment = "";




  /*╔════════════════════════════════════════════════════╗*/
  /*║             SALVATAGGI NEL FILE UTENTE             ║*/
  /*╚════════════════════════════════════════════════════╝*/

  /// Se ancora non esiste viene creata una nuova directory
  FilePath = "/Decapsulator/UserSaveFile/UserFileDatas.txt";
  /// Crea la directory
  SD_Card.mkdir(FilePath);

  comment = "sezione sensore : DHT11";         // crea il comment block, considera che verrà tutto in maiuscolo
  SD_Card.drawBlock(FilePath, comment);

  comment = "Temperatura aMiBientale";
  SD_Card.setValueByKey<float>(FilePath, "Temperatura", 26.15, comment);

  comment = "Umidità relativa percentuale";
  SD_Card.setValueByKey<uint8_t>(FilePath, "Umidità", 26.15, comment);

  comment = "sezione a casissimissimissimo";    // crea un secondo comment block, considera che verrà tutto in maiuscolo
  SD_Card.drawBlock(FilePath, comment);

  comment = "Stringa a caso";
  SD_Card.setValueByKey<String>(FilePath, "RandomStr", "Questa è la famosa stringa a caso bruh", comment);

  comment = "variabili senza commenti";         // crea un terzo comment block, considera che verrà tutto in maiuscolo
  SD_Card.drawBlock(FilePath, comment);

  SD_Card.setValueByKey<int>(FilePath, "NoCommentKey", 1518);



  /*╔═════════════════════════════════════════════════════════════════════╗*/
  /*║             SALVATAGGI NEL FILE DELLE VARIABILI INTERNE             ║*/
  /*╚═════════════════════════════════════════════════════════════════════╝*/

  /// Se ancora non esiste viene creata una nuova directory
  FilePath = "/Decapsulator/InternalVariables/InternalVariablesFileDatas.txt";
  /// Crea la directory
  SD_Card.mkdir(FilePath);




  #ifdef LOG_ACTIVE
    /// Viene salvato il tempo in cui inizia la funzione
    uint32_t timeEnd = micros();
    /// Stampa il tempo impiegato per salvare i dati
    LogInfo("WriteAllVariables", "Tempo esecuzione WriteAllVariables : %dus ~= %dms", timeEnd-timeStart, (timeEnd - timeStart) / 1000);
  #endif

  /// Viene notificata la fine della funziones
  return true;
}

void GetFileExample()
{
  #ifdef LOG_ACTIVE
    uint32_t timeStart = micros();
  #endif

  /// Variabile del percorso file e per scrivere i commenti. Questi ultimi servono solo quando si apre fisicamente il file e si legge, non sono strettamente neccessari
  String path = "/ExampleDirectory/MakeFileExample.txt";

  Serial.printf("Stampo i valori precedentemente scritti sulla console :\n\n");

  Serial.printf("%s = %.2f\n", "Temperatura", SD_Card.getValueByKey<float>(path, "Temperatura"));

  Serial.printf("%s = %d\n",   "Umidità", SD_Card.getValueByKey<uint8_t>(path, "Umidità"));

  Serial.printf("%s = %s\n",   "RandomStr", SD_Card.getValueByKey<String>(path, "RandomStr").c_str());

  Serial.printf("%s = %d\n",   "NoCommentKey", SD_Card.getValueByKey<int>(path, "NoCommentKey"));


  #ifdef LOG_ACTIVE
    uint32_t timeEnd = micros();
    Serial.printf("tempo esecuzione Get File : %dus ~= %dms\n", timeEnd-timeStart, (timeEnd - timeStart) / 1000);
  #endif
}



void setup()
{
  InitSD_Card(MiB, 3000, true); // Inizializza la SD card


  buffSD tmpBuff;

  //SD_Card.ls("/", 0, tmpBuff);                   // Lista i file nella root directory
  //SD_Card.mkdir("/testDirectory");                        // Crea una directory
  //SD_Card.ls("/", 0, tmpBuff);                   // Lista i file nella root directory
  //SD_Card.rmdir("/mydir");                        // Rimuove la directory creata prima
  //SD_Card.ls("/", 3, tmpBuff);                   // Lista la root directory con ricorsività di terzo livello, se a livello 0 si vedeva solo /mydir, a livello 3 si vedrà /mydir/qualche/altro/file
  //SD_Card.writeFile("/testDirectory/UserDatasFile.txt", "Key1\n");      // Crea /UserDatasFile.txt e scrive qualcosa
  //Serial.println("\n\nPronto per Leggere\n");
  //SD_Card.appendFile("/hello.txt", "World!\n");   // Aggiunge "World!\n" in fondo
  //SD_Card.readFile("/testDirectory/UserDatasFile.txt");                 // Legge il file e stampa il contenuto sul serial monitor
  //SD_Card.rmfile("/foo.txt");                 // cancella foo.txt se esiste
  //SD_Card.renameFile("/hello.txt", "/foo.txt");   // rinomina hello.txt in foo.txt
  //SD_Card.readFile("/foo.txt");                   // legge il nuovo file



  /// Esegue l'esempio
  //MakeFileExample(); // scrive il file
  //GetFileExample();  // Legge il file
}

void loop(){}
