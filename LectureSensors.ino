#include <DHT.h>
#include <DHT_U.h>

int sensor = 2;
int temperatura;
int humedad;

const int MQ2sensor   = A0;         // Definir la entrada analógica de MQ2
const int MQ5sensor   = A1;         // Definir la entrada analógica de MQ5
const int MQ7sensor   = A2;         // Definir la entrada analógica de MQ7
const int MQ135sensor = A3;       // Definir la entrada analógica de MQ135
const float RL = 1.0;             // El valor RL es la resistencia que tienen los modulos MQ
const float aire[4] = {9.83, 6.50, 27.10, 3.65};   // Valores aproximados del datasheet d los sensores MQ
float RS;                         // kohm
float RO;                         // kohm
float ratio;                       // RS/RO
int valor_crudo;                  // 0 a 1023

// Calibración del sensor
int muestra_calibracion            = 100;
int intervalo_calibracion          = 500;
int lectura_muestras               = 50;
int lectura_intervalos             = 5;

// Se definen 3 variables para que se detecte los 3 valores, los cuales son GLP, CO y HUMO
#define   HUMO       0
#define   GLP        1
#define   CO         2
#define   NH4        3

// COMO SE HACE LA CALIBRACION
// Las 3 curvas de cada gas pueden ser vistas en el datasheet del mq2, se toman los puntos, se saca la pendiente
// Cada uno muestra valores de PPM distintos de acuerdo a la sensibilidad del sensor, entonces este capta de una forma distinta
// cada uno de los componentes que se encuentren en el aire. En este caso para cada uno, se toma el valor que se muestra en el
// datasheet. Ahora para conseguir los 3 valores importante de cada concentración de gas, se deben de tener 3 valores importantes
// estos valores son X, Y y la pendiente, los cuales servirán para la calibración del dispositivo. NOTA: Los datos que se muestran
// en el datasheet se tiene que sacar su logaritmo (en base 10), para obtener los valores reales de la gráfica. ¿Por que se usa
// logaritmo? Porque el creador uso los logaritmos para que el grafico se vea lineal, sino se vería curvo, lo cual seria más
// dificil sacar los datos del gráfico. Se debe de usar la formula de la linea, la cual es "Y = mX + B"
// Utilizaremos el MQ2 como ejemplo, se van a tomar 2 puntos del grafico, donde tomaremos los siguiente puntos para GLP
// (200,1.6) y (10000,0.26). Ahora a estos valores se tiene que sacar su logaritmo de la siguiente forma
// (log(200), log(1.6)) y (log(10000), log(0.26)), y con los valores obtenidos de los logaritmos, se procede a hallar la pendeinte
// de esos dos puntos, de la siguiente forma " m = [log(y) - log(y0)] / [log(x) - log(x0)]", y aplicando los puntos anteriores se
// veria de la siguiente manera " m = [log(0.26) - log(1.6)] / [log(10000) - log(200)]" el cual da de resultado -0.46448 (-0.47).
// Luego tenemos que hallar el valor "b" de la función lineal, en este caso tomamos el punto (5000,0.35) para hallar b, y se resuelve
// de la siguiente manera: "log(0.36) = -0.47 * log(5000) + b", donde el valor de b es de 1.27

// Puntos de cada curva. Cada sensor con su respectivo componente
float curvaHUMO[3]  = {2.30, 0.53, -0.44}; 
float curvaGLP[3]   = {2.30, -0.15, -0.37};
float curvaCO[3]    = {1.70, 0.24, -0.677};
float curvaNH4[3]   = {1.00, 0.46, -0.235};

float ROhumo = 10;
float ROglp = 10;
float ROco = 10;
float ROnh4 = 10;
// IMPORTANTE, PRIMERO SE TIENE QUE HALLAR EL VALOR R0, AL CALIBRAR LOS SENSORES, YA QUE ESTE VA A TENER UN VALOR FIJO

