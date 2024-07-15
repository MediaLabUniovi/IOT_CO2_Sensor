/* =================================================================================
 * == Código que envía datos de un sensor a la base de datos de MediaLab en IONOS ==
 * =================================================================================
 *
 * Descripción:
 * Toma datos de CO2 en ppm, temperatura en ºC y humedad relativa en %
 * Código válido para sensor v2.0 (pcb v1.0) con tira led de 18 LED's
 * La alerta sonora viene activada en este caso, ASC desactivado
 *
 * ==================================
 * ==     Versión 5.0 del código   ==
 * ==   César Silva y  Juan Boto   ==
 * ==               MediaLab, 2022 ==
 * ==================================
 */


// Librerías de sensor y leds
#include <Adafruit_SCD30.h>    // Para sensor SCD30 (CO2, T y HR)
#include <Adafruit_NeoPixel.h> // Para la tira LED

// Librerias watchdog
#include <esp_task_wdt.h>

// Librerías Wifi y TCP si se utiliza ESP32
#include <WiFi.h>
#include <AsyncTCP.h>

//#include <ESP8266WiFi.h>
//#include <ESPAsyncTCP.h>

// Librerías sincronización y cliente HTTP
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>

// Librería para calcular la mediana de las medidas tomadas 
#include "QuickMedianLib.h"

// Archivo header con las credenciales SSID y password de la red Wifi
#include "credentials.h"

// Defines con parametros constantes
#define PIN 19       // pin de salida de control neopixel
#define NUMPIXELS 18 // número de píxeles
#define WDT_TIMEOUT 200 // Temporizador Watchdog


Adafruit_SCD30 scd30;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

HTTPClient http;

// variables para el cálculo de la mediana de medidas tomadas en 30 segundos
int valoresMediana[15] = {500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500};
int valoresMedianaLength = sizeof(valoresMediana) / sizeof(int);
int iteracionMediana = 0;
int mediana;

const int small_delay = 50;
int wifiSetupAttemp = 0;

int encender_led = 1; // Esta variable permite poner al neopixel en on/off
int buzzer_on = 1;    // Esta variable permite poner al zumbador en on/off


// Variables de temporización

unsigned long previousTime_0 = 0;
const unsigned long inverval_0 = 200; // 200 ms

unsigned long previousTime_1 = 0;
const unsigned long inverval_1 = 2000; // 2 segundos

unsigned long previousTime_2 = 0;
const unsigned long inverval_2 = 140000; // 140 segundos

// Variables auxiliares, modo, sonar...

int sonar = 0; // sonar 0,1,2

int co2 = 0, modo = 0;  // co2: guarda el valor actual de co2, modo: cambia el color de los leds
int co2_ant = 0, modo_antes = 0; // Guarda el estado anterior de las variables

int hum = 0;
float temper = 0;

int contadorMedianas = 0; // Contador de medianas tomadas

int contadorPitidos1 = 0; // un pitido
int contadorPitidos2 = 0; // pitido doble

int emisor = 18; // definir el pin digital del esp


/*
 * =======================
 * ==     FUNCIONES     ==
 * =======================
*/

/* == CAMBIAR COLOR LEDS ==*/
// Cambia el color dependiendo de 'modo': verde, ambar y rojo; dependiendo de la cantidad de CO2 en el aire
// Hay dos modos extra, azul para conexión wifi si se desconecta y rojo en zigzag por si no se lee
// el sensor.

