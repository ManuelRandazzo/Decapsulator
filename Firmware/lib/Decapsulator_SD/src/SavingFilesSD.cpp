#include "SavingFilesSD.hpp"

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
bool SaveToFile::Init(SPIClass &spi, uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t cs)
{
	if(!SD.begin(cs, spi, SPI_SD_FREQUENCY, "/sd_card", 15, false))
	{
		Serial.printf("\nSD Card Init\nCard Mount Failed, check connections :\
								     \nSD Card Declared Pins:\
								     \n   - Sck : %d,\
								     \n   - Miso: %d,\
								     \n   - Mosi: %d,\
								     \n   - Cs  : %d\n\n", 
								     sck, miso, mosi, cs);
		return false;
	}

	String CardType;
	double cardSize = (double)SD.cardSize() / (double)MiB;
	switch(SD.cardType())
	{
		case CARD_MMC  : CardType = "MMC"; break;
		case CARD_SD   : CardType = "SDSC"; break;
		case CARD_SDHC : CardType = "SDHC"; break;
		case CARD_NONE : Serial.printf("\nSD Card Infos\nNo SD card attached"); return false;
		default        : Serial.printf("\nSD Card Infos\nUnknown Card Type"); return false;
	}
	
	Serial.printf("\nSD Card Infos\nSD Card Type: %s\
							  \nSD Card Size: %.6fMiB\n", CardType.c_str(), cardSize);

	return true;
}
#pragma endregion INIZIALIZZAZIONE






#pragma region DIRECTORIES MANAGMENT

/// Lista i file in una certa directory
void SaveToFile::ls(String dirname, uint8_t levels, buffSD& existingFiles)
{
	Serial.printf("\nSD Card List\nListing directory: %s\n", dirname.c_str());

	File root = fs.open(dirname);
	if(!root)
	{
		Serial.printf("\nSD Card List\nFailed to open directory");
		return;
	}

	if(!root.isDirectory())
	{
		Serial.printf("\nSD Card List\nNot a directory");
		return;
	}

	
	File file;
	String PathInfo;
	do
	{
		file = root.openNextFile("r");
		
		if(file.isDirectory())
		{
			PathInfo += file.name();

			/// Ricorsione
			if(levels)
				ls(file.path(), levels - 1, existingFiles);
		}
		else
			existingFiles.push_back(file.name()); /// Salva sul buffer tutti i file che ci sono nella SD
	}
	while(file);

	/// Stampa tutto il buffer
	for(const std::string& s : existingFiles)
	{
		PathInfo += s.c_str();  // stampa ogni nome file
		PathInfo += "/";
	}
	
	Serial.printf("\nSD Card List\n%s", PathInfo.c_str());
}

/// Crea una nuova directory
void  SaveToFile::mkdir(const String FilePath)
{
	Serial.printf("\nSD Card Make Directory\nCreating Directory: %s ...\n", FilePath.c_str());
	if(fs.mkdir(FilePath))
		Serial.printf("\nSD Card Make Directory\nDirectory created successfully: %s", FilePath.c_str());
	else
		Serial.printf("\nSD Card Make Directory\nFailed to make the directory: %s", FilePath.c_str());
}

