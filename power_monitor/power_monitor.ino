#include <LiquidCrystal_PCF8574.h>
#include <SD.h> 
#include <Adafruit_INA219.h>

#define LOGGING_INTERVAL  1000         // Logging 1000 times per second
#define CHIP_SELECT       10           // CS pin on the SD card board
#define LCD_FULLBRIGHT    255          // Brightest possible setting for LCD
#define LCD_FULLDIM       0            // Dimmest possible setting for LCD
#define FILENAME_PREFIX   "log_"       // Prefix for log filenames
#define FILE_EXTENSION    ".csv"       // File extension for log files
#define SHUTDOWN_PIN      2            // Pin to turn off the testing
#define MAXFILESIZE_MiB   512          // Max size a file can be in MiB
#define SHUTDOWN_PRESSES  5            // Amount of seconds the shutdown needs to be pressed

Adafruit_INA219 ina219;                // Initialise the INA219 power monitor
LiquidCrystal_PCF8574 lcd(0x27);       // Set the LCD address to 0x27 for a 16 chars and 2 line display
File dataFile;                         // Creating global file

enum VALUE_INDEX                       // Enum to guarantee that the order of all arrays/datapoints is the same
{
  VOLTAGE,
  AMPERAGE
};

char VALUES[] = {'V', 'A'};       // Shorthand notation for units

void setup() 
{
  // Setup crude devices
  pinMode(SHUTDOWN_PIN, INPUT_PULLUP);

  // Setup serial
  Serial.begin(9600);

  // Setup LCD
  lcd.begin(16, 2);
  lcd.setBacklight(LCD_FULLBRIGHT);

  // Setup SD
  if (!SD.begin(CHIP_SELECT)) 
  {
    crashLoop("Unable to locate the SD card");
  }

  File root = SD.open("/");

  while(true)
  {
    File entry = root.openNextFile();
    if(!entry)
    {
      root.close();
      break;
    }
    entry.close();
    SD.remove(entry.name());
  }

  createNewFile();

  // Setup INA219
  if (!ina219.begin())
  {
    crashLoop("Unable to initialise the INA219 chips");
  }

  delay(5000);
}

void loop() 
{
  unsigned long currentTime = millis();
  static unsigned long nextDisplayRefresh = 0;
  static unsigned long nextLoggingMoment = 0;
  static int shutdownPresses = 0;

  // Update display routine
  if (currentTime >= nextDisplayRefresh) 
  {
    lcd.clear();
    lcd.home();
    lcd.print(millisToTime(currentTime / 1000));
    printData(currentTime / 1000);
    printSize();
    checkFileSize();
    nextDisplayRefresh = millis() + 1000;
  }

  // Logging routine
  if (currentTime >= nextLoggingMoment) 
  {
    static int msBetweenLogs = (int) (1000 / LOGGING_INTERVAL);
    dataFile.println(formatLogData());
    nextLoggingMoment = millis() + msBetweenLogs;
  }

  // Shutdown routine
  if (Serial.available()) 
  {
    char command = Serial.read();
    if (command == 's') 
    {
      crashLoop("Finished logging!");
    }
  }

  if(!digitalRead(SHUTDOWN_PIN))
  {
    delay(100);
    lcd.clear();
    lcd.home();
    lcd.print("Shutdown: " + String(shutdownPresses) + "/" + String(SHUTDOWN_PRESSES));
    delay(1000);
    if (shutdownPresses == SHUTDOWN_PRESSES)
    {
      crashLoop("Shutdown!");
    }
    shutdownPresses++;
  }
  else
  {
    shutdownPresses = 0;
  }
}

// Function to create a new file with a unique name using static index
void createNewFile() 
{
  static int fileIndex = 0;

  char fileName[20];
  snprintf(fileName, sizeof(fileName), "%s%d%s", FILENAME_PREFIX, fileIndex, FILE_EXTENSION);
  
  fileIndex++;

  dataFile.flush();
  dataFile.close();

  dataFile = SD.open(fileName, FILE_WRITE);

  if (dataFile) 
  {
    Serial.print("New file created: ");
    Serial.println(fileName);
    dataFile.println("V,µA");
  } 
  else 
  {
    crashLoop("Unable to write to file");
  }
}

// Function that returns String formatted time in h:m:s
String millisToTime(unsigned long totalSeconds) 
{
  int second = totalSeconds % 60;
  totalSeconds = (totalSeconds - second) / 60;
  int minute = totalSeconds % 60;
  totalSeconds = (totalSeconds - minute) / 60;
  int hour = totalSeconds;

  return String(hour) + ":" + String(minute) + ":" + String(second);
}

// Function that formats data as a CSV String
String formatLogData() 
{
  float* data = logData();
  
  return String(data[VOLTAGE]) + "," + String(data[AMPERAGE], 6);
}

// Function that logs the data and returns it as an array
float* logData() 
{
  static float results[2];

  results[VOLTAGE] = ina219.getBusVoltage_V();
  results[AMPERAGE] = ina219.getCurrent_mA();

  return results;
}

void printData(unsigned long time) 
{
  int index = time % 2;
  float value = logData()[index];
  char valueType = VALUES[index];

  lcd.setCursor(0, 1);
  lcd.print(String(valueType) + ":" + String(value, 2));
}

void printSize()
{
  double bytesToMb = (double) dataFile.size() / 1024 / 1024;

  lcd.setCursor(10, 0);
  lcd.print("MiB:");

  lcd.setCursor(10, 1);
  lcd.print(bytesToMb);
}

void checkFileSize()
{
  int bytesToMb = dataFile.size() / 1024 / 1024;
  
  if (bytesToMb >= MAXFILESIZE_MiB)
  {
    createNewFile();
  }
}

// Function that halts all execution and flashes the screen repeatedly
void crashLoop(String errorMsg) 
{
  dataFile.flush();
  dataFile.close();
  lcd.clear();
  lcd.home();
  lcd.print(errorMsg);

  for (int i = 0; i < 20; i++) 
  {
    delay(250);
    lcd.setBacklight(LCD_FULLDIM);
    delay(250);
    lcd.setBacklight(LCD_FULLBRIGHT);
    Serial.println(errorMsg);
  }

  delay(200);
  Serial.flush();
  lcd.setBacklight(LCD_FULLDIM);
  while (true);
}
