#include <SD.h>
#include <SPI.h>
#include <DHT.h>
#include <Wire.h>
#include <RTClib.h>

#define sensor 8
#define SSpin 10

File archivo;
RTC_DS3231 rtc;
DHT dht(sensor, DHT22);

char bufferDatoFecha[19];
char bufferDatoHora[19];
int UVOUT = A2;
int REF_3V3 = A3;
int i = 0;

float pv1 = 0, puv = 0;
float prt = 0, prh = 0;
float rv1 = 0, rvt = 0, rvh = 0, ruv = 0;

void setup() {
  Wire.begin();
  rtc.begin();
  pinMode(A0, INPUT);
  pinMode(UVOUT, INPUT);
  pinMode(REF_3V3, INPUT);

  Serial.begin(9600);
  dht.begin();

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  Serial.println("Iniciando SD");
  if (!SD.begin(SSpin)) {
    Serial.println("Fallo SD");
  }

  DateTime now = rtc.now();
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

  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
}

void loop() {
  if (SD.begin(SSpin)) {
    DateTime fecha = rtc.now();

    // Toma de datos cada 10 minutos
    if (fecha.minute() == 7 || fecha.minute() == 17 || fecha.minute() == 27 ||
        fecha.minute() == 37 || fecha.minute() == 47 || fecha.minute() == 57) {

      limpiar();

      for (int j = 1; j <= 180; j++) {
        float valor = analogRead(A0);
        float V = valor * (5.0 / 1023);
        float o = (V * 330) / (5.0 - V);

        if (o > 10000000) {
          o = 10000000;
        }

        float Vout = (5.0 * o) / (o + 330);

        float tmp = dht.readTemperature();
        float hum = dht.readHumidity();

        int uvLevel = averageAnalogRead(UVOUT);
        int refLevel = averageAnalogRead(REF_3V3);
        float outputVoltage = 3.33 / refLevel * uvLevel;

        float uvIntensity = mapfloat(outputVoltage, 0.98, 2.9, 0.0, 15.0);

        puv += uvIntensity;
        pv1 += Vout;
        prt += tmp;
        prh += hum;

        delay(1000);
      }

      rv1 = pv1 / 180;
      rvt = prt / 180;
      rvh = prh / 180;
      ruv = puv / 180;

      i = 1;

    } else {
      if (i == 1) {
        archivo = SD.open("PRUEBA3.csv", FILE_WRITE);

        if (archivo) {
          float valor = analogRead(A0);
          float V = valor * (5.0 / 1023);
          float o = (V * 330) / (5.0 - V);
          float Vout = (5.0 * o) / (o + 330);
          float v1 = 5.0 - rv1;

          sprintf(bufferDatoFecha, "%02d/%02d/%04d", fecha.day(), fecha.month(), fecha.year());
          sprintf(bufferDatoHora, "%02d:%02d", fecha.hour(), fecha.minute());

          Serial.println(bufferDatoFecha);
          Serial.println(bufferDatoHora);
          Serial.print(" V(330): ");
          Serial.println(Vout);
          Serial.print(" Temperatura: ");
          Serial.print(rvt);
          Serial.print("°C   Humedad: ");
          Serial.print(rvh);
          Serial.println("%");
          Serial.print(" Indice UV: ");
          Serial.println(round(ruv));

          archivo.print(bufferDatoFecha);
          archivo.print(",");
          archivo.print(bufferDatoHora);
          archivo.print(",");
          archivo.print(round(ruv));
          archivo.print(",");
          archivo.print(ruv, 2);
          archivo.print(",");
          archivo.print(v1, 4);
          archivo.print(",");
          archivo.print(rvt);
          archivo.print(",");
          archivo.println(rvh);

          archivo.close();

          semaforo();
          limpiar();
          i = 0;

        } else {
          Serial.println("No se encuentra el archivo");
        }
      }
    }
  } else {
    Serial.println("No se encuentra la tarjeta SD");
    delay(1000);
  }
}

void limpiar() {
  pv1 = 0;
  prt = 0;
  prh = 0;
  puv = 0;
  rv1 = 0;
  rvt = 0;
  rvh = 0;
  ruv = 0;
}

void semaforo() {
  if (ruv >= 0 && ruv < 1) {
    Serial.println("Nula");
    digitalWrite(7, HIGH);
    digitalWrite(6, LOW);
    digitalWrite(5, LOW);
    digitalWrite(4, LOW);
    digitalWrite(3, LOW);
    digitalWrite(2, LOW);
  } else if (ruv >= 1 && ruv < 3) {
    Serial.println("Baja");
    digitalWrite(7, LOW);
    digitalWrite(6, HIGH);
    digitalWrite(5, LOW);
    digitalWrite(4, LOW);
    digitalWrite(3, LOW);
    digitalWrite(2, LOW);
  } else if (ruv >= 3 && ruv < 6) {
    Serial.println("Moderada");
    digitalWrite(7, LOW);
    digitalWrite(6, LOW);
    digitalWrite(5, HIGH);
    digitalWrite(4, LOW);
    digitalWrite(3, LOW);
    digitalWrite(2, LOW);
  } else if (ruv >= 6 && ruv < 8) {
    Serial.println("Alta");
    digitalWrite(7, LOW);
    digitalWrite(6, LOW);
    digitalWrite(5, LOW);
    digitalWrite(4, HIGH);
    digitalWrite(3, LOW);
    digitalWrite(2, LOW);
  } else if (ruv >= 8 && ruv < 11) {
    Serial.println("Muy alta");
    digitalWrite(7, LOW);
    digitalWrite(6, LOW);
    digitalWrite(5, LOW);
    digitalWrite(4, LOW);
    digitalWrite(3, HIGH);
    digitalWrite(2, LOW);
  } else if (ruv >= 11) {
    Serial.println("Extremadamente alta");
    digitalWrite(7, LOW);
    digitalWrite(6, LOW);
    digitalWrite(5, LOW);
    digitalWrite(4, LOW);
    digitalWrite(3, LOW);
    digitalWrite(2, HIGH);
  }
}

int averageAnalogRead(int pinToRead) {
  byte numberOfReadings = 8;
  unsigned int runningValue = 0;

  for (int x = 0; x < numberOfReadings; x++)
    runningValue += analogRead(pinToRead);

  runningValue /= numberOfReadings;
  return (runningValue);
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
