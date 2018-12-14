/* Código para aprender a mexer na memória 
Dataflash da ATMEL, AT45DB161D. Inicialmente, 
ler o ID do fabricante e do dispositivo e posteriormente
escrever e ler bytes na memória. 

Arthur Moisés, 09h38, 20/07/2012 (início)
Todos os comandos a serem utilizados, implementados às 14:47 do dia 26/07/2012
// Parte do arquivo para receber dados do Processing e tratar:
*/

#include "Arduino.h"
#include "SoftwareSerial.h"

// Use pins 2 and 3 to talk to the GPS. 7 is the TX pin, 5 is the RX pin
SoftwareSerial mySerial = SoftwareSerial(7, 5); //  SoftwareSerial(RX, TX);  o pino TX do GPS SKYTREQ está ligado no pino 2 do Arduino

#define BUFFSIZ 40                // Tamanho do Buffer 31
#include <SPI.h>
boolean led = HIGH; // LED Para Toggle
int man; // variável pra armazenar o id do fabricante (manufacturer)
int id; // variável pra armazenar o id do dispositivo
unsigned char buff1read; // variável pra armazenar os dados da leitura do buffer1
unsigned char mread; // variável pra armazenar os dados da leitura da memória principal

char buffer[BUFFSIZ];        // string buffer for the sentence 
char *parseptr;              // a character pointer for parsing

char buffidx;  // variável pra armazenar o index do buffer
int index; // variável pra armazenar o índice das linhas que vem no arquivo txt
int velocidade; // variável pra armazenar a velocidade tratada no parser
int dir; // variável pra armazenar a direção tratada no parser
int idxBuff1 = 0;
int pgnum = 0; // número da página a ser gravado na dataflash
int pgsiz = 512; // tamanho da página na dataflash

boolean inicio = true;
boolean fim = false;
int i = 0;
int p = 0;
int l = 0;

// ###################################################################

long latitude, longitude; //variável pra armazenar longitude e latitude
uint8_t groundspeed, trackangle; // cria essas variáveis sem sinal do tamanho de 8 bits (armazena até 2^8)

// Seta o pino 10 para selecionar slave
// Na memória, pra cada opcode deve-se inicializar o /CS 
//como nível baixo

const int slaveSelectPin = 10; // Joga o /SS no pino 10


// ###################################### INÍCIO SETUP ############################################


void setup()
{
  pinMode(4,OUTPUT); // Pino para ligar o buzzer
  pinMode(6,OUTPUT); // led debug
  Serial.begin(9600);
  mySerial.begin(9600);
  pinMode(slaveSelectPin, OUTPUT);
  SPI.begin();
}

// ###################################### FIM SETUP ###############################################




// ###################################### INÍCIO LOOP #############################################


// ############################### CÓDIGO COM GRAVAÇÃO VIA PROCESSING ########################################

void loop()
{
  while(idxBuff1 < 504)
  {
     if (Serial.read() == 'k') // tirar os prints e ver se dá o mesmo problema de imprimir o 44 atrás do 22.
     {   // Deu outro problema, com atraso. Ou seja, o problema disso tudo é timming.
        readline(); // Pega os caracteres individuais que chegam e armazena em um array
        GravRamDataFlash(); // Faz parser no array e salva na RAM da Dataflash
        //i++; // Incrementa pra passar pra próxima linha
        //writeBuffer1ToMainMemory(pgnum);
        //mySerial.println(idxBuff1);
        //mySerial.println(index);
     }
  }
  zeraRest(); // Zera o fim de cada página (do 504 ao 512)
  
// ******** Visualizar escrita na RAM da dataflash *********    
//  for (int iz=0; iz < 512; iz++)
//  { 
//     readDataFromBuff1(iz);
//  }
// ******** Visualizar escrita na RAM da dataflash *********  

   writeBuffer1ToMainMemory(pgnum);

// ******** Visualizar escrita na página *********  

// if(millis() < 4000)
// {
// }
// else
// {
//   for (pgnum = 0 ; pgnum < 4096; pgnum++)
//   {
//      mySerial.print("Pagina: ");
//      mySerial.println(pgnum);
//      for (int ik=0; ik < 10; ik++)
//      { 
//         readFromMainMemory(pgnum,ik);
//         //mySerial.println(index);
//      } 
//   }
// }

// ******** Visualizar escrita na página *********  

  pgnum++;
  while (Serial.available() > 0)
  {
    Serial.read();
  }    
  idxBuff1 = 0;
  Serial.write('A');
  p++;
  mySerial.println(p);
  if (p==922) // valor antigo: 867
  {
    for (int pag = 923; pag <4096; pag++) // valor antigo: 868
    {
      pageErase(pag);
    }
    p = 0;
    digitalWrite(6,HIGH);
  }
}
  
