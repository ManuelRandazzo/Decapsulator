
#pragma once

/// @link: https://en.cppreference.com/w/cpp/types/is_same.html,  
/// @note: Non funziona per il microcontrollore di Arduino normale
#include <type_traits>

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <vector>
#include <string>

#define buffSD std::vector<std::string>

/// Configurazione dei pin SPI dedicati sull'ESP32-S3
#define SD_CS 10
#define SD_MOSI 11
#define SD_SCK 12
#define SD_MISO 13



typedef enum : uint32_t
{
  GB = 1024 * 1024 * 1024,
  MB = 1024 * 1024,
  kB = 1024,
  B  = 1,
}
SD_ByteUnit_t;




class SaveToFile
{
  public :
    /// Costruttore
    SaveToFile() {};

    /// Inizializza la SD Card con i pin assegnati e logga alcune informazioni circa l'SD montata
    bool Init(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t cs);

    /// Lista i file in una certa directory
    void ls(String dirname, uint8_t levels, buffSD& existingFiles);

    /// Crea una nuova directory
    void mkdir(const String FilePath);

    /// Rimuove una directory e tutte le sue sotto-directory
    void rmdir(const String FilePath);

    /// Legge il contenuto di un file
    String readFile(const String FilePath);

    /// Legge una determinata riga di un file
    String readFileRow(const String FilePath, uint32_t row);

    /// Disegna un comment block alla fine del file per visualizzare meglio le categorie delle variabili
    void drawBlock(String FilePath, String comment, const uint8_t spazi_tra_bordi_e_commento = 10);

    /// Ritorna il valore corrispondende ad una chiave (versione non esplicita).
    /// @warning Se il parametro non è stato settato da setValueByKey() il dato non verrà letto correttamente
    /// @note dato che viene restituito "auto" è lo USER che deve sapere di che tipo è realmente la variabile ed essere sicuro che la riceva
    auto getValueByKey(const String FilePath, String key);

    /// Ritorna il valore (del tipo specificato nel template) corrispondende ad una chiave (versione esplicita).
    /// @warning Se il parametro non è stato settato da setValueByKey() il dato non verrà letto correttamente
    template <typename valType>
    valType getValueByKey(const String FilePath, String key);

    /// Scrive su un file la chiave e il valore corrispondende (versione nonesplicita)
    String setValueByKey(const String FilePath, const String key, auto ValueToSet, String comment = "");
    
    /// Scrive su un file la chiave e il valore (del tipo specificato nel template) corrispondende (versione esplicita)
    template <typename valType>
    String setValueByKey(const String FilePath, const String key, valType ValueToSet, String comment = "");

    /// Rimuove una key da un file
    void removeKey(const String FilePath, const String key);

    /// Scrive il contenuto di un file
    void writeFile(const String FilePath, String message);
        
    /// Aggiunge qualcosa alla fine di un file
    void appendFile(const String FilePath, String message);

    /// Rimpiazza una certa stringa in un file (se esiste)
    void replaceInFile(const String FilePath, String ReplaceStr, String WithStr);
      
    /// Rinomina un file
    void renameFile(const String FilePath1, const String FilePath2);
        
    /// Rimuove un file
    void rmfile(const String FilePath);

    /// Restituisce la memoria totale
    double totalMemory(const SD_ByteUnit_t bytes = MB);

    /// Restituisce la memoria usata
    double usedMemory(const SD_ByteUnit_t bytes = MB);

    /// Restituisce la memoria libera
    double freeMemory(const SD_ByteUnit_t bytes = MB);

    /// Restituisce la stringa dell'unità SD_ByteUnit_t
    String byteUnitStr(const SD_ByteUnit_t unit = MB);
    
    /// Tipo della SD card
    sdcard_type_t cardType();

    /// Dimensione della SD card
    uint64_t cardSize();

    /// Elenca quanti settori (blocchi di memoria) SD card (override SDFS class)
    size_t numSectors();
    
    /// Dimensione minima di lettura e di modifica in scrittura(settore) della SD card (override SDFS class)
    size_t sectorSize();
    
    /// legge e copia su un buffer il contenuto della memoria di un determinato settore della SD card (override SDFS class)
    bool readRAW(uint8_t *buffer, uint32_t sector);
    
    /// Copia un buffer in un determinato settore della SD card (override SDFS class)
    bool writeRAW(uint8_t *buffer, uint32_t sector);

