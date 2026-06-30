
#include "SavingFilesSD.hpp"

SaveToFile GestoreFile;

void MakeFileExample()
{
  uint32_t timeStart = micros();

  /// Variabile del percorso file e per scrivere i commenti. Questi ultimi servono solo quando si apre fisicamente il file e si legge, non sono strettamente neccessari
  String path = "/ExampleDirectory/MakeFileExample.txt", comment = "";

  GestoreFile.mkdir("/ExampleDirectory");      // Crea una directory

  comment = "sezione sensore : DHT11";         // crea il comment block, considera che verrà tutto in maiuscolo
  GestoreFile.drawBlock(path, comment);

  comment = "Temperatura ambientale";
  GestoreFile.setValueByKey<float>(path, "Temperatura", 26.15, comment);

  comment = "Umidità relativa percentuale";
  GestoreFile.setValueByKey<uint8_t>(path, "Umidità", 26.15, comment);

  comment = "sezione a casissimissimissimo";    // crea un secondo comment block, considera che verrà tutto in maiuscolo
  GestoreFile.drawBlock(path, comment);

  comment = "Stringa a caso";
  GestoreFile.setValueByKey<String>(path, "RandomStr", "Questa è la famosa stringa a caso bruh", comment);

  comment = "variabili senza commenti";         // crea un terzo comment block, considera che verrà tutto in maiuscolo
  GestoreFile.drawBlock(path, comment);

  GestoreFile.setValueByKey<int>(path, "NoCommentKey", 1518);

  uint32_t timeEnd = micros();

  Serial.printf("tempo esecuzione Get File : %dus ~= %dms\n", timeEnd-timeStart, (timeEnd - timeStart) / 1000);
}

void GetFileExample()
{
  uint32_t timeStart = micros();

  /// Variabile del percorso file e per scrivere i commenti. Questi ultimi servono solo quando si apre fisicamente il file e si legge, non sono strettamente neccessari
  String path = "/ExampleDirectory/MakeFileExample.txt";

  Serial.printf("Stampo i valori precedentemente scritti sulla console :\n\n");

  Serial.printf("%s = %.2f\n", "Temperatura", GestoreFile.getValueByKey<float>(path, "Temperatura"));

  Serial.printf("%s = %d\n",   "Umidità", GestoreFile.getValueByKey<uint8_t>(path, "Umidità"));

  Serial.printf("%s = %s\n",   "RandomStr", GestoreFile.getValueByKey<String>(path, "RandomStr").c_str());

  Serial.printf("%s = %d\n",   "NoCommentKey", GestoreFile.getValueByKey<int>(path, "NoCommentKey"));

  uint32_t timeEnd = micros();

  Serial.printf("tempo esecuzione Get File : %dus ~= %dms\n", timeEnd-timeStart, (timeEnd - timeStart) / 1000);
}



void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("pronto per cominciare\n");

  while(!GestoreFile.Init(SD_SCK, SD_MISO, SD_MOSI, SD_CS)); // Inizializza la SD card


  buffSD tmpBuff;

  //GestoreFile.ls("/", 0, tmpBuff);                   // Lista i file nella root directory
  //GestoreFile.mkdir("/testDirectory");                        // Crea una directory
  //GestoreFile.ls("/", 0, tmpBuff);                   // Lista i file nella root directory
  //GestoreFile.rmdir("/mydir");                        // Rimuove la directory creata prima
  //GestoreFile.ls("/", 3, tmpBuff);                   // Lista la root directory con ricorsività di terzo livello, se a livello 0 si vedeva solo /mydir, a livello 3 si vedrà /mydir/qualche/altro/file
  //GestoreFile.writeFile("/testDirectory/UserDatasFile.txt", "Key1\n");      // Crea /UserDatasFile.txt e scrive qualcosa
  //Serial.println("\n\nPronto per Leggere\n");
  //GestoreFile.appendFile("/hello.txt", "World!\n");   // Aggiunge "World!\n" in fondo
  //GestoreFile.readFile("/testDirectory/UserDatasFile.txt");                 // Legge il file e stampa il contenuto sul serial monitor
  //GestoreFile.rmfile("/foo.txt");                 // cancella foo.txt se esiste
  //GestoreFile.renameFile("/hello.txt", "/foo.txt");   // rinomina hello.txt in foo.txt
  //GestoreFile.readFile("/foo.txt");                   // legge il nuovo file
  
  SD_ByteUnit_t ByteUnit = MB;
  String StrByteUnit = GestoreFile.byteUnitStr(ByteUnit);

  Serial.printf("Total space: %.6f%s\n", GestoreFile.totalMemory(ByteUnit), StrByteUnit); // Dimensione totale
  Serial.printf("Free  space: %.6f%s\n", GestoreFile.freeMemory(ByteUnit),  StrByteUnit); // Dimensione usata nella SD
  Serial.printf("Used  space: %.6f%s\n", GestoreFile.usedMemory(ByteUnit),  StrByteUnit); // Dimensione usata nella SD



  /// Esegue l'esempio
  MakeFileExample(); // scrive il file
  GetFileExample();  // Legge il file
}

void loop(){}
