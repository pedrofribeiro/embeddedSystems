/*
  First Arduino Project
  Author: Pedro Felipe de Oliveira Ribeiro
  Institute of Science and Technology - Federal University of Sao Paulo
*/
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <stdlib.h>
#include <string.h>
#include "Keypad.h"
/*===========================================================================*/
/*
  SYSTEM'S CONTROL FLAGS
*/
/*===========================================================================*/
/*
PURPOSE: Flag that informs if the current execution is the first, or not.
NEEDS EEPROM: YES
CAN BE MODIFIED: only 1 time
*/
int FIRST_USE;
/*
PURPOSE: Flag that holds the current free address of the EEPROM
NEEDS EEPROM: YES
CAN BE MODIFIED: yes, many times
*/
int EEPROM_GLOBAL;
/*
PURPOSE: Flag that holds the current number of users of the system
NEEDS EEPROM: YES
CAN BE MODIFIED: yes, many times
*/
int NUMBER_OF_USERS;
/*
PURPOSE: Flag that holds the current number of users of the system
NEEDS EEPROM: YES, but not directly, its calculated during runtime
CAN BE MODIFIED: yes, many times
*/
int NUMBER_OF_NEW_USER;
/*
PURPOSE: Flag of the current address to be used to keep the system's log of activities
NEEDS EEPROM: YES
CAN BE MODIFIED: yes, many times
*/
int LOG_ADDRESS;
/*
PURPOSE: Flag that holds the number of the current user
NEEDS EEPROM: YES
CAN BE MODIFIED: yes, many times
*/
int CURRENT_USER;


/*===========================================================================*/
/*
  System variables
*/
/*===========================================================================*/

const int PIN_RED_LED = 0;
const int PIN_GREEN_LED = 1;
const int PIN_BUZZER = 2;
const int btn1 = 3;
const int btn2 = 4;
const int btn3 = 5;
int ENTER;
int UP;
int DOWN;
const int bargraph[] = {31,33,35,37,39,41,43,45,47,49};
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
char buffer[70];
const byte Rows= 4;
const byte Cols= 4;
char keymap[Rows][Cols]=
{
{'1', '2', '3', 'A'},
{'4', '5', '6', 'B'},
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}
};
byte rPins[Rows]= {22,24,26,28}; 
byte cPins[Cols]= {32,34,36,38};
Keypad kpd= Keypad(makeKeymap(keymap), rPins, cPins, Rows, Cols);

/*===========================================================================*/
/*
  UTILITIES FUNCTIONS NEEDED BY THE SYSTEM
*/
/*===========================================================================*/

/*
NAME: EEPROMClear.
PURPOSE: Clear all the bits of the EEPROM memory available (multiplatform) and prepare the first_use flag.
PARAMETERS: void.
RETURNS: void.
*/
void EEPROMClear(){
  /*
    Iterate through each byte of the EEPROM storage.
    Uses the pre-provided length function.
  */
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
  /*setting up the fist use as true*/
  EEPROM.write(0,1);

}

/*
NAME: setupSystem.
PURPOSE: Sets up ALL the controls needed to the correct execution of the system.
PARAMETERS: void.
RETURNS: void.
*/
void setupSystem(){
  /*
    EEPROM used in a heap configuration
    (0) FLAG OF FIRST USE
    (1) POINTER TO NEXT AVALIABLE ADDRESS
    (2) NUMBER OF USERS
    (3) FACTORY USER NUMBER/NAME, after first use, the factory data will be overwritten.
    (4) FACTORY USER PWD
    (5) FIRST USER CONTROL LEVEL
    ...
    (n-2) LOG OF USE 2
    (n-1) LOG OF USE 1
    (n) LOG OF FIRST USE
  */
  int firstUse;
  firstUse = EEPROMRead(0);

  if(firstUse == 1){

    showMessages("PRIMEIRO USO","DO SISTEMA");
    delay(2500);
    EEPROM.write(0, 1); // turns first use to 1, as its beeing currently used
    FIRST_USE = 1; //keeps it as 1, as it has not been used yet
    EEPROM.write(1, 3); //turns global eeprom to 3
    EEPROM_GLOBAL = 3;
    EEPROM.write(2, 0); //number of users = 0
    EEPROM.write(4, 1); //factory SU, to be used ONLY ONCE, during firstUseInit()
    EEPROM.write(4, 111); //factory SU PWD, to be used ONLY ONCE, during firstUseInit()
    NUMBER_OF_USERS = 0;
    NUMBER_OF_NEW_USER = 0;
    LOG_ADDRESS = EEPROM.length()-1;
    EEPROM.write(EEPROM.length(),LOG_ADDRESS);
    return;

  }else{

    showMessages("INICIALIZANDO","SISTEMA");
    FIRST_USE = 0; //its not the first use!
    EEPROM_GLOBAL = EEPROMRead(1); //fixed position
    NUMBER_OF_USERS = EEPROMRead(2); //fixed position
    NUMBER_OF_NEW_USER = NUMBER_OF_USERS+1;
    LOG_ADDRESS = EEPROMRead(EEPROM.length());
    return;

  }
}

