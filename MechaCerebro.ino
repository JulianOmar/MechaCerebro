// Biblioteca para controlar LCD 16x2
#include <LiquidCrystal.h>
#include <Servo.h>

#define SERIAL 9600

#define SENSOR_TEMP A5
// LCD 16x2
#define PIN_LCD_RS 2
#define PIN_LCD_EN 3
#define PIN_LCD_D4 A3
#define PIN_LCD_D5 A2
#define PIN_LCD_D6 A1
#define PIN_LCD_D7 A0

#define COL_DISPLAY 16
#define ROW_DISPLAY 2

#define PIN_MOTOR A4
//PINS TECLADO
#define PIN_COL_1 7
#define PIN_COL_2 6
#define PIN_COL_3 5
#define PIN_COL_4 4
#define PIN_ROW_1 11
#define PIN_ROW_2 10
#define PIN_ROW_3 9
#define PIN_ROW_4 8
const byte fil = 4;
const byte col = 4;

byte pin_fil[] = {PIN_ROW_1, PIN_ROW_2, PIN_ROW_3, PIN_ROW_4};
byte pin_col[] = {PIN_COL_1, PIN_COL_2, PIN_COL_3, PIN_COL_4};

char teclas[fil][col] =
    {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}};

//SENSOR DE PROXIMIDAD
#define PIN_SENSOR 13
//PIN DEL SERVOMOTOR
#define PIN_SERVO = 12;


unsigned long tiempo_actual = 0;
unsigned long tiempo_anterior = 0;

//Crear el objeto LCD con los números correspondientes (rs, en, d4, d5, d6, d7)
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

//constantes de conversion

#define CARGA_VOLT 2.52

#define TIEMPO_MAX_MILIS 250 // 0.25 segundo.

#define ACTIVADO 1
#define DESACTIVADO 0

/**
 * EST_REPOSO stado inicial, esperando ingreso de cooredenadas
 * EST_MOV dispositivio en movimieto
 * EST_OSTACULO deteccion de ostaculo, revaluando nueva ruita
 * EST_RETORNOO regresando al origen
 * 
 */
#define EST_REPOSO 0
#define EST_MOV 1
#define EST_OSTACULO 2
#define EST_RETORNOO 3

#define NORTE 11
#define OESTE 22
#define SUR 33
#define ESTE 44

#define LIBRE 11
#define OBSTACULO 66

#define NUM_100 100
#define NUM_0 0
#define NUM_2 2
#define NUM_4 4
#define NUM_7 7
#define NUM_8 8

#define tam_mtx 7 //tamaño de matriz

int matriz[tam_mtx][tam_mtx];
int sensor_pos;             //registra si detecto o no un obstaculo
int pos_x, pos_y;           //posicion actual del dispositivo
int pos_dest_x, pos_dest_y; //posicion destino del dispositivo
int direccion;
bool mision_ok = false;

int estado_actual = EST_REPOSO; //estado inical del sistema

/**
 * cero lugar sin explorar
 * 8 posicion actual
 * UNO posicion anterior, recorrido
 * DOS ostaculo
 * 
 */
void init_matriz()
{
  for (int i = NUM_0; i < tam_mtx; i++)
    for (int j = NUM_0; j < tam_mtx; j++)
    {
      matriz[i][j] = NUM_0;
    }
  pos_y = pos_x = NUM_0;
  direccion = ESTE;
}
/**
 * leectura directo del sensor de proximidad
 * 
 */
int leer_sensor_prox()
{
  if () // NO HAY OSTACULO
    return LIBRE;
  return OBSTACULO;
}
/*
 * CAPTURA Y CONVERSION DEL SENSOR DE PROXIMIDAD
 */
void evaluar_obst()
{
  if (leer_sensor_prox() = LIBRE)
  {
    escribir_ruta(LIBRE);
    estado_actual = EST_MOV;
  }
  else
  {
    escribir_ruta(OBSTACULO);
    estado_actual = EST_OSTACULO;
  }
}

void escribir_ruta(int lectura)
{
  switch (direccion)
  {
  case (NORTE):
    matriz[pos_x][pos_y - 1] = lectura;
    break;
  case (SUR):
    matriz[pos_x][pos_y + 1] = lectura;
    break;
  case (ESTE):
    matriz[pos_x + 1][pos_y] = lectura;
    break;
  case (OESTE):
    matriz[pos_x - 1][pos_y] = lectura;
    break;
  }
}