//***********************************************************************************************************************************************
   // Código pra gravar a primeira linha, estilo ladyada:
   
   // Modelo de String que vai ser parseado aqui:
   // $1,46783572,23584042,5,90,1,84%
//    Serial.write('g');
//    delay(2000);
//    if(Serial.read() == 'k')
//    {
//      led = !led;
//      digitalWrite(6, led);
//    }
//}
//      if(Serial.available() > 0)
//      {
//        Serial.read();
//        if (mySerial.read() == 'G')
//        {
//          Serial.write('g');
//          while (idxBuff1 < 504)
//          {
//             if (Serial.read() == 'k')
//             {  
//                readline(); // Salva do buffer do Serial para um array de char, chamado buffer[BUFFSIZ]
//                GravRamDataFlash();
//                //Serial.write('g');
//              }
//          }
//        }
//        if (mySerial.read() == 'M')
//        {
          //Serial.write('w');
//          zeraRest();
//          idxBuff1 = 0;
//          writeBuffer1ToMainMemory(pgnum);  
//          delay(50);
//          pgnum++;
//          Serial.write('g');
//         while (Serial.available() > 0)
//          {
//            Serial.read();
//          }
//        }
//      }

//      fim = true;
//    }
//    else
//    {
//      Serial.write('f');
//    }    

//***********************************************************************************************************************************************
//        //################### Parte do código para leitura do buffer e da memória principal ##########
//        mySerial.println("");
//        mySerial.println("Inicio da leitura do buffer da dataflash:");
//        for (int iz=0; iz < 512; iz++)
//        { 
//            readDataFromBuff1(iz);
//        }
//        mySerial.println("Fim da leitura do buffer da dataflash:");
//        mySerial.println("Inicio da leitura da memoria principal da dataflash:");
//        for (pgnum = 0; pgnum < 40; pgnum++)
//        {
//          mySerial.print("Página: ");
//          mySerial.println(pgnum);
//          for (int ik=0; ik < 512; ik++)
//          { 
//              readFromMainMemory(pgnum,ik);
//          }   
//          while(Serial.available())
//          {
//            Serial.read();
//          }
//        }
//}

//// ############################### FUNÇÃO PRA ZERAR OS RESTANTE DOS BYTES DA DATAFLASH ################# 

void zeraRest()
{
  for (idxBuff1 = 504; idxBuff1 < 512;idxBuff1++)
  {
    writeDataToBuff1(idxBuff1,255);
  }
}

//// ############################### FUNÇÃO PRA ZERAR OS RESTANTE DOS BYTES DA DATAFLASH ################# 
        
//// ############################### FUNÇÃO DE PARSER DO BUFFER E ARMAZENAMENTO NA RAM DA DATAFLASH #################        
        
