#include <BluetoothSerial.h>

///////////
//-------//
//MOTORES//
//-------//
///////////

#define ENABLEIZQ 7
#define ENABLEDER 8
#define motorDA 11
#define motorDB 9
#define motorIA 3
#define motorIB 10

/////////////////
//-------------//
//TIEMPOS Y ETC//
//-------------//
/////////////////

unsigned long tiempoInicio = 0; // 5 segundos
int led = 4;

//////////////
//----------//
//PARTE SUMO//
//----------//
//////////////

#define TRIGGERIZQ 6
#define TRIGGERDER 12
#define UMBRAL 1500
int sensorP1 = A2;
int sensorP2 = A1;
int sensorP3 = A3;
int valorP1;
int valorP2;
int valorP3;
int sharp1 = A5; // frente
int sharp2 = A6; // diagonal izquierda
int sharp3 = A7; // diagonal derecha
int ultra1 = A4; // frente izquierda 
int ultra2 = A0;  // frente derecha 
long d1, d2; // distancias medidas
int faseReubicar = 0; // 0 = inactivo, 1 = retrocediendo, 2 = girando
int faseBuscar = 0;  // 0 = avanzar normal / 1 = giro completo

      ///////////////////////////////
      // F I N   V A R I A B L E S //
      ///////////////////////////////

////////////////
//------------//
//PARPADEO LED//
//------------//
////////////////

void tiempoSeguridad(){
  for(int i = 0; i < 2; i++){
    Serial.print(i);
    digitalWrite(led, 1);
    delay(500);
    digitalWrite(led, 0);
    delay(500);
 }
}

///////////////
//-----------//
//MOVIMIENTOS//
//-----------//
///////////////

void AVANZAR(int velocidad) {
 analogWrite(motorDA, velocidad);
 analogWrite(motorDB, LOW);
 analogWrite(motorIA, velocidad);
 analogWrite(motorIB, LOW);
 Serial.println("Avanza");
 Serial.println(velocidad);
}

void DERECHA(int velocidad) { 
 analogWrite(motorDA, velocidad);
 analogWrite(motorDB, LOW);
 analogWrite(motorIA, LOW);
 analogWrite(motorIB, velocidad);
 Serial.println("Derecha");
 Serial.println(velocidad);
}

void IZQUIERDA(int velocidad) { 
 analogWrite(motorDA, LOW);
 analogWrite(motorDB, velocidad);
 analogWrite(motorIA, velocidad);
 analogWrite(motorIB, LOW);
 Serial.println("Izquierda");
 Serial.println(velocidad);
}

void ATRAS(int velocidad) { 
 analogWrite(motorDA, LOW);
 analogWrite(motorDB, velocidad);
 analogWrite(motorIA, LOW);
 analogWrite(motorIB, velocidad);
 Serial.println("Atras");
 Serial.println(velocidad);
}

void NADA(){
 analogWrite(motorDA, LOW);
 analogWrite(motorDB, LOW);
 analogWrite(motorIA, LOW);
 analogWrite(motorIB, LOW);
 Serial.println("Nada");
}


//////////////////
//--------------//
//FUNCIONES SUMO//
//--------------//
//////////////////

int leerSHARP(int sharp, int n){
  long suma=0;
  for(int i=0;i<n;i++){
    suma=suma+analogRead(sharp);
  }  
  return(suma/n);
}

void leerUltrasonicos(){
  // FRENTE IZQUIERDO
  digitalWrite(TRIGGERIZQ, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGERIZQ, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGERIZQ, LOW);

  long t1 = pulseIn(ultra1, HIGH);

  d1 = t1 * 0.034 / 2;

  // FRENTE DERECHO
  digitalWrite(TRIGGERDER, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGERDER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGERDER, LOW);

  long t2 = pulseIn(ultra2, HIGH);

  d2 = t2 * 0.034 / 2;
}