/*
NAME: EEPROMRead.
PURPOSE: Reads a byte that is stored in the EEPROM, and returns its int val.
PARAMETERS: int address
RETURNS: integer.
*/
int EEPROMRead(int address){
  byte value;
  int result;
  value = EEPROM.read(address);
  result = int(value);
  return result;
}

/*
NAME: keepLog
PURPOSE: Records all the relevant behavior of the system
PARAMETERS: int operation, char* user
RETURNS: void.
*/
void keepLog(int operation, int user){
  /*
  LOG PROTOCOL 16 bits (2 positions)
  LOG is only good up to 52 days
  operation (3 bits)|user (5 bits)|
  */
  char charPartOne[10];
  int partOne;

  sprintf(charPartOne,"%d%d",operation,user);
  partOne = atoi(charPartOne);

  EEPROM.write(LOG_ADDRESS,partOne);
  delay(100);
  LOG_ADDRESS = LOG_ADDRESS - 1;
  EEPROM.write(EEPROM.length(),LOG_ADDRESS);
  delay(100);
}

/*
NAME: readLog
PURPOSE: Prints all the log entries to the serial output
PARAMETERS: void.
RETURNS: void.
*/
void readLog(){
  char entry[10];
  int temp;

  for (int i = (EEPROM.length()-1); i > LOG_ADDRESS; i--) {
    temp = EEPROMRead(i);
    delay(100);
    sprintf(entry,"%d\n",temp);
    Serial.println(entry);
  }
}

/*
NAME: buttonUpdate
PURPOSE: debounces the acquisition of an analog button
PARAMETERS: int pin
RETURNS: integer
*/
int buttonUpdate(int PIN){
  int estado = digitalRead(PIN);
  delay(100);
  return estado;
}

/*
NAME: showMessages
PURPOSE: prints messages on a lcd display
PARAMETERS: char* msg1, char* msg2
RETURNS: void.
*/
void showMessages(char* msg1, char* msg2){
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print(msg1);
  lcd.setCursor(0,1);
  lcd.print(msg2);
}

/*
NAME: createUser
PURPOSE: creates a triple of user, password and control level on the EEPROM
PARAMETERS: int NUMBER_OF_NEW_USER, int PWD, int control level
RETURNS: void.
*/
void createUser(int user_number, int pwd, int cl){

  NUMBER_OF_USERS = NUMBER_OF_USERS + 1;
  EEPROM.write(2,NUMBER_OF_USERS); //increases the NUMBER_OF_USERS
  EEPROM.write(EEPROM_GLOBAL,user_number); //creates a new user with number eq to NUMBER_OF_NEW_USER
  EEPROM_GLOBAL = EEPROM_GLOBAL + 1;
  EEPROM.write(1,EEPROM_GLOBAL); //increases the EEPROM pointer
  EEPROM.write(EEPROM_GLOBAL,pwd); //creates the pwd for the new user
  EEPROM_GLOBAL = EEPROM_GLOBAL + 1;
  EEPROM.write(1,EEPROM_GLOBAL); //increases the EEPROM pointer
  EEPROM.write(EEPROM_GLOBAL,cl); //creates the cl for the new user
  EEPROM_GLOBAL = EEPROM_GLOBAL + 1;
  EEPROM.write(1,EEPROM_GLOBAL); //increases the EEPROM pointer
  NUMBER_OF_NEW_USER = NUMBER_OF_USERS + 1;
  /*log*/
  if(cl == 1){
    keepLog(2,user_number);
  }else{
    keepLog(3,user_number);
  }

}

/*
NAME: excludeUser
PURPOSE: Removes a given user from the system
PARAMETERS: int number of the user to be removed
RETURNS: void.
*/
void excludeUser(int user_number){

  int pos = (3*user_number); /*new pointer to access positions*/
  EEPROM.write(pos,0); //removes the user with number eq to user_number
  pos = pos + 1;
  EEPROM.write(pos,0); //removes the pwd of the user
  pos = pos + 1;
  EEPROM.write(pos,0); //removes the cl of the user
  /*log*/
  keepLog(4,user_number);

}