void change_led_colors(int modo)
{

    // Declaracion array color leds
  int a_r[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int a_g[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int a_b[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  if (encender_led == 1) // Si la variable de control on/off esta en on(1)
  {

    switch (modo)
    {
    case 0: // Verde
    
      a_g[12] = 255;
      // a_g[13] = 255;
      a_g[14] = 255;
      a_g[15] = 255;
      // a_g[16] = 255;
      a_g[17] = 255;

      break;
      
    case 1: // Ambar
    
      a_g[6] = 165;
      // a_g[7] = 165;
      a_g[8] = 165;
      a_g[9] = 165;
      // a_g[10] = 165;
      a_g[11] = 165;

      a_r[6] = 255;
      // a_r[7] = 255;
      a_r[8] = 255;
      a_r[9] = 255;
      // a_r[10] = 255;
      a_r[11] = 255;

      break;
      
    case 2: // Rojo
    
      a_r[0] = 255;
      // a_r[1] = 255;
      a_r[2] = 255;
      a_r[3] = 255;
      // a_r[4] = 255;
      a_r[5] = 255;
      
      break;

    case 3: // Azul

      a_b[0] = 255;
      // a_r[1] = 255;
      a_b[2] = 255;
      a_b[3] = 255;
      // a_r[4] = 255;
      a_b[5] = 255;
      
      break;

    case 4: // rojo zigzag

      a_r[0] = 255;
      a_r[6] = 255;
      a_r[12] = 255;

      break;

    default:
      break;
    }

    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    pixels.show();

    for (int i = 0; i < 18; i++)
    {
      pixels.setPixelColor(i, pixels.Color(a_r[i], a_g[i], a_b[i]));
    }

    pixels.show();
  }
}

/* == CONEXIÓN A WIFI ==*/
// Función que conecta a una red Wifi dandole las credenciales, que se encontraran en credentials.h

void wifiSetup()
{
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    wifiSetupAttemp++;
    Serial.println("Conectando al WiFi...");
    if (wifiSetupAttemp >= 20)
    {
      wifiSetupAttemp = 0;
      Serial.println("Conectando al WiFi...");
      //change_led_colors(3);
      break;
    }
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("No se ha podido conectar al Wifi, se intentará de nuevo en unos segundos");
  }
  else
  {
    Serial.println("¡Conectado a Wifi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

/* == LEER DATOS DEL SENSOR ==*/
// Función que lee los datos del sensor SCD30, mete los valores en el array de la mediana
// cambia de modo (color) en consecuencia y activa el pitido si ha cambiado de modo y procede.

void leer()
{
  co2_ant = co2;

  if (scd30.dataReady())
  {
    co2 = scd30.CO2;
    valoresMediana[iteracionMediana] = co2;
    iteracionMediana++;
    hum = scd30.relative_humidity;
    temper = scd30.temperature;

    if (!scd30.read())
    {
      Serial.println("ERROR leyendo sensor...");
      return;
    }
    delay(500);

    if (mediana < 700)
      modo = 0;
    else if ((mediana >= 700) && (mediana < 1000))
      modo = 1;
    else
      modo = 2;

    if ((modo_antes == 0) && (modo == 1))
      sonar = 1;

    if ((modo_antes == 1) && (modo == 2))
      sonar = 2;

    if ((modo_antes == 0) && (modo == 2))
      sonar = 2;

    modo_antes = modo;

    change_led_colors(modo);

    //ledcWrite(ledChannel_buzzer, dutyCycle_buzzer);
    //pitido();

    if (iteracionMediana >= 15) // cuando he guardado 15 datos
      almacenar();

    Serial.print(co2);
    Serial.print(",");
    Serial.print(hum);
    Serial.print(",");
    Serial.print(temper);
    Serial.print(",");
    Serial.print(WiFi.macAddress());
    Serial.print(",");
    Serial.println();
    //Serial.print(modo);
    //Serial.print(",");
    //Serial.print(co2_ant);
    //Serial.println();
  }
}

/* == CALCULAE MEDIANA Y SUBIR VALOR A LA BASE DE DATOS ==*/
// Se ejecuta al llegar a 15 valores guardados en el array y calcula la mediana de los 15 valores
// Encía el valor de la mediana y comprueba que haya llegado a la base de datos
// Por otro lado, si dos medianas son menores que 360 ppm durante 360 s, recalibra el sensor

void almacenar()
{

  delay(small_delay);

  mediana = QuickMedian<int>::GetMedian(valoresMediana, valoresMedianaLength);

  String serverPath = "https://www.medialab-uniovi.es/post_sensorCO2.php?MAC_sensor='" + WiFi.macAddress() + "'&co2=" + mediana + "&humedad=" + hum + "&temp=" + temper;
  http.begin(serverPath.c_str());
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    Serial.print(" Descripcion: Error subiendo datos a la base de datos ");
    Serial.println();
  }
  http.end();

  delay(small_delay);

  iteracionMediana = 0;

  if (mediana < 360)
  {
    contadorMedianas++;
  }

  if (contadorMedianas == 2)
  {
    delay(2000);
    contadorMedianas = 0;
    Serial.println("Forzando calibración debido a datos co2 < 360 ppm");
    scd30.forceRecalibrationWithReference(415);
    delay(2000);
    Serial.println("Comprobando calibración automática: ");
    if (scd30.selfCalibrationEnabled())
    {
      Serial.print("Calibración automática (ON)");
    }
    else
    {
      Serial.print("Calibración automática (OFF)");
    }
  }
}

/* == EMITIR PITIDO  ==*/
// Si el zumbador está activo, se emite pitido o pitido doble al bajar la calidad de aire
// Solo puede pitar dos veces por cada 140 segundos para no molestar demasiado

void pitido()
{

  if (buzzer_on == 1)
  {
    if (sonar == 2 && contadorPitidos2 <= 1) // varios pitidos
    {
      contadorPitidos2++;
   pinMode(emisor, OUTPUT); // definir el pin como salida 
   for (int i = 0; i < 70; i++) { // emitir un sonido
   digitalWrite(emisor, HIGH); // salida en ALTO
   delay(1); // espera 1 ms
   digitalWrite(emisor, LOW); // salida en BAJO
   delay(1);
   }
   delay(50);
   
   pinMode(emisor, OUTPUT); // definir el pin como salida 
   for (int i = 0; i < 70; i++) { // emitir un sonido
   digitalWrite(emisor, HIGH); // salida en ALTO
   delay(1); // espera 1 ms
   digitalWrite(emisor, LOW); // salida en BAJO
   delay(1);
   }
   digitalWrite(emisor, LOW); // salida en BAJO
   sonar=0;
    }
    
    else if (sonar == 1 && contadorPitidos1 <= 1) // un pitido
    {
      contadorPitidos1++;
        pinMode(emisor, OUTPUT); // definir el pin como salida 
   for (int i = 0; i < 70; i++) { // emitir un sonido
   digitalWrite(emisor, HIGH); // salida en ALTO
   delay(1); // espera 1 ms
   digitalWrite(emisor, LOW); // salida en BAJO
   delay(1);
   }
   digitalWrite(emisor, LOW); // salida en BAJO
   sonar=0;
    }
    
  }
}

/* ========================*/


/*
 * ===========================
 * ==     CONF. INICIAL     ==
 * ===========================
*/

void setup()
{
  // CONFIGURACIÓN SERIAL, WIFI, LEDS
  
  Serial.begin(115200);
  wifiSetup();

  Serial.println("Configurando Watchdog...");
  esp_task_wdt_init(WDT_TIMEOUT, true); // enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);               // add current thread to WDT watch
  Serial.println("Watchdog configurado con éxito");

  pixels.begin();
  pixels.setBrightness(200);
  //Wire.begin();
  
  delay(small_delay);

  Serial.println("Iniciando prueba LED's");
  change_led_colors(0); // initial led colors
  delay(500);
  change_led_colors(1);
  delay(500);
  change_led_colors(2);
  delay(1000);
  change_led_colors(0);
  delay(200);
  Serial.println("Prueba LED's completa");


  // INICIO SENSOR SCD30

  Serial.println("Iniciando el sensor SCD30...");
  scd30.begin();
  // Intentar inicializar SCD30
  if (!scd30.begin())
  {
    Serial.println("No se ha encontrado SCD30, revisar conexión");
    change_led_colors(4);
    while (1)
    {
      delay(3000);
      scd30.begin();
      delay(1000);
      if (scd30.begin()== true)  {
        Serial.print("SCD30 Iniciado. ");
        delay(1000);
        break;
      }
        
    }
  }

  if (scd30.begin() == true)
    Serial.print("SCD30 Iniciado. ");

  // CONFIGURACIÓN BÁSICA DEL SCD30

  /* Ajustar tiempo entre medidas, rango 2-1800 segundos */
  // scd30.setMeasurementInterval(2);
  Serial.print("Intervalo de medidas: ");
  Serial.print(scd30.getMeasurementInterval());
  Serial.println(" segundos");

  /* Señalar altura a la que se coloca el sensor respecto del nivel del mar
  Gijón a unos 20m
  */
  scd30.setAltitudeOffset(20);
  Serial.print("Altura a nivel del mar: ");
  Serial.print(scd30.getAltitudeOffset());
  Serial.println(" metros");

  

  Serial.println("\n\n");

  /* Forzar calibración */
  //scd30.forceRecalibrationWithReference(750);
  //Serial.print("Referencia de caliración: ");
  //Serial.print(scd30.getForcedCalibrationReference());
  //Serial.println(" ppm");

  /* Activar ASC (Calibración automática)*/
  scd30.selfCalibrationEnabled(false);

  if (scd30.selfCalibrationEnabled())
    Serial.print("Calibración automática (ON)");
  else
    Serial.println("Calibración automática (OFF)");

  
  scd30.setTemperatureOffset(500);
  
  // REINICIO DEL WATCHDOG
  esp_task_wdt_reset();
  Serial.println("Watchdog puesto a 0.");

  Serial.println("\n\n");
  Serial.println("Configuración inicial COMPLETA CON EXITO");
  Serial.println("\n\n");
}

/* ========================*/


/*
 * ============================
 * ==     BUCLE PRINCIPAL    ==
 * ============================
*/


void loop()
{

  if (millis() - previousTime_0 >= inverval_0) // cada 200 ms
  {
    previousTime_0 = millis();
    pitido();
  }

  if (millis() - previousTime_1 >= inverval_1) // cada 2 seg
  {
    previousTime_1 = millis();
    leer();
    if (co2 != co2_ant)
      esp_task_wdt_reset();
      delay(small_delay);
  }

  if (millis() - previousTime_2 >= inverval_2) // cada 140 seg
  {
    previousTime_2 = millis();
    if (WiFi.status() != WL_CONNECTED)
    {
      wifiSetup();
    }
    contadorPitidos1 = 0;
    contadorPitidos2 = 0;
    contadorMedianas = 0;
  }
}

/* ========================*/
