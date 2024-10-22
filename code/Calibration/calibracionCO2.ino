
#include "SparkFun_SCD30_Arduino_Library.h"

#define LEDPIN 2 

SCD30 airSensor;

unsigned long TiempoAhora = 0;

#ifdef ALTERNATIVE_I2C_PINS
#define I2C_SDA 22
#define I2C_SCL 21
#endif

uint16_t calibrationValue = 415;


void calibrateSensor()
{
  printf("Calibrando a %d PPM\n", calibrationValue);
  airSensor.setForcedRecalibrationFactor(calibrationValue);
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

  // Set pin mode
  pinMode(LEDPIN,OUTPUT);

  // Initialize the SCD30 driver
  #ifdef ALTERNATIVE_I2C_PINS
    Wire.begin(I2C_SDA, I2C_SCL);
  #else
    Wire.begin();
  #endif
  if (airSensor.begin() == false)
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    while (1)
    ;
  }
  Serial.println("Ready.");
}

void loop()
{
  if(millis() > TiempoAhora + 300000){
        TiempoAhora = millis();
        calibrateSensor();
        Serial.println("Hecho.");
        while (1);
    }
}