  private :
    /// Oggetto per la gestione dei File System
    fs::FS &fs = SD;
};


/*
 * pin 1 - not used          |  Micro SD card     |
 * pin 2 - CS (SS)           |                   /
 * pin 3 - DI (MOSI)         |                  |__
 * pin 4 - VDD (3.3V)        |                    |
 * pin 5 - SCK (SCLK)        | 8 7 6 5 4 3 2 1   /
 * pin 6 - VSS (GND)         | ▄ ▄ ▄ ▄ ▄ ▄ ▄ ▄  /
 * pin 7 - DO (MISO)         | ▀ ▀ █ ▀ █ ▀ ▀ ▀ |
 * pin 8 - not used          |_________________|
 *                             ║ ║ ║ ║ ║ ║ ║ ║
 *                     ╔═══════╝ ║ ║ ║ ║ ║ ║ ╚═════════╗
 *                     ║         ║ ║ ║ ║ ║ ╚══════╗    ║
 *                     ║   ╔═════╝ ║ ║ ║ ╚═════╗  ║    ║
 * Connections for     ║   ║   ╔═══╩═║═║═══╗   ║  ║    ║
 * full-sized          ║   ║   ║   ╔═╝ ║   ║   ║  ║    ║
 * SD card             ║   ║   ║   ║   ║   ║   ║  ║    ║
 * Pin name         |  -  DO  VSS SCK VDD VSS DI CS    -  |
 * SD pin number    |  8   7   6   5   4   3   2   1   9 /
 *                  |                                  █/
 *                  |__▍___▊___█___█___█___█___█___█___/
 *
 * Note:  The SPI pins can be manually configured by using `SPI.begin(sck, miso, mosi, cs).`
 *        Alternatively, you can change the CS pin and use the other default settings by using `SD.begin(cs)`.
 *
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | SPI Pin Name | ESP8266 | ESP32 | ESP32‑S2 | ESP32‑S3 | ESP32‑C3 | ESP32‑C6 | ESP32‑H2 |
 * +==============+=========+=======+==========+==========+==========+==========+==========+
 * | CS (SS)      | GPIO15  | GPIO5 | GPIO34   | GPIO10   | GPIO7    | GPIO18   | GPIO0    |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | DI (MOSI)    | GPIO13  | GPIO23| GPIO35   | GPIO11   | GPIO6    | GPIO19   | GPIO25   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | DO (MISO)    | GPIO12  | GPIO19| GPIO37   | GPIO13   | GPIO5    | GPIO20   | GPIO11   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | SCK (SCLK)   | GPIO14  | GPIO18| GPIO36   | GPIO12   | GPIO4    | GPIO21   | GPIO10   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 *
 * For more info see file README.md in this library or on URL:
 * https://github.com/espressif/arduino-esp32/tree/master/libraries/SD
 */


#pragma region INIZIALIZZAZIONE
bool SaveToFile::Init(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t cs)
{
  SPI.begin(sck, miso, mosi, cs);
  if(!SD.begin(cs))
  {
    Serial.println("Card Mount Failed");
    return false;
  }

  Serial.print("SD Card Type: ");
  switch(SD.cardType())
  {
    case CARD_MMC :
      Serial.println("MMC");
    case CARD_SD :
      Serial.println("SDSC");
    case CARD_SDHC :
      Serial.println("SDHC");
    case CARD_NONE :
      Serial.println("No SD card attached");
    default :
      Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  return true;
}
#pragma endregion INIZIALIZZAZIONE






#pragma region DIRECTORIES MANAGMENT

/// Lista i file in una certa directory
void SaveToFile::ls(String dirname, uint8_t levels, buffSD& existingFiles)
{
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root)
  {
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }

  
  File file;
  do
  {
    file = root.openNextFile("r");
    
    if(file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels)
        ls(file.path(), levels - 1, existingFiles);
    }
    else
      existingFiles.push_back(file.name()); /// Salva sul buffer tutti i file che ci sono nella SD
  }
  while(file);

  /// Stampa tutto il buffer in seriale
  for(const std::string& s : existingFiles)
      Serial.printf("  FILE: %s", s.c_str());  // stampa ogni nome file
  
}

/// Crea una nuova directory
void  SaveToFile::mkdir(const String FilePath)
{
  Serial.printf("Creating Dir: %s\n", FilePath);
  fs.mkdir(FilePath) ? Serial.println("Dir created") : Serial.println("mkdir failed");
}

