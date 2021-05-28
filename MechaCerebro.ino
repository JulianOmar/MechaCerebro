// Biblioteca para controlar LCD 16x2
#include <LiquidCrystal.h>
#include <Servo.h>
#include <Keypad.h>

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

//SENSOR DE PROXIMIDAD
#define PIN_PROX 13
//PIN DEL SERVOMOTOR
#define PIN_SERVO 12
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
Keypad teclado4x4 = Keypad(makeKeymap(teclas), pin_fil, pin_col, fil, col);

Servo servoMotor;
//constantes de conversion de temperatura
#define VOLT 5.0
#define MIN_TEMP 50
#define LECTURAMAX_TEMP 1024
#define EQUIVALENCIA 100
#define TEMP_TOPE 60
#define DIST_EQUI 0.01723
#define DIST_TOPE 100

unsigned long tiempo_actual = 0;
unsigned long tiempo_anterior = 0;
unsigned long tiempo_actual_2 = 0;
unsigned long tiempo_anterior_2 = 0;

//Crear el objeto LCD con los números correspondientes (rs, en, d4, d5, d6, d7)
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

//constantes de conversion

#define CARGA_VOLT 2.52

#define TIEMPO_MAX_MILIS 250   // 0.25 segundo.
#define TIEMPO_MAX_MILIS_2 100 // 0.1 segundo.

#define ACTIVADO 1
#define DESACTIVADO 0

/**
 * EST_REPOSO stado inicial, esperando ingreso de cooredenadas
 * EST_MOV dispositivio en movimieto
 * EST_OBSTACULO deteccion de ostaculo, revaluando nueva ruita
 * EST_RETORNOO regresando al origen
 * 
 */
#define EST_REPOSO 0
#define EST_COORD 1
#define EST_MOV 2
#define EST_OBSTACULO 3
#define EST_RETORNO 4

#define NORTE 0
#define ESTE 1
#define SUR 2
#define OESTE 3

#define LIBRE 11
#define OBSTACULO 66
#define RECORRIDO 1
#define DISPO 8

#define NUM_100 100
#define NUM_0 0
#define NUM_1 1
#define NUM_2 2
#define NUM_3 3
#define NUM_4 4
#define NUM_7 7
#define NUM_8 8

#define DIR_90 90
#define DIR_0 0
#define DIR_180 180
#define DIR_360 360
#define tam_mtx 7 //tamaño de matriz

int matriz[tam_mtx][tam_mtx];
int sensor_pos;             //registra si detecto o no un obstaculo
int pos_x, pos_y;           //posicion actual del dispositivo
int pos_dest_x, pos_dest_y; //posicion destino del dispositivo
int orientacion;            //orientacion del actual del dispositivo
int prox_orientacion;       //orientacion del prox del dispositivo en el retorno
bool mision_ok = false;

int estado_actual = EST_REPOSO; //estado inical del sistema
int estado_anterior;
bool init = true;             //inicio del sistema
bool abort_temp = false;      //abortar mision por temperatura
bool abort_manual = false;    //abortar mision manualmente
bool obst_encontrado = false; //registra si se esta detectando un obstaculo o no

char tecla;
const char TECLA_ABORT = 'A';
const char TECLA_INICIO = '#';

int iter_giro = 0; //registra los giros del sensor que realizo

int matriz_giro[4][4] = {
    {DIR_90, DIR_0, DIR_360, DIR_180},
    {DIR_180, DIR_90, DIR_0, DIR_360},
    {DIR_360, DIR_180, DIR_90, DIR_0},
    {DIR_0, DIR_360, DIR_180, DIR_90},
};

int orient_vec[NUM_4] = {NORTE, ESTE, SUR, OESTE};

/////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////
void imprimir_matriz()
{
  for (int i = NUM_0; i < tam_mtx; i++)
  {
    for (int j = NUM_0; j < tam_mtx; j++)
    {
      Serial.print(matriz[i][j]);
      Serial.print("  ");
    }
    Serial.println();
  }
}
/**
 * ROTAR SOBRE SI MISMO, DE SER NECESARIO
 * AVANZAR A LA PORXIMA POSICION CONTIGUA
 */