void GravRamDataFlash() // Lê o Array gerado pelo "readline()" e grava na RAM da dataflash
{ 
        
        parseptr = buffer; // iguala o ponteiro com a primeira posição de memória do array
        parseptr = strchr(parseptr, '$');
        // ############### Armazena o $ ####################
        writeDataToBuff1(idxBuff1,parseptr[0]);                       // Armazena o $ na RAM da Dataflash
        parseptr = buffer+1;                                          // pula pra próxima posição da String
        idxBuff1++;                                                   // pula pra próxima posição da RAM
      
        // ############## Armazena o index #################
        index = parsedecimal(parseptr);      
        writeDataToBuff1(idxBuff1,(index&0xff00)>>8);                 // Armazena o MSByte do index
        idxBuff1++;
        writeDataToBuff1(idxBuff1,(index&0xff));                      // Armazena o LSByte do index
        //parseptr++;
        parseptr = strchr(parseptr, ',');
        idxBuff1++;
      
        // ############## Armazena a vírgula ###############
        writeDataToBuff1(idxBuff1,parseptr[0]);                       // Armazena a , na RAM da Dataflash
        parseptr++;                                                   // pula pra próxima posição da String
        idxBuff1++;                                                   // pula pra próxima posição da RAM
      
        // ############## Armazena a Longitude #############
        longitude = parsedecimal(parseptr);                           // Armazena a próxima sequência de dígitos em longitude

        writeDataToBuff1(idxBuff1,(longitude & 0xff000000)>>24);                                 // Armazena o 4° Byte (MSB) da longitude
        idxBuff1++;
        writeDataToBuff1(idxBuff1,(longitude & 0xff0000)>>16);                                 // Armazena o 3° Byte da longitude
        idxBuff1++;
        writeDataToBuff1(idxBuff1,(longitude & 0xff00)>>8);                                    // Armazena o 2° Byte da longitude
        idxBuff1++;
        writeDataToBuff1(idxBuff1,(longitude & 0xff));                                // Armazena o 1° Byte (LSB) do index
        //parseptr++;                                                   // pula pra próxima posição da String
        parseptr = strchr(parseptr, ',');
        idxBuff1++;                                                   // pula pra próxima posição da RAM
      
        // ############## Armazena a vírgula ###############
        writeDataToBuff1(idxBuff1,parseptr[0]);                      // Armazena a , na RAM da Dataflash
        parseptr++;                                                  // pula pra próxima posição da String
        idxBuff1++;                                                  // pula pra próxima posição da RAM
       
        // ############## Armazena a Latitude #############
        latitude = parsedecimal(parseptr);                           // Armazena a próxima sequência de dígitos em longitude
        writeDataToBuff1(idxBuff1,(latitude & 0xff000000)>>24);        // Armazena o 4° Byte (MSB) da longitude
        idxBuff1++;
        writeDataToBuff1(idxBuff1,(latitude & 0xff0000)>>16);          // Armazena o 3° Byte da longitude
        idxBuff1++;
        writeDataToBuff1(idxBuff1,(latitude & 0xff00)>>8);             // Armazena o 2° Byte da longitude
        idxBuff1++;
        writeDataToBuff1(idxBuff1,(latitude & 0xff));                  // Armazena o 1° Byte (LSB) do index
        //parseptr++;                                                  // pula pra próxima posição da String
        parseptr = strchr(parseptr, ',');
        idxBuff1++;                                                  // pula pra próxima posição da RAM
      
        // ############## Armazena a vírgula ###############
        writeDataToBuff1(idxBuff1,parseptr[0]);                      // Armazena a , na RAM da Dataflash
        parseptr++;                                                  // pula pra próxima posição da String
        idxBuff1++;                                                  // pula pra próxima posição da RAM
      
        // ############## Armazena o char de Tipo de Baliza ###############
        writeDataToBuff1(idxBuff1,parseptr[0]);                      // Armazena a , na RAM da Dataflash
        //parseptr++;                                                  // pula pra próxima posição da String
        parseptr = strchr(parseptr, ',');
        idxBuff1++;                                                  // pula pra próxima posição da RAM
      
        // ############## Armazena a vírgula ###############
        writeDataToBuff1(idxBuff1,parseptr[0]);                      // Armazena a , na RAM da Dataflash
        parseptr++;                                                  // pula pra próxima posição da String
        idxBuff1++;                                                  // pula pra próxima posição da RAM
      
        // ############## Armazena a velocidade ###############
        velocidade = parsedecimal(parseptr);    
        writeDataToBuff1(idxBuff1,(velocidade&0xFF00)>>8);           // Armazena o MSByte da velocidade
        idxBuff1++;
        writeDataToBuff1(idxBuff1,(velocidade&0xFF));                // Armazena o LSByte da velocidade
        //parseptr++;
        parseptr = strchr(parseptr, ',');
        idxBuff1++;
      
        // ############## Armazena a vírgula ###############
        writeDataToBuff1(idxBuff1,parseptr[0]);                      // Armazena a , na RAM da Dataflash
        parseptr++;                                                  // pula pra próxima posição da String
        idxBuff1++;                                                  // pula pra próxima posição da RAM
      
        // ############## Armazena o char de Tipo de direção ###############
        writeDataToBuff1(idxBuff1,parseptr[0]);                      // Armazena a , na RAM da Dataflash
        //parseptr++;                                                  // pula pra próxima posição da String
        parseptr = strchr(parseptr, ',');
        idxBuff1++;                                                  // pula pra próxima posição da RAM
      
        // ############## Armazena a vírgula ###############
        writeDataToBuff1(idxBuff1,parseptr[0]);                      // Armazena a , na RAM da Dataflash
        parseptr++;                                                  // pula pra próxima posição da String
        idxBuff1++;                                                  // pula pra próxima posição da RAM
      
        // ############## Armazena a direção ###############
        dir = parsedecimal(parseptr);    
        writeDataToBuff1(idxBuff1,(dir&0xff00)>>8);                 // Armazena o MSByte da dir
        idxBuff1++;
        writeDataToBuff1(idxBuff1,(dir&0xff));                      // Armazena o LSByte da dir
        //parseptr=parseptr+2; // avança 2 bytes até chegar na %
        parseptr = strchr(parseptr, '%');
        idxBuff1++;
      
        // ############### Armazena o # ####################
        writeDataToBuff1(idxBuff1,parseptr[0]);                       // Armazena o $ na RAM da Dataflash
        parseptr = buffer+1;                                          // pula pra próxima posição da String
        idxBuff1++;                                                   // pula pra próxima posição da RAM
        //buffidx=0;
}