void setup() {
  
 Serial.begin(115200);
 dht.begin();
 pinMode(13,OUTPUT);
 digitalWrite(13,HIGH);
 Serial.print("Calibrando...");

 ROhumo = calibracionSensor(MQ2sensor, aire[0]);
 ROglp  = calibracionSensor(MQ5sensor, aire[1]);
 ROco   = calibracionSensor(MQ7sensor, aire[2]);
 ROnh4  = calibracionSensor(MQ135sensor, aire[3]);
 digitalWrite(13,LOW);

 //Serial.println("Calibracion terminada!");
 //Serial.print("RO-humo = ");
 //Serial.print(ROhumo);
 //Serial.print("RO-glp = ");
 //Serial.print(ROglp);
 //Serial.print("RO-co = ");
 //Serial.print(ROco);
 //Serial.print("RO-nh4 = ");
 //Serial.print(ROnh4);
 delay(1000);
}

void loop() {
  long ppm_humo    = 0;
  long ppm_glp     = 0;
  long ppm_co      = 0;
  long ppm_nh4     = 0;
  
  ppm_humo   = obtener_porc_gas(lecturaMQ(MQ2sensor)/ROhumo, HUMO);
  ppm_glp    = obtener_porc_gas(lecturaMQ(MQ2sensor)/ROglp, GLP);
  ppm_co     = obtener_porc_gas(lecturaMQ(MQ2sensor)/ROco, CO);
  ppm_nh4    = obtener_porc_gas(lecturaMQ(MQ135sensor)/ROnh4, NH4);

  temperatura = dht.readTemperature();
  humedad = dht.readHumidity();

  Serial.println("**********         VALORES PPM          **********");
  Serial.print("HUMO: ");
  Serial.print(ppm_humo);
  Serial.println(" ppm");
  
  Serial.print("GLP: ");
  Serial.print(ppm_glp);
  Serial.println(" ppm");
  
  Serial.print("CO: ");
  Serial.print(ppm_co);
  Serial.println(" ppm");

  Serial.print("NH4: ");
  Serial.print(ppm_nh4);
  Serial.println(" ppm");

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.print("°C");

  Serial.print("Humedad: ");
  Serial.print(ppm_nh4);
  Serial.println("%");
  delay(500);
}

//  FUNCIONES
// Permite conseguir el valor RS de acuerdo lo que recibe el sensor
float calcularRS(int crudo){
  return (((float) RL * (1023 - crudo) / crudo));
}

// Permite hacer la calibracion del sensor definiendo el pin de conexion analogico
// La funcion permite calcular el Ro
float calibracionSensor(int pin, float aireSensor){
  int i;
  float val=0;
              
  for (i=0; i<muestra_calibracion;i++){
    val += calcularRS(analogRead(pin));
    delay(intervalo_calibracion);
  }
  val = val/muestra_calibracion;
  val = val/aireSensor;
  return val;
}

// Esta funcion sirve para volver a calcular el RS del sensor, esto se debe a que el RS cambia de acuerdo a las distintas
// concentraciones que hay en el datasheet.
float lecturaMQ(int pin){
  int i;
  float rs=0;

  for (i=0; i<lectura_intervalos;i++){
    rs += calcularRS(analogRead(pin));
    delay(lectura_muestras);
  }
  rs = rs/lectura_intervalos;
  return rs;
}

// Esta funcion determina las curvas que se van a detectar, es mas un ordenador, para definir lo que va a hacer el sensor y el 
// componente que se quiere leer.
long obtener_porc_gas(float ratio_rs_ro, int id_gas){
  if (id_gas == HUMO){
    return obtener_porc_sensor(ratio_rs_ro,curvaHUMO);
  } else if (id_gas == GLP){
    return obtener_porc_sensor(ratio_rs_ro,curvaGLP);
  } else if (id_gas == CO){
    return obtener_porc_sensor(ratio_rs_ro,curvaCO);
  } else if (id_gas == NH4){
    return obtener_porc_sensor(ratio_rs_ro,curvaNH4);
  }
  return 0;
}

long obtener_porc_sensor(float ratio_rs_ro, float *pcurve){
  
  return (pow(10,( ((log(ratio_rs_ro)-pcurve[1])/pcurve[2]) + pcurve[0])));
}