/*
NAME: validateAccess
PURPOSE: verifies the match between a user and his password.
PARAMETERS: int user, int password
RETURNS: boolean.
*/
int validateAccess(int user, int password){

  int res, u, pwd;
  u = 3*user;
  pwd = u+1;
  res = EEPROMRead(pwd);
  delay(400);
  if(res == 0) {
    showMessages("USUARIO","INEXISTENTE");
    delay(2500);
    return 0;
  }
  if(password == res) return 1;
  else return 0;

}

/*
NAME: isSuperUser
PURPOSE: verifies if a given user is a super user
PARAMETERS: int user.
RETURNS: boolean.
*/
int isSuperUser(int user){
  int res, usr, cl;
  usr = 3*user;
  cl = usr+2;
  res = EEPROMRead(cl);
  delay(400);
  if(res == 1) return 1;
  else return 0;
}

/*
NAME: readInput
PURPOSE: reads the entered input
PARAMETERS: type of the input to be read: pwd, user, or factory super user.
RETURNS: integer.
*/
int readInput(int type){
  char informedInput[10], asterisc[10], tt4[5];
  char keypressed;
  int level = 0;
  int intInformedInput;
  int finishedTyping = 0;

  strcpy(informedInput,"");
  strcpy(asterisc,"");

type_message:
  switch (type) {
    case 0:
      showMessages("SENHA SU PADRAO",asterisc);
    break;
    case 1:
      /*bargraph*/
      for (int h = 0; h < strlen(asterisc); h++) {
        digitalWrite(bargraph[h], HIGH);
        digitalWrite(bargraph[h+1], HIGH);
        digitalWrite(bargraph[h+2], HIGH);
      }
      showMessages("INFORME A SENHA",asterisc);
    break;
    case 2:
      showMessages("INFORME USUARIO",informedInput);
    break;
  }

  /*waits until input is entered*/
  while(finishedTyping == 0){

      ENTER = buttonUpdate(btn3);

      keypressed = kpd.getKey();
      if(keypressed != NO_KEY){
        strncat(informedInput,&keypressed,1);
        strcat(asterisc,"*");
        goto type_message;
      }

      if(ENTER == HIGH){
				/*does not accepts blank entries*/
        if(strcmp(informedInput,"") == 0) goto type_message;
        finishedTyping = 1;
        continue;
      }
  }

  /*sets bargraph to zero*/
  for (int h = 0; h < 10; h++) {
    digitalWrite(bargraph[h], LOW);
  }

  /*converting the informedInput to integer*/
  intInformedInput = atoi(informedInput);

  return intInformedInput;
}

/*
NAME: setLed
PURPOSE: Configures a given led to an interest position
PARAMETERS: int led pin, int duration as high
RETURNS: void.
*/
void setLed(int p, int t, int k){

  if(k == 1){
    digitalWrite(p,HIGH);
  }else{
    digitalWrite(p,HIGH);
    delay(t);
    digitalWrite(p,LOW);
  }

}

/*
NAME: setBuzzer
PURPOSE: Configures the buzzer action
PARAMETERS: int buzzer pin, int freq, int duration
RETURNS: void.
*/
void setBuzzer(int p, int f, int d){
  tone(p, f, d);
}

/*
NAME: violationMode
PURPOSE: Halts all access for N milliseconds.
PARAMETERS: void.
RETURNS: void.
*/
void violationMode(){

  /*alert sound*/
  setBuzzer(PIN_BUZZER,800,5000);
  unsigned long LONG_DELAY_MS = 2905; 
  unsigned long startMillis = millis();
  showMessages("ACESSO","SUSPENSO");
  /*counting up to 2 hours*/
  while (millis() - startMillis < LONG_DELAY_MS);

}