/// Rimuove una directory e tutte le sue sottodirectory
void SaveToFile::rmdir(const String FilePath)
{
  Serial.printf("Removing Dir: %s\n", FilePath);
  fs.rmdir(FilePath) ? Serial.println("Dir removed") : Serial.println("rmdir failed");
}

#pragma endregion DIRECTORIES MANAGMENT







#pragma region FILES MANAGMENT

  #pragma region READ FROM FILE
    /// Legge il contenuto di un file
    String SaveToFile::readFile(const String FilePath)
    {
      File file = fs.open(FilePath, FILE_READ);
      if(!file)
      {
        if(fs.exists(FilePath))
        {
          Serial.println("Failed to open file for reading");
          return "";
        }
        else
        {
          Serial.println("File doesn't exist");
          return "";
        }
      }



      String FileContent = "";
      while(file.available())
        /// Forma la stringa carattere per carattere
        FileContent += static_cast<char>(file.read());

      file.close(); 

      //Serial.print("Read from file: ");
      //Serial.write(FileContent);

      return FileContent;
      
    }

    /// Legge una determinata riga di un file.
    /// @note Le righe partono da 1, ma se row=0 allora viene incrementato ad 1 
    String SaveToFile::readFileRow(const String FilePath, uint32_t row)
    {
      /// Stringa temporanea che conterrà il contenuto del file
      String FileContent = this->readFile(FilePath);

      /// Tiene traccia del caporiga '\n' per ogni riga
      uint32_t RigheFinoAdOra = 0;
      /// Indice di caporiga
      int32_t i;

      /// se row = 0 allora viene incrementato ad 1 
      if(row == 0)
        row = 1;

      /// Esce se ha trovato la riga
      while( RigheFinoAdOra != row )
      {
        i = FileContent.indexOf("\n");
        RigheFinoAdOra++;

        /// Se ancora non è stata trovata la riga
        if(RigheFinoAdOra != row)
        {
          /// Esce dal metodo se non ci sono più righe
          if(i < 0)
            return "";
          
          /// Rimuove la stringa fino al primo capo riga.
          /// @note i+2 perchè si considera la dimensione di "\n" nella rimozione
          FileContent.remove(0, i + 2);
        }
        else if(i > 0) /// se ci sono altre righe dopo le rimuove, se no manda la stringa direttamente
          FileContent = FileContent.substring(0, i);

      }

      return FileContent;
    }

    /// Ritorna il valore corrispondende ad una chiave (versione non esplicita).
    /// @warning Se il parametro non è stato settato da setValueByKey() il dato non verrà letto correttamente
    /// @note dato che viene restituito "auto" è lo USER che deve sapere di che tipo è realmente la variabile ed essere sicuro che la riceva
    auto SaveToFile::getValueByKey(const String FilePath, String key)
    {
      return getValueByKey<auto>(FilePath, key);
    }
    
    /// Ritorna il valore (del tipo specificato nel template) corrispondende ad una chiave (versione esplicita).
    /// @warning Se il parametro non è stato settato da setValueByKey() il dato non verrà letto correttamente
    template <typename valType>
    valType SaveToFile::getValueByKey(const String FilePath, String key)
    {
      String FileContent = this->readFile(FilePath);

      /// Aggiunge " : " alla stringa utente perchè dopo questo vi sarà il valore
      key += " : ";

      /// Indice in cui inizia la key e quindi inizia il valore
      int startValueIndex = FileContent.indexOf(key);

      /// La stringa non esiste
      if(startValueIndex < 0)
        return static_cast<valType>(NULL);

      /// Indice in cui finisce il valore
      int stopValueIndex = FileContent.indexOf(",\n\n", startValueIndex);

      /// Si salva il valore in una stringa, poi verrà castato in base al tipo supportato
      String ValueStrToCast = FileContent.substring(startValueIndex + key.length(), stopValueIndex);

      /// Valore da ritornare del tipo specificato dall'utente
      valType ValueToReturn;

      if constexpr (std::is_integral_v<valType>) 
        ValueToReturn = static_cast<valType>(ValueStrToCast.toInt());
      else if constexpr (std::is_same_v<valType, float>)
        ValueToReturn = ValueStrToCast.toFloat();
      else if constexpr (std::is_same_v<valType, double>)
        ValueToReturn = ValueStrToCast.toDouble();
      else if constexpr (std::is_same_v<valType, char> || std::is_same_v<valType, unsigned char>)
        ValueToReturn = static_cast<valType>(ValueStrToCast.charAt(0));
      else if constexpr (std::is_same_v<valType, String>)
        ValueToReturn = ValueStrToCast;


      /// Ritorna il valore castato della stringa
      return ValueToReturn;
    }

    /// Disegna un comment block alla fine del file per visualizzare meglio le categorie delle variabili
    void SaveToFile::drawBlock(String FilePath, String comment, const uint8_t spazi_tra_bordi_e_commento)
    {
      ///         Risultato simile a:   
      ///
      ///    ╔═══════════════════════════╗
      ///    ║          COMMENT          ║
      ///    ╚═══════════════════════════╝

      String FileContent = this->readFile(FilePath);

      /// Se il comment block esiste già non lo riscrive
      if(FileContent.indexOf(comment) != -1)
        return;

      
      String StrToSend = "", EdgeSupInf = "", Spaces = "";

      for(uint8_t i = 0; i < comment.length(); i++)
        EdgeSupInf += "═";    // Aggiunge tanti caratteri tant'è la lunghezza della stringa

      for(uint8_t i = 0; i < spazi_tra_bordi_e_commento; i++)
      {
        EdgeSupInf += "══";   // Aggiunge due caratteri tant'è lo spazio dal bordo del commento
        Spaces += " ";        // Aggiunge tanti caratteri tant'è lo spazio dal bordo del commento
      }

      comment.toUpperCase();

      StrToSend += "╔" +        EdgeSupInf         + "╗\n";
      StrToSend += "║" + Spaces + comment + Spaces + "║\n";
      StrToSend += "╚" +        EdgeSupInf         + "╝\n";

      this->appendFile(FilePath, StrToSend);
    }
  #pragma endregion READ FROM FILE

  #pragma region WRITE TO FILE

    /// Scrive su un file la chiave e il valore corrispondente (versione non esplicita)
    String SaveToFile::setValueByKey(const String FilePath, const String key, auto ValueToSet, String comment)
    {
      String FileContent = this->readFile(FilePath);

      /// Se esiste un commento lo crea
      comment = comment != "" ? "///" + comment : "";

      /// Compone la riga nel formato giusto per poter poi essere letta da getValueByKey()    Risultato:      /// comment
      String NewStr = comment + "\n" + key + " : " + String(ValueToSet) + ",\n\n";//                          key : ValueToSet,\n\n

      /// Se la chiave esiste già (endComment != -1) allora la sovrascrive
      int endComment = FileContent.indexOf("\n" + key);
      if(endComment != -1)
      {
        /// Trova dove comincia il commento
        int startComment = FileContent.lastIndexOf("/// ", endComment) - 3;

        /// Copia il commento in base agli indici trovati
        String OldComment = FileContent.substring(startComment, endComment);
        
        /// Copia il vecchio valore
        String OldValue = this->getValueByKey<String>(FilePath, key);

        /// Compone la stringa che deve essere sostituita
        String OldStr = OldComment + key + " : " + OldValue + ",\n\n";

        /// Rimpiazza la vecchia key con quella nuova
        replaceInFile(FilePath, OldStr, NewStr);    
      }
      else /// Aggiunge la "key : value," alla fine del file
        this->appendFile(FilePath, NewStr.c_str());

      /// Ritorna la stringa scritta
      return NewStr;
    }

    /// Scrive su un file la chiave e il valore (del tipo specificato nel template) corrispondende (versione esplicita)
    template <typename valType>
    String SaveToFile::setValueByKey(const String FilePath, const String key, valType ValueToSet, String comment)
    {
      return setValueByKey(FilePath, key, ValueToSet, comment);
    }

    /// Rimuove una key da un file
    void SaveToFile::removeKey(const String FilePath, const String key)
    {
      String FileContent = this->readFile(FilePath);

      /// Se la chiave esiste (endComment != -1) allora la rimuove
      int endComment = FileContent.indexOf("\n" + key);
      if(endComment != -1)
      {
        /// Trova dove comincia il commento
        int start = FileContent.lastIndexOf("/// ", endComment) - 3;
        
        /// Trova dove finisce la key
        int end = FileContent.indexOf(",\n\n", start) + 2;

        /// Ricava la stringa che contiene tutta la key
        String FullKey = FileContent.substring(start, end);

        /// Rimpiazza la key con un astringa vuota ("")
        replaceInFile(FilePath, FullKey, "");    
      }
    }

    /// Scrive TUTTO il contenuto di un file CANCELLANDO TUTTO quello vecchio
    void SaveToFile::writeFile(const String FilePath, const String message)
    {
      /// Se il file non esiste lo crea e ci scrive dentro
      File file = fs.exists(FilePath) ? fs.open(FilePath, FILE_WRITE) : fs.open(FilePath, FILE_WRITE, true);

      /// Problema ad aprire il file
      if(!file)
        return;

      file.print(message) ? Serial.println("File written") : Serial.println("Write failed");
      
      file.close();
    }

    /// Scrive qualcosa alla fine di un file, utile per scrivere dati
    void SaveToFile::appendFile(const String FilePath, const String message)
    {
      File file = fs.exists(FilePath) ? fs.open(FilePath, FILE_APPEND) : fs.open(FilePath, FILE_APPEND, true);

      /// Problema ad aprire il file
      if(!file)
        return;

      file.print(message) ? Serial.println("Message appended") : Serial.println("Append failed");
      
      file.close();
    }

    /// Rimpiazza una certa stringa in un file (se esiste)
    void SaveToFile::replaceInFile(const String FilePath, const String ReplaceStr, const String WithStr)
    {
      /// Legge il file
      String FileContent = this->readFile(FilePath);
      
      /// Rimpiazza
      FileContent.replace(ReplaceStr, WithStr);

      /// Riscrive il file
      writeFile(FilePath, FileContent.c_str());
    }
  #pragma endregion WRITE TO FILE

  #pragma region RENAME / REMOVE FILE
    /// Rinomina un file
    void SaveToFile::renameFile(const String FilePath1, const String FilePath2)
    {
      Serial.printf("Renaming file %s to %s\n", FilePath1, FilePath2);
      fs.rename(FilePath1, FilePath2) ? Serial.println("File renamed") : Serial.println("Rename failed");
    }

    /// Rimuove un file
    void SaveToFile::rmfile(const String FilePath)
    {
      Serial.printf("Deleting file: %s\n", FilePath);
      fs.remove(FilePath) ? Serial.println("File deleted") : Serial.println("Delete failed");
    }
  #pragma endregion RENAME / REMOVE FILE