void rotar_avanzar(int giro)
{
  if (giro == DIR_360)
  {
    rotar(DIR_180);
    rotar(DIR_180);
  }
  if (giro != DIR_90)
  {
    rotar(giro);
  }
  rotar(DIR_90);                           //ESPERAR TEMPORIZADOR DE ENDEREZADO
  digitalWrite(PIN_MOTOR, (HIGH / NUM_4)); //AVANZAR A LA PROXIMA DIRECCION
}
/**
 * orientacion del proximo paso en el retorno al origen
 * reescritura de la matriz
 */
int prox_paso()
{
  if (((pos_y - NUM_1) < NUM_0) && matriz[pos_x][(pos_y - NUM_1)] == RECORRIDO)
  {
    matriz[pos_x][(pos_y - 1)] = DISPO;
    matriz[pos_x][pos_y] = OBSTACULO;
    pos_y--;
    return NORTE;
  }

  if (((pos_x + NUM_1) >= tam_mtx) && matriz[pos_x + NUM_1][pos_y] == RECORRIDO)
  {
    matriz[pos_x + NUM_1][pos_y] = DISPO;
    matriz[pos_x][pos_y] = OBSTACULO;
    pos_x++;
    return ESTE;
  }
  if (((pos_y + NUM_1) >= tam_mtx) && matriz[pos_x][pos_y + 1] == RECORRIDO)
  {
    matriz[pos_x][pos_y + NUM_1] = DISPO;
    matriz[pos_x][pos_y] = OBSTACULO;
    pos_y++;
    return SUR;
  }
  if (((pos_x - NUM_1) < NUM_0) && matriz[pos_x - NUM_1][pos_y] == RECORRIDO)
  {
    matriz[pos_x - NUM_1][pos_y] = DISPO;
    matriz[pos_x][pos_y] = OBSTACULO;
    pos_x--;
    return OESTE;
  }
}

/**
 * llegada al destino, retornando al origen por ruta marcada
 */
void retornar()
{
  prox_orientacion = prox_paso();
  rotar_avanzar(matriz_giro[orientacion][prox_orientacion]);
}
/**
 * permite rotar el dispositivo, se incorpora temporizador extra dado 
 * que el servo y el dispositivo demorar en alcanzar su posicion deseada
 */
void rotar(int giro)
{
  servoMotor.write(giro);
  if (giro != DIR_90)
    digitalWrite(PIN_MOTOR, HIGH);
  tiempo_actual_2 = millis(); //TEMPORIZADOR PARA ROTAR
  while ((tiempo_actual_2 - tiempo_anterior_2) >= TIEMPO_MAX_MILIS_2)
  {
    tiempo_actual_2 = millis();
  }
  tiempo_anterior_2 = tiempo_actual_2;

  digitalWrite(PIN_MOTOR, LOW); //DETENER MOTOR
  servoMotor.write(DIR_90);     // ENDEREZAR DIRECCION
}

bool escribir_obst()
{
  switch (orientacion)
  {
  case NORTE:
    if ((pos_y - NUM_1) < NUM_0)
      return false;
    matriz[pos_x][(pos_y - NUM_1)] = OBSTACULO;
    break;
  case ESTE:
    if ((pos_x + NUM_1) >= tam_mtx)
      return false;
    matriz[pos_x + NUM_1][pos_y] = OBSTACULO;
    break;
  case SUR:
    if ((pos_y + NUM_1) >= tam_mtx)
      return false;
    matriz[pos_x][pos_y + NUM_1] = OBSTACULO;
    break;
  case OESTE:
    if ((pos_x - NUM_1) < NUM_0)
      return false;
    matriz[pos_x - NUM_1][pos_y] = OBSTACULO;
    break;
  }
}
/**
 * obstaculo detectad, recalculado ruta
 */
