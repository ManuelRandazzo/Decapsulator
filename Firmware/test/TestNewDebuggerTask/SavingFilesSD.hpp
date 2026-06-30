
#pragma once

/// @link: https://en.cppreference.com/w/cpp/types/is_same.html,  
/// @note: Non funziona per il microcontrollore di Arduino normale
#include <type_traits>

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <vector>
#include <string>
#include "Debug.hpp"

#define buffSD std::vector<std::string>

/// Configurazione dei pin SPI dedicati sull'ESP32-S3
#define SD_CFG_CS   SD_CS
#define SD_CFG_MOSI TFT_MOSI
#define SD_CFG_SCK  TFT_SCLK
#define SD_CFG_MISO TFT_MISO


/// @brief Byte effettivi
typedef enum : uint32_t
{
    GiB = 1024 * 1024 * 1024,
    MiB = 1024 * 1024,
    KiB = 1024,
    B   = 1,
}
SD_ByteUnit_t;




/**
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

        /// Ritorna il valore (del tipo specificato nel template) corrispondende ad una chiave (versione esplicita).
        /// @warning Se il parametro non è stato settato da setValueByKey() il dato non verrà letto correttamente
        template <typename valType>
        valType getValueByKey(const String FilePath, String key);

        /// Scrive su un file la chiave e il valore corrispondende di qualsiasi tipo
        String setValueByKey(const String FilePath, const String key, auto ValueToSet, String comment = "");

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
        double totalMemory(const SD_ByteUnit_t bytes = MiB);

        /// Restituisce la memoria usata
        double usedMemory(const SD_ByteUnit_t bytes = MiB);

        /// Restituisce la memoria libera
        double freeMemory(const SD_ByteUnit_t bytes = MiB);

        /// Restituisce la stringa dell'unità SD_ByteUnit_t
        String byteUnitStr(const SD_ByteUnit_t unit = MiB);
        
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

