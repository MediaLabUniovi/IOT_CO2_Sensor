/* =================================================================================
 * == Código que envía datos de un sensor a la base de datos de MediaLab en IONOS ==
 * =================================================================================
 * 
 * Descripción:
 * Toma datos de CO2 en ppm, temperatura en ºC y humedad relativa en %
 * Código válido para sensor v2.0 (pcb v1.0) con tira led de 18 LED's
 * La alerta sonora viene desactivada en este caso
 * La alerta sonora viene activada
 * 
 * ==================================
 * ==     Versión 2.0 del código   ==
 * ==   Juan Boto,  César Silva,   ==
 * == Simón López y Andrea Salgado ==
 * ==               MediaLab, 2021 ==
 * ==================================
 */



// ====================================
// == Librerías y defines necesarios ==
// ====================================

// Librerías de sensores y memoria
#include <EEPROM.h>                         // Memoria EEPROM
#include <Adafruit_SCD30.h>                 // Para sensor SCD30 (CO2, T y HR)
#include <Adafruit_NeoPixel.h>              // Para la tira LED
#include "esp_wpa2.h"

// Librerías Wifi y TCP si se utiliza ESP32 o versión antigua ESP8266
#include <WiFi.h>
#include <AsyncTCP.h>

// Librerías sincronización y cliente HTTP
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>

// Archivo header con las credenciales SSID y password de la red Wifi
#include "credentials.h"

// Librería para calcular la mediana de las medidas tomadas para su envío a la base de datos
#include "QuickMedianLib.h"

// Defines con parametros constantes
#define EEPROM_SIZE 1  // numero de bits a los que se requiere acceso
#define PIN 19         // pin de salida de control neopixel 
#define NUMPIXELS 18   // número de píxeles


// ======================
// == Inicio librerías ==
// ======================

// Llamada a la estructura de la librería de control del sensor
Adafruit_SCD30  scd30;

// LLamada a la estructura de la tira LED
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// LLamada a la estructura HTTP
HTTPClient http;


// ===============
// == Variables ==
// ===============
//variables para el cálculo de la mediana de medidas tomadas en 30 segundos
int valoresMediana[15]={500,500,500,500,500,500,500,500,500,500,500,500,500,500,500};
int valoresMedianaLength = sizeof(valoresMediana) / sizeof(int);
int iteracionMediana=0;
int mediana;

const int total_tam = 5000, tam_res = 154; 
char total[total_tam];
char temp[50];

int encender_led = 1; // Esta variable permite poner al neopixel en on/off

bool buzzer_on = 0;                                                           // Esta variable permite poner al zumbador en on/off
uint16_t dutyCycle_buzzer = 0;                                                // con duty cycle 0, el zumbador está silenciado
const int freq = 5000, ledChannel_buzzer = 0, resolution = 10, piezoPin = 18; // frecuencia del zumbador, canal, resolucion y output data pin

// Variables de temporización

unsigned long currentTime = millis(); // Usando la librería 'time' los intervalos enviados son controlados
unsigned long previousTime_0 = 0;
const long inverval_0 = 200;

unsigned long previousTime_1 = 0;
const long inverval_1 = 2000;

unsigned long previousTime_2 = 0;
const long inverval_2 = 1000 * 30;

const int buzzer_sequence_tam = 6;
int buzzer_sequence [buzzer_sequence_tam] = {500,500,0,500,0,500};

// Variables auxiliares, modo, sonar...

int aux = 0; // aux para pitido
int sonar = 0; // sonar 0,1,2 

int co2 = 0, modo = 0, modo_antes=0; //co2: guarda el valor actual de co2, modo: cambia el color de los leds activando diferentes modos

int hum = 0;
float media = 0, temper = 0;

/* ======================================================*/

// ===============
// == Funciones ==
// ===============