/*
NAME: firstUseInit
PURPOSE: Sets the correct operation for the first use of the system
PARAMETERS: void.
RETURNS: void.
*/
void firstUseInit(){

  int suPassword;
  int password;
  int newPassword;
  int possibleRetries = 2;
  char text[15];
  strcpy(text,"#TENT REST:");

  setLed(PIN_RED_LED,0,1);
  setLed(PIN_GREEN_LED,0,0);

  /*reads the superuser password from memory, and the informed password from input*/
  suPassword = EEPROMRead(4);
  delay(200);

  /*log*/
  CURRENT_USER = 0; //factory super user (fsu)
  keepLog(6,CURRENT_USER);

  validate_access:
    /*validates if the entered password is correct*/
    password = readInput(0);

    if(password == suPassword){
      /*log*/
      CURRENT_USER = 0; //factory super user (fsu)
      keepLog(0,CURRENT_USER);

      /* needs to create a new user, and a new password
        the user necessarily will be a super user */
      showMessages("SENHA CORRETA","");
      delay(2500);
      showMessages("CRIANDO SUPER","USUARIO 1");
      delay(2500);
      newPassword = readInput(1);
      createUser(NUMBER_OF_NEW_USER,newPassword,1);
      delay(1000);
      showMessages("S-USUARIO CRIADO","COM SUCESSO");

      /*log*/
      CURRENT_USER = 1; //super user 1
      keepLog(2,CURRENT_USER);

      /*marking the flag of first use as false*/
      EEPROM.write(0,0);
      FIRST_USE = EEPROMRead(0);
      delay(1000);

      showMessages("REINICIANDO","");
      delay(2000);
      return;

    }else{

      /*log*/
      CURRENT_USER = 0; //fsu
      keepLog(1,CURRENT_USER);

      showMessages("SENHA","INCORRETA");
      delay(2500);
      if(possibleRetries > 0){
        sprintf(text,"# TENT. REST %d",possibleRetries);
        showMessages(text,"CUIDADO");
        delay(2500);
        possibleRetries--;
        goto validate_access;
      }else{

        /*log*/
        CURRENT_USER = 0; //fsu
        keepLog(5,CURRENT_USER);

        showMessages("#TENT ESGOTADO","");
        delay(2000);
        showMessages("ENTRANDO EM","MODO VIOLADO");
        delay(1000);
        showMessages("3","");
        delay(500);
        showMessages("2","");
        delay(500);
        showMessages("1","");
        delay(500);
        showMessages("ACESSO","SUSPENSO");
        delay(500);

        /*calls up violationMode*/
        violationMode();

      }// internal if
    }//external if
}

/*
NAME: superUserOptions
PURPOSE: displays the super user only options
PARAMETERS: int currentUser
RETURNS: void.
*/
void superUserOptions(){

  int selectedOption = 0;
  int currrentOption = 0;

  superUser_options:
  switch (currrentOption) {
    case 0:
      showMessages("ABRIR PORTA *","CRIAR USUARIO");
    break;
    case 1:
      showMessages("ABRIR PORTA", "CRIAR USUARIO*");
    break;
    case 2:
      showMessages("CRIAR USUARIO", "CRIAR S-USUARIO*");
    break;
    case 3:
      showMessages("CRIAR S-USUARIO", "EXCLUIR USUARIO*");
    break;
    default: break;
  }

  selectedOption = 0;
  while (selectedOption == 0) {

    ENTER = buttonUpdate(btn3);
    DOWN  = buttonUpdate(btn2);
    UP    = buttonUpdate(btn1);

    if((UP == HIGH) && (DOWN == LOW) && (ENTER == LOW)){
      currrentOption = currrentOption + 1;
      goto superUser_options;
    }
    if((UP == LOW) && (DOWN == HIGH) && (ENTER == LOW)){
      currrentOption = currrentOption -1;
      goto superUser_options;
     }
    if((currrentOption > 3) || (currrentOption < 0)) {
      currrentOption = 0;
      goto superUser_options;
    }
    if((UP == LOW) && (DOWN == LOW) && (ENTER == HIGH)){
      selectedOption = 1;
      continue;
    }
  } /*end of while*/

  if(selectedOption == 1){
      switch (currrentOption) {
        case 0:
          showMessages("ACESSO","AUTORIZADO");
          delay(2500);
          setLed(PIN_RED_LED,0,1);
          setLed(PIN_GREEN_LED,1000,1);
          setBuzzer(PIN_BUZZER,500,500);
          /*log*/
          keepLog(0,CURRENT_USER);
          return; /*breaks out of the function menu*/
        break;
        case 1:
          char s1[5];
          sprintf(s1,"%d",NUMBER_OF_NEW_USER);
          showMessages("NOVO USUARIO:", s1);
          delay(2500);
          int pss;
          pss = readInput(1);
          createUser(NUMBER_OF_NEW_USER,pss,0);
          showMessages("USUARIO CRIADO","COM SUCESSO");
          delay(2500);
          /*log*/
          keepLog(3,CURRENT_USER);
          return;
        break;
        case 2:
          char s2[5];
          sprintf(s2,"%d",NUMBER_OF_NEW_USER);
          showMessages("NOVO S-USUARIO:", s2);
          delay(2500);
          int pss2;
          pss2 = readInput(1);
          createUser(NUMBER_OF_NEW_USER,pss2,1);
          showMessages("S-USUARIO CRIADO","COM SUCESSO");
          delay(2500);
          /*log*/
          keepLog(2,CURRENT_USER);
          return;
        break;
        case 3:
          showMessages("PROCEDIMENTO","PARA EXCLUSAO");
          delay(2500);
          int usex;
          usex = readInput(2);
          excludeUser(usex);
          showMessages("EXCLUSAO","CONCLUIDA");
          delay(2500);
          /*log*/
          keepLog(4,CURRENT_USER);
          return;
        break;
        default: break;
      }//switch
  }else{
    goto superUser_options;
  }
}//fn

