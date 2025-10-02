#include <SD.h>
#include <SPI.h>
#include <DHT.h>
#include <DHT_U.h>

#define sensor 7
#define SSpin 10

File archivo;
DHT dht(sensor, DHT11);

int i = 1;
float pv1 = 0, pv2 = 0, pv3 = 0;
float prt = 0, prh = 0;

void setup() {
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  Serial.begin(9600);

  dht.begin();

  Serial.println("Iniciando SD");
  if (!SD.begin(SSpin)) {
    Serial.println("Fallo SD");
    return;
  }
}

void loop() {
  for (int j = 1; j <= 180; j++) {
    float valor = analogRead(A0);
    float valor2 = analogRead(A1);
    float valor3 = analogRead(A2);

    float V = valor * (5.0 / 1023);
    float o = (V * 330) / (5.0 - V);
    float V2 = valor2 * (5.0 / 1023);
    float o2 = (V2 * 1000) / (5.0 - V2);
    float V3 = valor3 * (5.0 / 1023);
    float o3 = (V3 * 10000) / (5.0 - V3);

    if (o > 10000000) {
      o = 10000000;
    }

    if (o2 > 10000000) {
      o2 = 10000000;
    }

    if (o3 > 10000000) {
      o3 = 10000000;
    }

    float Vout = (5.0 * o) / (o + 330);
    float Vout2 = (5.0 * o2) / (o2 + 1000);
    float Vout3 = (5.0 * o3) / (o3 + 10000);
    
    float tmp = dht.readTemperature();
    float hum = dht.readHumidity();

    pv1 = pv1 + Vout;
    pv2 = pv2 + Vout2;
    pv3 = pv3 + Vout3;
    prt = prt + tmp;
    prh = prh + hum;

    delay(1000);
  }

  if (SD.begin(SSpin)) {
    archivo = SD.open("PRUEBA2.csv", FILE_WRITE);

    if (archivo) {

      float rv1 = pv1 / 180;
      float rv2 = pv2 / 180;
      float rv3 = pv3 / 180;
      float rvt = prt / 180;
      float rvh = prh / 180;

      float v1 = 5.0 - rv1;
      float v2 = 5.0 - rv2;
      float v3 = 5.0 - rv3;

      archivo.print(i);
      archivo.print(",");
      archivo.print(v1, 4);
      archivo.print(",");
      archivo.print(v2, 4);
      archivo.print(",");
      archivo.print(v3, 4);
      archivo.print(",");
      archivo.print(rvt);
      archivo.print(",");
      archivo.println(rvh);

      archivo.close();
      
      i++;

      Serial.print(" V(330): ");
      Serial.print(v1, 4);
      Serial.print("  V(1K): ");
      Serial.print(v2, 4);
      Serial.print("  V(10K): ");
      Serial.println(v3, 4);
      Serial.print(" Temperatura: ");
      Serial.print(rvt);
      Serial.print("°C   Humedad: ");
      Serial.print(rvh);
      Serial.println("%");

      pv1 = 0;
      pv2 = 0;
      pv3 = 0;
      prt = 0;
      prh = 0;

      rv1 = 0;
      rv2 = 0;
      rv3 = 0;
      rvt = 0;
      rvh = 0;

    } else {
      Serial.println("no se encuentra el archivo");
      delay(1000);
    }
  } else {
    Serial.println("No se encuentra la tarjeta SD");
    delay(1000);
  }

  for (int k = 1; k <= 57; k++) {
    delay(60000);
    Serial.print("Minuto: ");
    Serial.println(k);
  }
}