/* ======================= Función Modo y cambio de modo ===========================*/
// Cambia entre 3 modos de color: verde, ambar y rojo; dependiendo de la cantidad de CO2 en el aire
void cambio(int modo){

  if (NUMPIXELS == 18){

  int a_r[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int a_g[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int a_b[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  if (encender_led == 1) // Si la variable de control on/off esta en on(1)
  {
    switch (modo)
    {
    case 0: // Verde
      a_g[12] = 255;
      a_g[13] = 255;
      a_g[14] = 255;
      a_g[15] = 255;
      a_g[16] = 255;
      a_g[17] = 255;
      
      break;
    case 1: // Ambar
      a_g[6] = 165;
      a_g[7] = 165;
      a_g[8] = 165;
      a_g[9] = 165;
      a_g[10] = 165;
      a_g[11] = 165;

      a_r[6] = 255;
      a_r[7] = 255;
      a_r[8] = 255;
      a_r[9] = 255;
      a_r[10] = 255;
      a_r[11] = 255;

      
      break;
    case 2: //Rojo 
      a_r[0] = 255;
      a_r[1] = 255;
      a_r[2] = 255;
      a_r[3] = 255;
      a_r[4] = 255;
      a_r[5] = 255;

      break;
    default:
      break;
    }
  }

  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.show();

  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(a_r[i], a_g[i], a_b[i]));
  }

  delay(1);
  pixels.show(); }

  else {

  int a_r[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  int a_g[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  int a_b[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

  if (encender_led == 1) //neopixels on are available
  {
    switch (modo)
    {
    case 0: //verde
      a_g[6] = 255;
      a_g[7] = 255;
      a_g[8] = 255;
      
      break;
    case 1: //naraja
      a_g[3] = 165;
      a_g[4] = 165;
      a_g[5] = 165;

      a_r[3] = 255;
      a_r[4] = 255;
      a_r[5] = 255;
      
      break;
    case 2: //rojo
      a_r[0] = 255;
      a_r[1] = 255;
      a_r[2] = 255;

      break;
    default:
      break;
    }
  }

  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.show();

  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(a_r[i], a_g[i], a_b[i]));
  }

  delay(1);
  pixels.show();

  }
}
/* ======================================================*/


/* ======================= Función configuración WIFI ==========================*/
// Función que conecta a una red Wifi dandole las credenciales, que se encontraran en credentials.h

void wifiSetup()
{
  WiFi.disconnect(true); 
  WiFi.mode(WIFI_STA);
  //esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY)); //provide identity
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY)); //provide username --> identity and username is same
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD)); //provide password
  esp_wifi_sta_wpa2_ent_enable(); //set config settings to enable function
  WiFi.begin(ssid); //connect to wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conectando al WiFi...");
    Serial.println(EAP_IDENTITY);
  }
 
  Serial.println("¡Conectado a Wifi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

/* ======================================================*/


/* ======================= Función leer datos del SCD30 ==========================*/
void leer()
{
   if (scd30.dataReady()) {
      //Variables para guardar los datos del sensor
      co2 = scd30.CO2;
      valoresMediana[iteracionMediana] = co2;
      iteracionMediana++;
      hum = scd30.relative_humidity;
      temper = scd30.temperature;
      
      if (!scd30.read()){ 
        Serial.println("ERROR leyendo sensor..."); 
        return; 
   }
  delay(500);

  if (co2 < 700)
    modo = 0;
  else if ((co2 >= 700) && (co2 < 1000))
    modo = 1;
  else
    modo = 2;
  
  if ((modo_antes == 0) && (modo == 1)) 
  {
    sonar = 1;    
  }
  
  if ((modo_antes == 1) && (modo == 2)) 
  {
    sonar = 2;    
  }


  if ((modo_antes == 0) && (modo == 2)) 
  {
    sonar = 2;    
  }



  modo_antes = modo;
  cambio(modo);
  ledcWrite(ledChannel_buzzer, dutyCycle_buzzer);

  Serial.print(co2);
  Serial.print(",");
  Serial.print(hum);
  Serial.print(",");
  Serial.print(temper);
  Serial.print(",");
  Serial.print(WiFi.macAddress());
  Serial.println();
  }
}


/* ======================= Función almacenar datos en la base de datos ==========================*/
void almacenar(){ 

    mediana= QuickMedian<int>::GetMedian(valoresMediana, valoresMedianaLength);

    String serverPath = "https://www.medialab-uniovi.es/post_sensorCO2.php?MAC_sensor='"+WiFi.macAddress()+"'&co2="+mediana+"&humedad="+hum+"&temp="+temper;
    http.begin(serverPath.c_str());
    int httpResponseCode = http.GET();
    
    if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      }
    else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
    http.end();
    
    iteracionMediana=0;

    // Limitación de ppm inferior. No puede haber menos de 400 ppm, utilizamos 360 por el error intrinseco
    if (mediana < 360)
    {
        delay(2000);
        Serial.println("Forzando calibración debido a datos co2 < 360 ppm");
        scd30.forceRecalibrationWithReference(410);
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
/* ======================================================*/

/* ======================= Función emitir pitidos ==========================*/
void pitido()
{
    if (sonar == 2) //varios pitidos
    {
      Serial.println("Emitiendo pitido doble... (co2>1000)");
      dutyCycle_buzzer = buzzer_sequence[aux];
      aux = aux + 1;
      if (aux >= buzzer_sequence_tam)
      {
        aux = 0;
        sonar = 0;
      }
    }
    else if (sonar == 1) //un pitido
    {
      Serial.println("Emitiendo un solo pitido... (co2>700)");
      dutyCycle_buzzer = 500;
      sonar = 0;
    }
    else dutyCycle_buzzer = 0;
    ledcWrite(ledChannel_buzzer, dutyCycle_buzzer);
}
/* ======================================================*/


// ========================
// == Programa principal ==
// ========================

/* === CONFIGURACIÓN INICIAL === */

void setup() {
  Serial.begin(115200);
  wifiSetup();
  pixels.begin();
  Wire.begin();
  EEPROM.begin(EEPROM_SIZE);

  Serial.println("Iniciando prueba LED's");
  cambio(0); //initial led colors
  delay(200);
  cambio(1);
  delay(200);
  cambio(2);
  delay(400);
  Serial.println("Prueba LED's completa");

  buzzer_on = EEPROM.read(0); //Se lee el ultimo valor on/off del zumbador antes de que el micro se apagara

  ledcSetup(ledChannel_buzzer, freq, resolution); // Confuguración de las funcionalidades LED PWM
  ledcAttachPin(piezoPin, ledChannel_buzzer);     // Vincular el canal GPIO para ser controlado

  
   Serial.println("Iniciando el sensor SCD30...");
  // Intentar inicializar SCD30
  if (!scd30.begin()) {
    Serial.println("No se ha encontrado SCD30, revisar conexión");
    while (1) { delay(10); }
  }

  // CONFIGURACIÓN BÁSICA DEL SCD30
  
  /* Ajustar tiempo entre medidas, rango 2-1800 segundos */
  //scd30.setMeasurementInterval(2);
  Serial.print("Intervalo de medidas: ");
  Serial.print(scd30.getMeasurementInterval());
  Serial.println(" segundos");


  /* Señalar altura a la que se coloca el sensor respecto del nivel del mar
  Gijón a unos 20m*/
  scd30.setAltitudeOffset(20);
  Serial.print("Altura a nivel del mar: ");
  Serial.print(scd30.getAltitudeOffset());
  Serial.println(" metros");
  
  Serial.println("\n\n");

  /* Forzar calibración */
  //scd30.forceRecalibrationWithReference(550);
  Serial.print("Referencia de caliración: ");
  Serial.print(scd30.getForcedCalibrationReference());
  Serial.println(" ppm");
  
  /* Activar ASC (Calibración automática)*/ 
  scd30.selfCalibrationEnabled(true);
  if (scd30.selfCalibrationEnabled())
  {
    Serial.print("Calibración automática (ON)");
  }
  else
  {
    Serial.print("Calibración automática (OFF)");
  }

  Serial.println("\n\n");
  
  Serial.println("Configuración inicial COMPLETA CON EXITO");
  Serial.println("\n\n");
}


/* ======================================================*/


/* === BUCLE PRINCIPAL === */

void loop() {
  currentTime = millis();

  if (currentTime - previousTime_0 > inverval_0) //cada 200ms
  {
    previousTime_0 = currentTime;
    //pitido();
  }

  if (currentTime - previousTime_1 > inverval_1) //cada 2 seg
  {
    previousTime_1 = currentTime;
    leer();
  }

  if (currentTime - previousTime_2 > inverval_2) //cada 30 seg
  {
    
    previousTime_2 = currentTime;

    almacenar();
    if (WiFi.status() != WL_CONNECTED)
    {
      wifiSetup();
    }
  }

}