/**
 * captura de las coordenadas de destino
 */
void ingreso_coordenadas()
{
  bool error = false;
  do
  {
    if (error)
    {
      lcd.setCursor(0, 1);
      lcd.print("ERROR DE COORD ");
      error = false;
    }
    /* CAPTURA DE COORDENADAS POR TECLADO */
    //lectura del coor x
    //escritura de coor x
    //lectura del coor y
    //escritura de coor y
    error = true;
  } while (pos_dest_x < tam_mtx && pos_dest_y < tam_mtx);
  lcd.setCursor(0, 1);
  lcd.print("COORDENADAS OK ");
}

/**
 * revisar si la mision fue abortada o el dispositivo llego a destino
 */
bool control_caos()
{
  if (pos_x == pos_dest_x && pos_y == pos_dest_y) //dispositiovo llego a destino
    return true;
  if () //finalizacion de mision manualmente por teclado
    return true;
  if () //finalizacin por deteccion de presencia por parte del senseor de temperatura
    return true;
  return false;
}

/**
 * Regula la velocidad del motor para avanzar o rotor
 */
void actualizar_motor()
{
  if (estado_actual == EST_MOV)
  {
    digitalWrite(PIN_MOTOR, (HIGH / NUM_2)); // mitad de la velocidad maxima del motor
  }
  else if (estado_actual == EST_OSTACULO) //motor detenido
  {
    digitalWrite(PIN_MOTOR, LOW);
  }
  else if (estado_actual == EST_ROTAR)
  {
    digitalWrite(PIN_MOTOR, (HIGH / NUM_4)); //un cuarto de la velocidad maxcima del motor para roptar sobre si mismo
  }
}
/**
 * llegada al destino, retornando al origen por ruta marcada
 */ 
void retornar()
{

}

/**
 * obstaculo detectad, recalculado ruta
 */ 
void recalculando_ruta()
{
}

/**
 * movimeinto a destino ddel  dispositivo
 */
void mover_dispo()
{
  if (evaluar_obst())
  {
  }
  actualizar_motor();
}

/*
 * EVALUA ES ESTADO DEL SISTEMA
 */
int evaluar_estado_actual()
{
  if (control_caos() == true)
    estado_actual = EST_RETORNOO;
  else if (leer_sensor_prox() == OBSTACULO)
    estado_actual = EST_OSTACULO;
  else if (mision_ok) //mision finalizada
    estado_actual = EST_REPOSO;
  else
    estado_actual = EST_MOV;

  return estado_actual;
}

/**
 * Evaluacion del estado del sistema y acciones a realizar
 * actualiza display
 */
void maquinadeEstado()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  switch (evaluar_estado_actual())
  {
  case (EST_REPOSO): // estado inicial
    lcd.print("ESTADO EN REPOPSO");
    ingreso_coordenadas();
    break;
  case (EST_MOV): //dispositivo en movimiento
    lcd.print("ESTADO EN MOVIMIENTO");
    mover_dispo();
    break;
  case (EST_OSTACULO): //deteccion de ostaculo
    lcd.print("OBSTACULO DETECTADO");
    recalculando_ruta();
    break;
  case (EST_RETORNOO): // regresando al pnto de partida
    lcd.print("RETORNANDO");
    retornar();
    break;
  }
}

void iniciar_lcd()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("");
  lcd.setCursor(0, 1);
  lcd.print(" ");
}

/**
 * Imprime por la pantalla LCD en estado actual del sistema 
 */
void actualizar_lcd()
{
}

void setup()
{
  Serial.begin(SERIAL);

  lcd.begin(COL_DISPLAY, ROW_DISPLAY); //display de estado del sistema
  iniciar_lcd();
  Servo.attach(PIN_SERVO);
  pinMode(PIN_MOTOR, OUTPUT); //MOTOR
  pinMode(SENSOR_TEMP, INPUT); //sensor de temperatura
}

void loop()
{

  // Toma el tiempo actual.
  tiempo_actual = millis();

  // Verifica cuanto transcurrido de tiempo
  if ((tiempo_actual - tiempo_anterior) >= TIEMPO_MAX_MILIS)
  {
    // Actualizo el tiempo anterior.
    tiempo_anterior = tiempo_actual;
    maquinadeEstado();
  }
}