#pragma endregion FILES MANAGMENT









#pragma region MEMORY INFOS

/// Restituisce la memoria totale
double SaveToFile::totalMemory(const SD_ByteUnit_t bytes) 
{
  return double(SD.totalBytes()) / double(bytes);
}

/// Restituisce la memoria usata
double SaveToFile::usedMemory(const SD_ByteUnit_t bytes)
{
  return double(SD.usedBytes()) / double(bytes); 
}

/// Restituisce la memoria libera
double SaveToFile::freeMemory(const SD_ByteUnit_t bytes)
{
  return double(SD.totalBytes() - SD.usedBytes()) / double(bytes);
}

/// Restituisce la stringa dell'unità SD_ByteUnit_t
String SaveToFile::byteUnitStr(const SD_ByteUnit_t ByteUnit)
{
  switch(ByteUnit)
  {
    case GB : return "GB";
    case MB : return "MB";
    case kB : return "kB";
    case B  : return "B";
    default : return "Invalid ByteUnit";
  }
}
    
/// Tipo della SD card
sdcard_type_t cardType()
{
  return SD.cardType();
}

/// Dimensione della SD card
uint64_t cardSize()
{
  return SD.cardType();
}

/// Elenca quanti settori (blocchi di memoria) SD card (override SDFS class)
size_t numSectors()
{
  return SD.numSectors();
}

/// Dimensione minima di lettura e di modifica in scrittura(settore) della SD card (override SDFS class)
size_t sectorSize()
{
  return SD.sectorSize();
}

/// legge e copia su un buffer il contenuto della memoria di un determinato settore della SD card (override SDFS class)
bool readRAW(uint8_t* buffer, uint32_t sector)
{
  return SD.readRAW(buffer, sector);
}

/// Copia un buffer in un determinato settore della SD card (override SDFS class)
bool writeRAW(uint8_t* buffer, uint32_t sector)
{
  return SD.writeRAW(buffer, sector);
}

#pragma endregion MEMORY INFOS