//// ############################### FUNÇÃO DE PARSER DO BUFFER E ARMAZENAMENTO NA RAM DA DATAFLASH #################          

// ################ Sequência de código para testar as funcionalidades da memória, LED vermelho e buzzer ###########################
      
//      byte arrive = Serial.read();
//      if(arrive == 'I') // Lê o id do fabricante e do dispositivo
//      {
//        readManDevID();  
//      }
//      else if(arrive == 'B') // Grava o dado N no endereço adb (0 a 511) do Buffer1
//      {
//        digitalWrite(6,HIGH); //led debug
//        for(int adb = 0; adb < 512; adb++)
//        {
//          writeDataToBuff1(adb,'$');
//        }
//        digitalWrite(6,LOW); // led debug
//      }
//      
//      else if(arrive == 'R') // Lê o dado no endereço add do Buffer1
//      {
//        for(int add = 0; add < 512; add++)
//        {
//          readDataFromBuff1(add);
//        }
//      } 
//      else if(arrive == 'M') // Grava a página inteira do buffer na página N (0 a 4095) da memória principal
//      {
//        digitalWrite(6,HIGH);
//        writeBuffer1ToMainMemory(1); // Especifica em qual das 4096 páginas escrever
//        digitalWrite(6,LOW);
//      }
//      else if(arrive == 'F') // Lê os dados gravado no endereço adm da página N.
//      {
//        for(int adm = 0; adm < 512; adm++)
//        {
//          readFromMainMemory(0, adm);
//        }
//      }
//      else if(arrive == 'O') // Grava os dados da página N no Buffer1
//      {
//        digitalWrite(6,HIGH);
//        mainMemoryToBuffer1(5); 
//        digitalWrite(6,LOW);
//      }
//      else if(arrive == 'Z') // Liga o buzzer
//      {
//        // PI PI PI PI PI PI
//        for(int w=0; w<100; w++)
//        {
//            analogWrite(9,255);
//            delay(100);
//            analogWrite(9,0);
//            delay(100);
//        }
//      }
//      else if(arrive == 'L') // Acende ou apaga o LED Vermelho
//      {
//        digitalWrite(4,led);
//        led = !led;
//      }      

// ################ Sequência de código para testar as funcionalidades da memória, LED vermelho e buzzer ###########################



// ###################################### FIM LOOP ##############################################


// ########################################################## INÍCIO FUNÇÕES PARA PROCESSAMENTO ##############################################################