void REUBICAR(){ 
  valorP1 = analogRead(sensorP1);
  valorP2 = analogRead(sensorP2);
  valorP3 = analogRead(sensorP3);

  if(faseReubicar == 0){
    if(sensorP1 && sensorP2 <= 1500){ // Detecta borde con los 2 TCRT
      ATRAS(127);              // Retrocede
      tiempoInicio = millis();       
      faseReubicar = 1;              // pasa a fase retrocediendo
    }
  }

  else if(faseReubicar == 1){
    if(millis() - tiempoInicio >= 400){  // tiempo de retroceso (ajustar si es necesario en pruebas)
      DERECHA(127);                // empieza a girar
      tiempoInicio = millis();
      faseReubicar = 2;                  // pasa a fase girando
    }
  }

  else if(faseReubicar == 2){
    if(millis() - tiempoInicio >= 700){  // tiempo para 180° (ajustar si es necesario en pruebas)
      NADA();                            
      faseReubicar = 0;   // vuelve a estar listo para otra reubicación
    }
  }
}

void SUMO(int velocidad){
  leerUltrasonicos();
  int s1 = leerSHARP (A5, 20); //20 lecturas como un valor intermedio que suele ser suficiente para suavizar 
  int s2 = leerSHARP (A6, 20); //picos momentáneos sin hacer que la lectura sea demasiado lenta.
  int s3 = leerSHARP (A7, 20);

  REUBICAR();

  // --- FRENTE: prioridad máxima ---
  if(s1 < 200 || d1 < 30 || d2 < 30){ //MAS TARDE ACOMODAR EL RANGO DE DETECCION
    AVANZAR(255);
    Serial.println("DETECTA ADELANTE");
    return; //como la funcion es void, no devuelve nada, solo la corta
  }

  // --- DIAGONALES con prioridad ---
  else if(s3 < 200){   // diagonal derecha
    DERECHA(127);
    Serial.println("DETECTA DERECHA");
    return;
  }
  else if(s2 < 200){ // diagonal izquierda
    IZQUIERDA(127); 
    Serial.println("DETECTA IZQUIERDA");
    return;
  }
/*🔹 Qué pasa si no hay detección
Antes de llegar a esta parte, ya se revisan SHARP y ultrasonidos con return.
Si ninguno detecta, entonces el código llega a la faseBuscar.
Ahí decide si avanza (faseBuscar == 0) o gira (faseBuscar == 1).
Por eso no hace falta un else extra: la faseBuscar ya es la rutina por defecto cuando nada detecta.*/
if(faseBuscar == 0){
    AVANZAR(255);
    Serial.println("BUSCANDO...");
    if(millis() - tiempoInicio >= 600){ // tiempo de avance antes de girar
      tiempoInicio = millis();
      faseBuscar = 1;
    }
}
else if(faseBuscar == 1){
    DERECHA(127);  // giro rápido
    Serial.println("BUSCANDO...");
    if(millis() - tiempoInicio >= 600){ // tiempo de giro completo
      tiempoInicio = millis();
      faseBuscar = 0;  // vuelve a avanzar
    }
}
}

      ///////////////////////////////
      // F I N   F U N C I O N E S //
      ///////////////////////////////
            //sigue el setup//

void setup() {
  //MONITOR SERIE//
  Serial.begin(115200);
  //MOTORES//
  pinMode(ENABLEIZQ, OUTPUT);
  pinMode(ENABLEDER, OUTPUT);
  digitalWrite(ENABLEIZQ, HIGH);
  digitalWrite(ENABLEDER, HIGH);
  pinMode(motorDA,OUTPUT);
  pinMode(motorDB,OUTPUT);
  pinMode(motorIA,OUTPUT);
  pinMode(motorIB,OUTPUT);
  //AREA//
  faseBuscar = 0;
  faseReubicar = 0;
  tiempoInicio = millis();  // inicializa contador de tiempo
  pinMode(sharp1, INPUT)
  pinMode(sharp2, INPUT)
  pinMode(sharp3, INPUT)
  pinMode(ultra1, INPUT)
  pinMode(ultra2, INPUT)
  delay(3000);
  digitalWrite(led, 1);
  delay(500);
  digitalWrite(led, 0);
  delay(500);
  tiempoSeguridad();
}

void loop() { 
  SUMO(255);
}