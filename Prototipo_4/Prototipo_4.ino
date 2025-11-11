#include <RTClib.h>
#include <Wire.h>

RTC_DS3231 rtc;

const int Rf = 6;
const int Gf = 5;
const int Bf = 3;

const int Ru = 11;
const int Gu = 10;
const int Bu = 9;

float uv;
int UVOUT = A3;
int REF_3V3 = A1;

//------------------- Parámetros de la RNA -------------------//

double w_Capa_oculta1[2][5] = {
  { 2.8060021, -1.1711409, 4.122045, 2.350187, 0.81597537 },
  { 2.3484783, 2.2630205, -3.2768, 2.1375208, -2.6784663 }
};

double b_Capa_oculta1[5] = {
  -1.4936172, -1.3820752, -0.48103565, -1.0889859, 2.4271436
};

double w_Capa_oculta2[5][3] = {
  { 2.4358888, -1.8404518, -1.6751294 },
  { 1.1724445, -0.6933083, -1.5696467 },
  { -2.957599, 1.1990123, 1.3012772 },
  { 1.4084017, -1.1040592, -1.2870497 },
  { -2.26612, 2.7496896, -0.6572841 }
};

double b_Capa_oculta2[3] = { -0.9334416, 0.62091994, -0.20813814 };
double w_Capa_salida[3] = { 2.4138823, -1.6681055, -0.36259282 };
double b_Capa_salida = 0.6181946;

double mediaVout = 2.2540182926829266;
double mediaHora = 755.0;
double mediaUV = 2.4275609756097563;

double desviacionVout = 1.0734817809845172;
double desviacionHora = 238.15261213488014;
double desviacionUV = 1.593268680677765;

//------------------- Funciones de activación -------------------//

double sigmoid(double x) {
  return 1.0 / (1.0 + exp(-x));
}

double linear(double x) {
  return x;
}

//---------------------------------------------------------------//
void setup() {
  Serial.begin(9600);
  rtc.begin();

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  DateTime now = rtc.now();
  Serial.print(now.year(), DEC); Serial.print('/');
  Serial.print(now.month(), DEC); Serial.print('/');
  Serial.print(now.day(), DEC); Serial.print(" ");
  Serial.print(now.hour(), DEC); Serial.print(':');
  Serial.print(now.minute(), DEC); Serial.print(':');
  Serial.println(now.second(), DEC);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(Rf, OUTPUT);
  pinMode(Gf, OUTPUT);
  pinMode(Bf, OUTPUT);
  pinMode(Ru, OUTPUT);
  pinMode(Gu, OUTPUT);
  pinMode(Bu, OUTPUT);
}

void loop() {
  DateTime fecha = rtc.now();
  float valor = analogRead(A0);

  int hora = fecha.hour();
  int minutos = fecha.minute();
  int HoraTotal = hora * 60 + minutos;

  double vout330 = 5.0 - (valor * 5.0 / 1023.0); 

  double horaEstandar = (HoraTotal - mediaHora) / desviacionHora;
  double VoutEstandar = (vout330 - mediaVout) / desviacionVout;
  double indexUV = IndiceUV(horaEstandar, VoutEstandar);
  double indiceUVDes = indexUV * desviacionUV + mediaUV;

  
  // Sensor UV ML8511
  float uvLevel = analogRead(UVOUT);
  float refLevel = analogRead(REF_3V3);
  float outputVoltage = uvLevel * 3.3 / refLevel;
  float uvIntensity = mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0);
  uv = uvIntensity;
  
  if (uv < 0) {
    uv = 0;
  }

  // LED RNA
  if (indiceUVDes < 2.0) {
    setColor(255-0, 255-255, 255-0); // Verde
  } else if (indiceUVDes < 6.0) {
    setColor(255-255, 255-255, 255-0); // Amarillo
  } else if (indiceUVDes < 8.0) {
    setColor(255-255, 255-165, 255-0); // Naranja
  } else if (indiceUVDes < 11.0) {
    setColor(255-255, 255-0, 255-0); // Rojo
  } else {
    setColor(255-128, 255-0, 255-128); // Morado
  }

  if (uv < 2.0) {
    setColorUV(255-0, 255-255, 255-0); // Verde
  } else if (uv < 6.0) {
    setColorUV(255-255, 255-255, 255-0); // Amarillo
  } else if (uv < 8.0) {
    setColorUV(255-255, 255-165, 255-0); // Naranja
  } else if (uv < 11.0) {
    setColorUV(255-255, 255-0, 255-0); // Rojo
  } else {
    setColorUV(255-128, 255-0, 255-128); // Morado
  }

  Serial.print("Hora: ");
  Serial.print(hora); Serial.print(':'); Serial.print(minutos);
  Serial.print(" | Índice UV (RNA): "); Serial.print(indiceUVDes);
  Serial.print(" | Índice UV (ML8511): ");
  Serial.println(uv, 2);


  /*Serial.println("");
  Serial.println(valor);
  Serial.println(vout330);
  Serial.println(outputVoltage);*/

  delay(32000);
}

void setColorUV(int red, int green, int blue) {
  analogWrite(Ru, red);
  analogWrite(Gu, green);
  analogWrite(Bu, blue);
}

void setColor(int red, int green, int blue) {
  analogWrite(Rf, red);
  analogWrite(Gf, green);
  analogWrite(Bf, blue);
}

double IndiceUV(double a, double b) {
  double salida_oculta1[5];
  double salida_oculta2[3];
  double sum = 0;

  for (int i = 0; i < 5; i++) {
    salida_oculta1[i] = a * w_Capa_oculta1[0][i] + b * w_Capa_oculta1[1][i] + b_Capa_oculta1[i];
    salida_oculta1[i] = sigmoid(salida_oculta1[i]);
  }

  for (int i = 0; i < 3; i++) {
    salida_oculta2[i] = 0;
    for (int j = 0; j < 5; j++) {
      salida_oculta2[i] += salida_oculta1[j] * w_Capa_oculta2[j][i];
    }
    salida_oculta2[i] += b_Capa_oculta2[i];
    salida_oculta2[i] = sigmoid(salida_oculta2[i]);
  }

  for (int i = 0; i < 3; i++) {
    sum += salida_oculta2[i] * w_Capa_salida[i];
  }

  return linear(sum + b_Capa_salida);
}

int averageAnalogRead(int pinToRead) {
  byte numberOfReadings = 8;
  unsigned int runningValue = 0;
  for (int x = 0; x < numberOfReadings; x++)
    runningValue += analogRead(pinToRead);
  return runningValue / numberOfReadings;
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
