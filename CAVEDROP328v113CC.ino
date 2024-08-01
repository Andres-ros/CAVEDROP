// CAVEDROP v113CC
// Andrés Ros julio 2024
// Bajo licencia Creative Commons CC BY-NC-SA 4.0
// sistema para la detección de impactos, utilizado como contador de gotas en espeleotemas de cavidades
// Se puede programar por medio de interruptores el total de gotas por minutos, cada 10 minutos o 1 hora
// materiales: Atmega328PB, modulo reloj DS3231, Sensor piezoelectrico, OpenLog mini SD, DIP 4 , Baterias AA x3, resistencia 1M, led.
// El sensor piezoelectrico detecta los impactos que son procesados en el Atmega328PB generando un registro con fecha, hora:minuto, total gotas periodo tiempo seleccionado (1mn, 10mn, 1h).
// La sesiblidad del sensor piezoelectrico puede ser ajustada.
// Se incluye la libreria SoftwareSerial.h para desvincular los pines Xt y RX para no interferir entre OpenLog y cargas o revisiones del programa
// Un buffer almacena registros y los vuelca periodicamente al OpenLog.
// Mi agradecimiento a los trabajos de E.K. Mallon 

#include <Wire.h>
#include <RTClib.h>
#include <SoftwareSerial.h>

// Define los pines
#define LED_PIN 2       // Pin para el LED
#define piezoPin A1     // Pin para el sensor piezoeléctrico en un pin analógico
#define SWITCH_PIN_1 3  // Pin para el interruptor de 1 minuto
#define SWITCH_PIN_10 4 // Pin para el interruptor de 10 minutos
#define SWITCH_PIN_60 5 // Pin para el interruptor de 1 hora

#define BUFFER_SIZE 10  // Tamaño del buffer para almacenar registros de gotas

RTC_DS3231 rtc;  // Crea una instancia del RTC
SoftwareSerial mySerial(10, 11); // Configura SoftwareSerial en los pines 10 y 11 RX, TX

int gotasContador = 0;
unsigned long lastImpactTime = 0;
const unsigned long impactCooldown = 300; // Tiempo mínimo entre impactos para evitar falsos positivos
unsigned long lastCheckTime = 0;
unsigned long tiempoRegistro = 60000; // Tiempo de registro inicial en milisegundos (1 minuto)

char buffer[BUFFER_SIZE][64];
int bufferIndex = 0;

// Variables para el filtro de paso bajo
float filteredValue = 0;
const float alpha = 0.1;  // Coeficiente de suavizado para el filtro

// Variables para medir el voltaje, para mantner la hora aúnque se descarge la bateria principal
const int batteryPin = A0; // Pin analógico para medir el voltaje de la batería
const float referenceVoltage = 5.0; // Voltaje de referencia del Arduino
const float dividerRatio = 2.0; // Ratio del divisor de voltaje

void setup() {
  Serial.begin(9600); // Comunicación para monitoreo y OpenLog
  mySerial.begin(9600); // Inicia SoftwareSerial para otra comunicación si es necesario

  pinMode(piezoPin, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SWITCH_PIN_1, INPUT_PULLUP);
  pinMode(SWITCH_PIN_10, INPUT_PULLUP);
  pinMode(SWITCH_PIN_60, INPUT_PULLUP);

  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("No se pudo encontrar el módulo RTC");
    while (1); // Detiene el programa si no se encuentra el RTC
  }

  // Leer la hora actual del RTC al iniciar,revisa la hora para no perderla
  DateTime now = rtc.now();
  Serial.print("Fecha y hora actuales: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  // comentar esta linea una vez cargado el programa y registrada la hora
  // volver a cargar el programa una vez sincronizada la hora con la linea comentada, para evitar buscar la sincornizacion hora, cada vez que se reinicie
 rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Ajusta la hora del RTC al momento de la compilación

  delay(2000); // Esperar para asegurarse de que todo esté listo

  // Comandos para configurar el archivo de registro
  Serial.println("new RegistroGotas.txt"); // Crea un nuevo archivo si no existe
  delay(100);
  Serial.println("append RegistroGotas.txt"); // Añade datos al final del archivo
  delay(100);

  // Escribir la cabecera en el archivo CSV
  Serial.println("CaveDrop 328");
  delay(100);
  Serial.println("v113CC_002"); // se añade el numero de unidad para individualizar
  delay(100);
  Serial.println("Fecha, hora, gotas");
  delay(100);

  lastCheckTime = millis();
}

void loop() {
  int piezoValue = analogRead(piezoPin);
  filteredValue = alpha * piezoValue + (1 - alpha) * filteredValue; // Aplicar filtro de paso bajo

// ajuste sensibiidad piezolectrico
  if (filteredValue > 15 && millis() - lastImpactTime > impactCooldown) { 
    digitalWrite(LED_PIN, HIGH);
    gotasContador++;
    lastImpactTime = millis();
    delay(100);
    digitalWrite(LED_PIN, LOW);
  }

  // Verificar los estados de los interruptores y ajustar el tiempo de registro
  if (digitalRead(SWITCH_PIN_1) == LOW) {
    tiempoRegistro = 60000; // 1 minuto
  } else if (digitalRead(SWITCH_PIN_10) == LOW) {
    tiempoRegistro = 600000; // 10 minutos
  } else if (digitalRead(SWITCH_PIN_60) == LOW) {
    tiempoRegistro = 3600000; // 1 hora
  }

  if (millis() - lastCheckTime >= tiempoRegistro) {
    registrarGota(gotasContador);
    gotasContador = 0;
    lastCheckTime = millis();
    descargarBuffer();
  }
}

void registrarGota(int contador) {
  DateTime now = rtc.now();
  snprintf(buffer[bufferIndex], sizeof(buffer[bufferIndex]), "%04u-%02u-%02u,%02u:%02u,%d", 
           now.year(), now.month(), now.day(), now.hour(), now.minute(), contador);
  bufferIndex++;
  if (bufferIndex >= BUFFER_SIZE) {
    bufferIndex = 0; // Resetear el índice si se llena el buffer
  }
}

void descargarBuffer() {
  for (int i = 0; i < bufferIndex; i++) {
    Serial.println(buffer[i]); // Enviar al OpenLog y asegurar que cada registro esté en una nueva línea
  }
  bufferIndex = 0; // Reiniciar el índice del buffer después de descargar
}