uint32_t parsedecimal(char *str) {
  uint32_t d = 0;
  
  while (str[0] != 0) {
   if ((str[0] > '9') || (str[0] < '0'))
     return d;
   d *= 10;  // d = d * 10
   d += str[0] - '0'; // d = d + str[0] - '0'
   str++; // str = str + 1
  }
  return d;
}

void readline(void) {
  char c; // cria variável c de 8 bits
  
  buffidx = 0; // start at begninning
  while (1) {
      c=Serial.read(); // Armazena a linha toda na variável c
      if (c == -1) // Se por acaso c == -1, sai do loop.
        continue;
      //mySerial.print(c);
      if (c == '\n') // Se houver "new line", sai do loop
        continue;
      if ((buffidx == BUFFSIZ-1) || (c == '\r')) { // se tiver chegado a BUFFSIZ 31 ou carriage return (fim da linha) volta a buffer[buffidx]=0.
        //buffer[buffidx]= c;
        //buffer[buffidx] = 0;
        buffidx = 0;
        return;
      }
      buffer[buffidx++]= c;
      //buffidx++;
  }
}

// ###################################### FIM FUNÇÕES PARA PROCESSAMENTO ######################################


// ###################################### FUNÇÕES PARA MANIPULAÇÃO DA DATAFLASH ###############################

// ############################################## VERIFICA SE DISPOSITIVO ESTÁ OCUPADO ########################
uint8_t readStatusBusy()
{
  uint8_t tmp;
  digitalWrite(slaveSelectPin,LOW); // inicialização da memória através de /CS
  SPI.transfer(0xD7); //opcode para ler o nome do fabricante e informações do dispositivo
  tmp = SPI.transfer(0x00);
  digitalWrite(slaveSelectPin,HIGH);  
  return(tmp & 0x80);
}
// ############################################## VERIFICA SE DISPOSITIVO ESTÁ OCUPADO ########################

// ############################################## READ MANUFACTURER AND DEVICE ID (I) #############################
// Lê o ID do fabricante e do dispositivo
// 1F (31 dec) = ATMEL
// 26H (38 dec) = Dataflash 16Mbits
void readManDevID()
{
  digitalWrite(slaveSelectPin,LOW); // inicialização da memória através de /CS
  SPI.transfer(0x9F); //opcode para ler o nome do fabricante e informações do dispositivo
  man = SPI.transfer(0xFF);
  id = SPI.transfer(0xFF);
  mySerial.print("Man: ");
  mySerial.println(man);
  mySerial.print("ID: ");
  mySerial.println(id);
  digitalWrite(slaveSelectPin,HIGH);     
}
// ############################################## READ MANUFACTURER AND DEVICE ID (I) #############################



// ############################################## WRITE DATA TO BUFF1 (B) #########################################

void writeDataToBuff1(uint16_t page_addr, uint8_t data) 
{  
  digitalWrite(slaveSelectPin,LOW); // inicialização da memória através de /CS
  SPI.transfer(0x84); //opcode para gravar no Buffer 1
  SPI.transfer(0x00);		
  SPI.transfer((unsigned char)(page_addr>>8));  //upper part of internal buffer address
  SPI.transfer((unsigned char)(page_addr));	 //lower part of internal buffer address
  SPI.transfer(data);
  //Serial.println(data);	
  while(!readStatusBusy()) // Espera até que a escrita termine
  {}
  //delay(10);
  digitalWrite(slaveSelectPin,HIGH); // finalização da escrita através de /CS
}
// ############################################## WRITE DATA TO BUFF1 (B) #########################################



// ############################################## READ DATA FROM BUFF1 (R) ########################################

void readDataFromBuff1(uint16_t page_addr) 
{
  digitalWrite(slaveSelectPin,LOW); // inicialização da memória através de /CS
  SPI.transfer(0xD4);                            // opcode para leitura do buffer1
  SPI.transfer(0x00);				 //don't cares
  SPI.transfer((unsigned char)(page_addr>>8));  //upper part of internal buffer address
  SPI.transfer((unsigned char)(page_addr));	 //lower part of internal buffer address
  SPI.transfer(0x00);                            //don't cares
  // Leitura e escrita no Serial Monitor
  buff1read = SPI.transfer(0x00);		 //read data byte
  mySerial.print("Buff1ReadAddress: ");
  mySerial.print(page_addr);
  mySerial.print(" = ");
  mySerial.println(buff1read,DEC);
  digitalWrite(slaveSelectPin,HIGH);  // finalização da memória através de /CS
}