/// Rimuove una directory e tutte le sue sottodirectory
void SaveToFile::rmdir(const String FilePath)
{
	Serial.printf("\nSD Card Remove Directory\nRemoving Dir: %s\n ...", FilePath.c_str());
	if(fs.rmdir(FilePath))
		Serial.printf("\nSD Card Remove Directory\nDirectory removed successfully: %s", FilePath.c_str());
	else
		Serial.printf("\nSD Card Remove Directory\nFailed to remove the directory: %s", FilePath.c_str());
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
				Serial.printf("\nSD Card Read\nFailed to open file for reading : %s", FilePath.c_str());
				return "";
			}
			else
			{
				Serial.printf("\nSD Card Read\nFile doesn't exist : %s", FilePath.c_str());
				return "";
			}
		}



		String FileContent = "";
		while(file.available())
			/// Forma la stringa carattere per carattere
			FileContent += static_cast<char>(file.read());

		file.close();

		Serial.printf("\nSD Card Read\nFile readed successfully : %s\n", FilePath.c_str());

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

		Serial.printf("\nSD Card read row\nRow readed successfully : %s\nContent of row : %s\n", FilePath.c_str(), FileContent.c_str());

		return FileContent;
	}

  	#pragma endregion READ FROM FILE

  	#pragma region WRITE TO FILE

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
		{
			Serial.printf("\nSD Card draw section block\nComment block existing yet. Leaving without changes");
			return;
		}

		
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
		/// Stringa per fare i log    
		String Logs;

		/// Salva se il file esiste o meno
		const bool exists = fs.exists(FilePath);
		Logs += exists ? "File already exists, proceed to overwrite it.\n" : "File doesn't exist, creating a new one.\n";

		/// Se il file non esiste lo crea e ci scrive dentro
		File file = fs.open(FilePath, FILE_WRITE, !exists);

		/// Problema ad aprire il file
		if(!file)
		{
			Serial.printf("\nSD Card Write\nProblem while opening the file.\nAborting the process and leaving...");
			return;
		}

		Logs += file.print(message) ? "File written succesfully" : "Write failed";
		
		Serial.printf("\nSD Card Write\n%s", Logs.c_str());

		file.close();
	}

	/// Scrive qualcosa alla fine di un file, utile per scrivere dati
	void SaveToFile::appendFile(const String FilePath, const String message)
	{
		/// Stringa per fare i log    
		String Logs;

		/// Salva se il file esiste o meno
		const bool exists = fs.exists(FilePath);
		File file = exists ? fs.open(FilePath, FILE_APPEND) : fs.open(FilePath, FILE_APPEND, true);
		Logs += exists ? "File already exists, proceed to overwrite it.\n" : "File doesn't exist, creating a new one.\n";

		/// Problema ad aprire il file
		if(!file)
		{
			Serial.printf("\nSD Card Write\nProblem while opening the file.\nAborting the process and leaving...");
			return;
		}

		Logs += file.print(message) ? "Message appended succesfully" : "Append failed";
	
		Serial.printf("\nSD Card Write\n%s", Logs.c_str());
	
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
		if(fs.rename(FilePath1, FilePath2))
			Serial.printf("\nSD Card Rename file\nRenaming file %s to %s.\nFile renamed successfully", FilePath1.c_str(), FilePath2.c_str());
		else
			Serial.printf("\nSD Card Rename file\nRenaming file %s to %s.\nRename failed", FilePath1.c_str(), FilePath2.c_str());
	}

	/// Rimuove un file
	void SaveToFile::rmfile(const String FilePath)
	{
		if(fs.remove(FilePath))
			Serial.printf("\nSD Card Remove file\nDeleting file: %s.\nFile deleted successfully.", FilePath.c_str());
		else
			Serial.printf("\nSD Card Remove file\nDeleting file: %s.\nDelete failed.", FilePath.c_str());
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
		case GiB : return "GiB";
		case MiB : return "MiB";
		case KiB : return "KiB";
		case B   : return "B";
		default  : return "Invalid ByteUnit";
	}
}

/// Tipo della SD card
sdcard_type_t SaveToFile::cardType()
{
	return SD.cardType();
}

/// Dimensione della SD card
uint64_t SaveToFile::cardSize()
{
	return SD.cardType();
}

/// Elenca quanti settori (blocchi di memoria) SD card (override SDFS class)
size_t SaveToFile::numSectors()
{
	return SD.numSectors();
}

/// Dimensione minima di lettura e di modifica in scrittura(settore) della SD card (override SDFS class)
size_t SaveToFile::sectorSize()
{
	return SD.sectorSize();
}

/// legge e copia su un buffer il contenuto della memoria di un determinato settore della SD card (override SDFS class)
bool SaveToFile::readRAW(uint8_t* buffer, uint32_t sector)
{
	return SD.readRAW(buffer, sector);
}

/// Copia un buffer in un determinato settore della SD card (override SDFS class)
bool SaveToFile::writeRAW(uint8_t* buffer, uint32_t sector)
{
	return SD.writeRAW(buffer, sector);
}

#pragma endregion MEMORY 


SaveToFile SD_Card;