/*
NAME: login
PURPOSE: controls the login process
PARAMETERS: int currentUser
RETURNS: void.
*/
void login(){
  int userName;
  int userPassword;
  int possibleRetries = 2;
  char text[15];

  access_validation:

    userName = readInput(2);
    userPassword = readInput(1);
    CURRENT_USER = userName;

    if(validateAccess(userName,userPassword) == 1){
      /*correct combination of user and password given!*/

      /*log*/
      keepLog(0,CURRENT_USER);

      if(isSuperUser(userName) == 1){
        showMessages("SUPER","USUARIO");
        delay(2500);
        superUserOptions();
        return;
      }else{
        /*DOES NOTHING, ACCESS WAS ALREADY GRANTED!*/
        /*could activate a simple electric motor*/
        showMessages("ACESSO","AUTORIZADO");
        delay(2500);
        setLed(PIN_RED_LED,0,0);
        setLed(PIN_GREEN_LED,1000,1);
        setBuzzer(PIN_BUZZER,500,500);
        return;
      }

    }else{

      /*log*/
      keepLog(1,CURRENT_USER);

      showMessages("SENHA","INCORRETA");
      delay(2000);

      if(possibleRetries > 0){
        sprintf(text,"#TENT. REST. %d",possibleRetries);
        showMessages(text,"");
        delay(1000);
        possibleRetries--;
        goto access_validation;
      }else{

        /*log*/
        keepLog(5,CURRENT_USER);

        showMessages("#TENT ESGOTADO","");
        delay(2000);
        showMessages("MODO VIOLADO","");
        delay(1000);
        showMessages("3","");
        delay(500);
        showMessages("2","");
        delay(500);
        showMessages("1","");
        delay(500);
        showMessages("ACESSO","SUSPENSO");
        delay(500);
        /*calls up violationMode*/
        violationMode();
      }
    } // else externo
}

/*
NAME: menu
PURPOSE: Displays the options of the current state of the system
PARAMETERS: void.
RETURNS: void.
*/
void menu(){
  login();
}


/*
NAME: useInit
PURPOSE: controls any use other than the first
PARAMETERS: void.
RETURNS: void.
*/
void useInit(){

  showMessages("SISTEMA","REINICIADO");
  delay(2000);
  setLed(PIN_RED_LED,0,1);
  setLed(PIN_GREEN_LED,0,0);
  menu();

}

/*
NAME: Operation
PURPOSE: Controls the whole operation
PARAMETERS: flag of first use
RETURNS: void.
*/
void operation(int first_use){

  if(first_use == 1){
    firstUseInit();
  }else if(first_use == 0){
    useInit();
  }

}


void setup(){
/* Waits for serial to open */
 Serial.begin(9600);
 while (!Serial) {
   ; /* wait for serial port to connect. Needed for native USB port only*/
 }

/*necessary components (inputs and outputs) declarations*/
pinMode(btn1, INPUT);
pinMode(btn2, INPUT);
pinMode(btn3, INPUT);
pinMode(PIN_RED_LED, OUTPUT);
pinMode(PIN_GREEN_LED, OUTPUT);
pinMode(PIN_BUZZER,OUTPUT);
pinMode(bargraph[0], OUTPUT);
pinMode(bargraph[1], OUTPUT);
pinMode(bargraph[2], OUTPUT);
pinMode(bargraph[3], OUTPUT);
pinMode(bargraph[4], OUTPUT);
pinMode(bargraph[5], OUTPUT);
pinMode(bargraph[6], OUTPUT);
pinMode(bargraph[7], OUTPUT);
pinMode(bargraph[8], OUTPUT);
pinMode(bargraph[9], OUTPUT);

 /*prepares the environment*/
 EEPROMClear();

 /*creates the systems structures*/
 //setupSystem();
}

void loop(){
  /*starts the system's operation*/
  setupSystem();
  operation(FIRST_USE);
}