// ############################################## READ DATA FROM BUFF1 (R) #########################################



// ############################################## WRITE BUFF1 TO MAIN MEMORY (M) ###################################

void writeBuffer1ToMainMemory(uint16_t page_addr) 
{ 
  digitalWrite(slaveSelectPin,LOW); // inicialização da memória através de /CS
  SPI.transfer(0x83); //opcode para gravar na Main Memory
  SPI.transfer((unsigned char)(page_addr >> 6));
  SPI.transfer((unsigned char)(page_addr << 2));
  SPI.transfer(0x00);	// don´t care bytes
  digitalWrite(slaveSelectPin,HIGH);
  digitalWrite(slaveSelectPin,LOW);
  while(!readStatusBusy()) // Espera até que a escrita termine
  {}
  digitalWrite(slaveSelectPin,HIGH); // finalização da memória através de /CS
}

// ############################################## WRITE BUFF1 TO MAIN MEMORY (M) ####################################



// ############################################## READ FROM MAIN MEMORY (F) #########################################

void readFromMainMemory(uint16_t page_addr, uint16_t offset)
{
  digitalWrite(slaveSelectPin,LOW); // inicialização da memória através de /CS  
  SPI.transfer(0xD2); //opcode para ler a Main Memory
  SPI.transfer((unsigned char)(page_addr >> 6));
  SPI.transfer((unsigned char)((page_addr << 2) | (offset >> 8)));
  SPI.transfer((unsigned char)(offset & 0xff));
  SPI.transfer(0x00);        // don't care byte
  SPI.transfer(0x00);        // don't care byte
  SPI.transfer(0x00);        // don't care byte
  SPI.transfer(0x00);        // don't care byte
  //Leitura e escrita no Serial Monitor
  mread = SPI.transfer(0x00);		 //read data byte
  mySerial.print("MemoryAdress: ");
  mySerial.print(offset);
  mySerial.print(" = ");
  mySerial.println(mread,DEC);
  while(!readStatusBusy()) // Espera até que a escrita termine
  {}
  digitalWrite(slaveSelectPin,HIGH); // finalização da memória através de /CS
}

// ############################################## READ FROM MAIN MEMORY (F) #########################################


// ############################################## MAIN MEMORY TO BUFFER1 (N) #########################################

void mainMemoryToBuffer1(uint16_t page_addr)
{
  digitalWrite(slaveSelectPin,LOW); // inicialização da memória através de /CS  
  SPI.transfer(0x53); //opcode para transferir da Main Memory para Buffer1
  SPI.transfer((unsigned char)(page_addr >> 7));
  SPI.transfer((unsigned char)(page_addr << 1));
  SPI.transfer(0x00);	// don´t care bytes
  digitalWrite(slaveSelectPin,HIGH);
  digitalWrite(slaveSelectPin,LOW);
  while(!readStatusBusy()) // Espera até que a escrita termine
  {}
  digitalWrite(slaveSelectPin,HIGH); // finalização da memória através de /CS
}

// ############################################## MAIN MEMORY TO BUFFER1 (N) #########################################

// ############################################## PAGE ERASE ########################################
void pageErase(uint16_t page_addr)
{
  digitalWrite(slaveSelectPin,LOW); // inicialização da memória através de /CS  
  SPI.transfer(0x81); //opcode para apagar a página
  SPI.transfer((uint8_t)(page_addr >> 6));
  SPI.transfer((uint8_t)(page_addr << 2));
  SPI.transfer(0x00);
  digitalWrite(slaveSelectPin,HIGH);
  digitalWrite(slaveSelectPin,LOW);
  while(!readStatusBusy()) // Espera até que o apagamento termine
  {}
}
  
// ############################################## PAGE ERASE #########################################