void recalcular_ruta()
{
  escribir_obst();
  switch (iter_giro)
  {
  case NUM_0: //IZQUIERDA
    rotar(DIR_180);
    iter_giro++;
    orientacion = orient_vec[(orientacion - NUM_1) % NUM_4];
    break;
  case NUM_1: //DERECHA
    rotar(DIR_0);
    rotar(DIR_0);
    orientacion = orient_vec[(orientacion + NUM_1) % NUM_4];
    orientacion = orient_vec[(orientacion + NUM_1) % NUM_4];
    iter_giro++;
    break;
  case NUM_2: //ATRAS
    rotar(DIR_0);
    orientacion = orient_vec[(orientacion + NUM_1) % NUM_4];
    avanzar();
    rotar(DIR_0);
    rotar(DIR_0);
    orientacion = orient_vec[(orientacion + NUM_1) % NUM_4];
    orientacion = orient_vec[(orientacion + NUM_1) % NUM_4];
    escribir_obst();
    iter_giro = NUM_0;
    break;
  }
}
/**
 * verifica si no nos salimos del mapa y actualiza la matriz
 */
bool avanzar()
{
  switch (orientacion)
  {
  case NORTE:
    if ((pos_y - NUM_1) < NUM_0 && matriz[pos_x][(pos_y - NUM_1)] != OBSTACULO)
      return false;
    matriz[pos_x][(pos_y - NUM_1)] = DISPO;
    matriz[pos_x][pos_y] = RECORRIDO;
    pos_y--;
    break;
  case ESTE:
    if ((pos_x + NUM_1) >= tam_mtx && matriz[pos_x + NUM_1][pos_y] != OBSTACULO)
      return false;
    matriz[pos_x + NUM_1][pos_y] = DISPO;
    matriz[pos_x][pos_y] = RECORRIDO;
    pos_x++;
    break;
  case SUR:
    if ((pos_y + NUM_1) >= tam_mtx && matriz[pos_x][pos_y + NUM_1] != OBSTACULO)
      return false;
    matriz[pos_x][pos_y + NUM_1] = DISPO;
    matriz[pos_x][pos_y] = RECORRIDO;
    pos_y++;
    break;
  case OESTE:
    if ((pos_x - NUM_1) < NUM_0 && matriz[pos_x - NUM_1][pos_y] != OBSTACULO)
      return false;
    matriz[pos_x - NUM_1][pos_y] = DISPO;
    matriz[pos_x][pos_y] = RECORRIDO;
    pos_x--;
    break;
  }
  return true;
}

/**
 * movimeinto a destino del  dispositivo
 */
void mover_dispo()
{
  if (pos_x >= pos_dest_x)
  {
    rotar(DIR_90);
    orientacion = orient_vec[(orientacion + NUM_1) % NUM_4];
  }
  /**
   * false: fuera de rango u obstaculo encontrado
   */
  if (avanzar())
  {
    digitalWrite(PIN_MOTOR, (HIGH / NUM_2)); // mitad de la velocidad maxima del motor
    iter_giro = NUM_0;                       //reinicio contador de giro
  }
  else
    recalcular_ruta();
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
      lcd.setCursor(NUM_0, NUM_1);
      lcd.print("ERROR DE COORD ");
      error = false;
    }
    /* CAPTURA DE COORDENADAS POR TECLADO */
    do
    {
      lcd.print("Ingrese coor X...");
      pos_dest_x = teclado4x4.getKey();
    } while (pos_dest_x);
    lcd.clear();
    do
    {
      lcd.print("Ingrese coor Y...");
      pos_dest_y = teclado4x4.getKey();
    } while (pos_dest_y);

    error = true;
  } while (pos_dest_x < tam_mtx && pos_dest_y < tam_mtx);
  lcd.clear();
  lcd.setCursor(NUM_0, NUM_1);
  lcd.print("COORDENADAS OK ");
}
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
  matriz[pos_y][pos_x] = DISPO;
  orientacion = SUR; //orietnacion inicial del dispositivo
}

/**
 * revisar si la mision fue abortada o el dispositivo llego a destino
 */
bool control_caos()
{
  if (pos_x == pos_dest_x && pos_y == pos_dest_y) //dispositiovo llego a destino
  {
    mision_ok = true;
    return true;
  }

  if (abort_temp) //finalizacin por deteccion de presencia por parte del senseor de temperatura
    return true;
  return false;
}

/**
 * EVALUA EL ESTADO DEL SISTEMA
 */
int evaluar_estado_actual()
{
  tecla = teclado4x4.getKey(); // CAPTURA DE INGRESO DEL TECLADO

  if (tecla)
  {
    switch (estado_actual)
    {
    case EST_REPOSO:
      if (tecla == TECLA_INICIO)
      {
        init_matriz();
        estado_actual = EST_COORD;
      }
      break;
    case EST_MOV:
      if (tecla == TECLA_ABORT) //finalizacion de mision manualmente por teclado
        estado_actual = EST_RETORNO;
      break;
    case EST_OBSTACULO:
      if (tecla == TECLA_ABORT) //finalizacion de mision manualmente por teclado
        estado_actual = EST_RETORNO;
      break;
    }
  }
  leer_sensores();
  if ((estado_actual == EST_OBSTACULO || estado_actual == EST_MOV) && control_caos())
    estado_actual = EST_RETORNO;

  else if (obst_encontrado && estado_actual == EST_MOV)
    estado_actual = EST_OBSTACULO;
  else if (estado_actual != EST_REPOSO && estado_actual != EST_COORD)
    estado_actual = EST_MOV;

  return estado_actual;
}

/**
 * Evaluacion del estado del sistema y acciones a realizar
 * actualiza display
 */
void maquinadeEstado()
{
  estado_anterior = estado_actual;
  lcd.clear();
  lcd.setCursor(NUM_0, NUM_0);
  switch (evaluar_estado_actual())
  {
  case (EST_REPOSO): // estado inicial

    lcd.print("ESTADO EN REPOPSO");
    if (estado_actual != estado_anterior || init)
      Serial.println("ESTADO EN REPOPSO");
    init = false;
    break;
  case (EST_COORD): // estado EN ESPERA DE COORDENADAS
    lcd.print("ESTADO ESPERANDO COORD");
    if (estado_actual != estado_anterior)
      Serial.println("ESTADO ESPERANDO COORD");
    ingreso_coordenadas();
    break;
  case (EST_MOV): //dispositivo en movimiento
    lcd.print("ESTADO EN MOVIMIENTO");
    if (estado_actual != estado_anterior)
      Serial.println("ESTADO EN MOVIMIENTO");
    mover_dispo();
    break;
  case (EST_OBSTACULO): //deteccion de ostaculo
    lcd.print("OBSTACULO DETECTADO");
    if (estado_actual != estado_anterior)
      Serial.println("OBSTACULO DETECTADO");
    recalcular_ruta();
    break;
  case (EST_RETORNO): // regresando al pnto de partida
    lcd.print("RETORNANDO");
    if (estado_actual != estado_anterior)
      Serial.println("RETORNANDO");
    retornar();
    break;
  }
}

/**
 *  LECTURA DEL SENSOR DE DISTACNIA ULTRASONICO 
 */
int leer_ultrasonido()
{
  pinMode(PIN_PROX, OUTPUT);
  // limpio el TRIGGER
  digitalWrite(PIN_PROX, LOW);
  delayMicroseconds(2);
  //pongo HIGH el trigger por 10 microsegundos
  digitalWrite(PIN_PROX, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_PROX, LOW);
  pinMode(PIN_PROX, INPUT);

  //Leo la señal ECHO y retorno distancia
  return (int)(DIST_EQUI * pulseIn(PIN_PROX, HIGH));
}

/**
 * RELEVAMIENTO DE LOS SENSORES
 */
void leer_sensores()
{
  int temp_actual = (int)((VOLT / LECTURAMAX_TEMP * analogRead(SENSOR_TEMP)) * EQUIVALENCIA - MIN_TEMP);
  if (temp_actual > TEMP_TOPE) // aborta por execo de temperatura
    abort_temp = true;
  else
    abort_temp = false;

  if (leer_ultrasonido() >= DIST_TOPE) // se detecto la presencia de un obstaculo
    obst_encontrado = true;
  else
    obst_encontrado = false;
}

void setup()
{
  Serial.begin(SERIAL);
  lcd.begin(COL_DISPLAY, ROW_DISPLAY); //display de estado del sistema
  lcd.display();
  pinMode(PIN_MOTOR, OUTPUT);  //MOTOR
  pinMode(SENSOR_TEMP, INPUT); //sensor de temperatura
  servoMotor.attach(PIN_SERVO);
  servoMotor.write(DIR_90); //inicializa el servo
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
