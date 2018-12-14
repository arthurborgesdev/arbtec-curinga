// A simple sketch to read GPS data and parse the $GPRMC string 
// see http://www.ladyada.net/make/gpsshield for more info

// If using Arduino IDE prior to version 1.0,
// make sure to install newsoftserial from Mikal Hart
// http://arduiniana.org/libraries/NewSoftSerial/

#include "Arduino.h"
#include "SoftwareSerial.h"
// Use pins 2 and 3 to talk to the GPS. 2 is the TX pin, 3 is the RX pin
SoftwareSerial mySerial = SoftwareSerial(2, 3); // o pino TX do GPS SKYTREQ está ligado no pino 2 do Arduino
#include <SPI.h>
#define GPSRATE 9600 // Frequência do GPS SKYTREQ
// The buffer size that will hold a GPS sentence. They tend to be 80 characters long
// so 90 is plenty.
#define BUFFSIZ 90 // plenty big --> 

const int slaveSelectPin = 10; // Joga o /SS no pino 10

// global variables
char buffer[BUFFSIZ];        // string buffer for the sentence 
char *parseptr;              // a character pointer for parsing
char buffidx;                // an indexer into the buffer
//char ponto[24];

int tempo_Bal = 10;    // Tempo para começar a notificar antes de chegar na baliza
int tam_Angulo = 45;   // Margem de erro pra balizar ângulo
int tam_Baliza = 0;    // Margem de erro pra balizar coordenadas - é mudado mais pra frente
int ang;               // Valor do ângulo armazenado na memória Dataflash
int tipoDir;           // Tipo de Direção armazenada na memória Dataflash 
int veloc;             // velocidade armazenada na memória Dataflash 
int tipoBal;           // Tipo de Baliza armazenada na memória Dataflash

long auxLat;
long auxLon;           // variável auxiliar para leitura da Longitude           
long longit = 0;       // Longitude armazenada na memória Dataflash
long latit = 0;        // Latitude armazenada na memória Dataflash 

int idx_atual = 0;     // Fica gravada o último ponto em que ocorre o primeiro match (idx_atual = idxLong) [USAR PARA EXPANDIR INTERVALO]
int idx = 0;           // Index do ReadDFPoint()
int idxLong = 0;       // Index de ReadLong()
//int idxLat = 0;      // Index de ReadLat()  - Não existe mais

//int idx_perm = 0;
//int add_perm = 0;
int pg_perm = 0;        // pg_perm = idx_perm/21  - Armazena a página onde começou a aparecer as  balizar [USAR PARA EXPANDIR INTERVALO]

//int idx_df = 0;        // Index do ReadDFPoint() agora é 'idx' - não existe mais 
int add_df = 0;          // 
int pg_df = 0;           //

// *****Parte do código para acender o LED mediante balizamento*****
int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated
// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 1000;           // interval at which to blink (milliseconds)
// *****Parte do código para acender o LED mediante balizamento*****

int dif = 0;  // Diferença entre pg_atual e pg_perm
boolean inicio = true;
boolean prime = true;
int i; // variável do for pra trabalhar com loop
//int count = 0; //variável pra contar quantos index se passaram que estão dentro do intervalo. - não existe mais

int pg_atual = 0;      // = idx_atual/21 - calcula a última página em que ocorreu idxLong
int pgnum = 0;         // armazena o número da página da Dataflash
int addLong = 0;       // endereço de ReadLong()
//int addLat = 0;      // endereço de ReadLat() - não existe mais
//int add = 0;         // Descomentar para rodar funções mais embaixo, de SO e leitura da memória
//int ad = 0;          // Não existe mais
//long l = 0;          // Não existe mais 

boolean led = HIGH; // LED Para Toggle
int man; // variável pra armazenar o id do fabricante (manufacturer)
int id; // variável pra armazenar o id do dispositivo
unsigned char buff1read; // variável pra armazenar os dados da leitura do buffer1
unsigned char mread; // variável pra armazenar os dados da leitura da memória principal


// The time, date, location data, etc.
uint8_t hour, minute, second, year, month, date; // cria essas variáveis sem sinal do tamanho de 8 bits (armazena até 2^8)
float latitude, longitude, groundspeed; 
long latitude2, longitude2;
uint8_t trackangle; // cria essas variáveis sem sinal do tamanho de 8 bits (armazena até 2^8)
char latdir, longdir; // variáveis de 8 bits
char status; // variável de 8 bits


void setup() 
{ 
  pinMode(6,OUTPUT); // led debug
  pinMode(8, OUTPUT);  // pino pra ligar o VCC do GPS pra permitir reset
  pinMode(9, OUTPUT); // pino do buzzer
  pinMode(4, OUTPUT); // led vermelho, que acompanha o buzzer
  Serial.begin(9600); // Para comunicação com o computador
  mySerial.begin(GPSRATE); // Começa a comunicação com o GPS usando a porta serial virtual
  //Serial.println("GPS parser"); // Escreve "GPS parser" na porta serial
  pinMode(slaveSelectPin, OUTPUT);
  SPI.begin();
} 
 
 
void loop() 
{ 
  while(latitude2 == 0) // Roda essa função até o GPS estabilizar
  {
      readline(); // Lê a linha que chega do GPS e armazena em um array
      parseGPS(); // Pega os valores do Array e salva em variáveis específicas (latitude2)
      //Serial.println(latitude2);
      delay(1);
      //Serial.print("Velocidade GPS: ");
      //Serial.println(groundspeed);
  }
  if(millis() > 1000) // Após a primeira leitura, a segunda leitura será regida por essa parte, e não pela parte do while acima
  {
    readline();
    parseGPS();
    //Serial.println("Leu GPS");
  }
  // valores arbitrários pra testar balizamento com o dispositivo sem movimento
//  latitude2 = 16553063; 
//  longitude2 = 49153063;
  // imprime Longitude
  tam_Baliza = int((groundspeed/3.6)*tempo_Bal*20) + 100;
  //Serial.print("Longitude: "); 
  //Serial.println(longitude2);
  
  // *************** PARTE 1 - LEITURA E COMPARAÇÃO COMPLETA DA MEMÓRIA ******************
  if (prime == true)
  {
    // Antigo valor era de 867
    while(pgnum < 922) // Varre a memória até a última posição gravada com linhas, até achar um valor parecido com o dado que chega do GPS
    {    
         // Imprime a página
         //Serial.print("Pagina: ");
         //Serial.println(pgnum);
         // Enquanto a 'addLong' (variável que conta a posição de memória) for menor que 504 (tamanho final da 21ª linha por página) 
         while (addLong < 504)
         {      
                // Escreve o endereço na memória (de 0 a 504)
                //Serial.print("addLong: ");
                //Serial.println(addLong);
                readLong(); // Lê apenas o index e a longitude de cada linha (usado durante a primeira vez, pra economizar tempo)
                //Serial.print("longit: ");
                //Serial.println(longit); // Longitude armazenada na memória
                if ((longitude2 >= longit - tam_Baliza) && (longitude2 <= longit + tam_Baliza)) // Se a longitude estiver dentro do intervalo
                {
                  if (inicio == true) // Se for a primeira linha a ser achada
                  {
                    //idx_perm = idxLong; // O index da primeira linha recebe o número da linha (de 0 a 18207)
                    pg_perm = idxLong/21; // calcula o número da página (idx_perm div 21)
                    //add_perm = ((idxLong % 21)-1) * 24; // calcula o endereço inicial ((idx_perm mod 21)-1) * 24
                    // Parte para garantir que o add_perm não seja negativo
                    //if (add_perm < 0)
                    //{
                    //   add_perm = add_perm + 24;
                    //}     
                    inicio = false; // Faz com que essa parte não se repita mais
                  }
                  idx_atual = idxLong; // armazena o index atual para depois imprimí-lo
                  pg_atual = idx_atual / 21; // calcula e armazena a página atual
                  //add = ((idx_atual % 21)-1) * 24; // calcula e armazena o endereço da memória
                  // Parte para garantir que o add não seja negativo
                  //if (add < 0)
                  //{
                   //    add = add + 24;
                 // } 
                  // add recebe a posição inicial da última linha lida, pronto para começar uma nova leitura e ter acesso aos mesmos dados
                  //count++; // Conta quantas linhas estão dentro do intervalo
                  
                  // Imprime as linhas que estão dentro do intervalo
                  //Serial.println("***********"); 
                  //Serial.print("idx Longitude: ");
                  //Serial.println(idxLong);
                  //Serial.print("Longitude: ");
                  //Serial.println(longit);
                  //Serial.print("Pag Atual: ");
                  //Serial.println(pg_atual);
                  //Serial.print("Endereco: ");
                  // Serial.println(add);
                  //Serial.println("***********");   
                  // Serial.print("idx_perm: ");
                  // Serial.println(idx_perm);
                  //Serial.println("***********"); 
                }      
                // Não há necessidade de incrementar addLong ou add, pois eles se incrementam dentro das funções readLong() e readDFPoint() respectivamente
         }
         addLong = 0; // Faz o addLong igual a 0 para começar do zero na próxima página
         pgnum++; // Incrementa a página
    }   
  }
  prime = false;
    // *************** PARTE 1 - LEITURA E COMPARAÇÃO COMPLETA DA MEMÓRIA ******************
  
  
  // Imprime o idx_atual
//  Serial.print("Idx_atual: ");
//  Serial.println(idx_atual);
//  // Imprime o count
//  Serial.print("Count: ");
//  Serial.println(count); 
  
  //pgnum = pg_perm; // Faz a página atual ser a página onde começou o match das longitudes
  // imprime pg_perm
//  Serial.print("Pg_perm: ");
//  Serial.println(pg_perm);
//  // imprime add_perm
//  Serial.print("Add_perm: ");
//  Serial.println(add_perm);

  // *************** PARTE 2 - LEITURA E COMPARAÇÃO PARCIAL DA MEMÓRIA ******************
  add_df = 0;  // Zera o endereço de memória para começar a pesquisar desde o começo da página
  pg_df = pg_perm; // Faz a página inicial da busca parcial ser a mesma da primeira em que foi achado o
  //i = idx_atual - count + 1;
  //while(i < idx_atual) // Para i = 14000 - 50+1 até o idx_atual = 14000, onde parou
  dif = pg_atual - pg_perm;
  for (i = pg_perm; i < pg_atual; i++)
  {   
    if(longitude2 >= longit - tam_Baliza)
    {
      pg_perm++;
      pg_atual++;
      if(pg_atual >= 922 || pg_perm >= 922)
      {  
        pg_atual = 922;
        pg_perm = 922 - dif;
      }
    if(longitude2 <= longit + tam_Baliza)
    {
      if(pg_atual <= 0 || pg_perm <= 0)
      {  
        pg_perm = 0;
        pg_perm = 0 + dif;
      }
    }
    while(add_df < 504)
    {
      readDFPoint();
      //Serial.println("redou");
      //Serial.print("Longitude2: ");
      //Serial.println(longitude);
      //Serial.print("longit: ");
      //Serial.println(longit);
      if (trackangle < 45 && ang >= 315 && ang < 360)
      {
          ang = (ang - 360);  
      }
      if ((longitude2 >= longit - tam_Baliza) && (longitude2 <= longit + tam_Baliza)) // Se a longitude estiver dentro do intervalo
      {
        //Serial.println("****Longitude legal");
        //Serial.print("Longit: ");
        //Serial.println(longit);
        //Serial.print("Latit: ");
        //Serial.println(latit);
        if ((latitude2 >= latit - tam_Baliza) && (latitude2 <= latit + tam_Baliza)) // Se a latitude estiver dentro do intervalo
        {
          if ((trackangle >= ang - tam_Angulo) && (trackangle <= ang + tam_Angulo))
          {
              if (tipoDir = 1 || tipoDir == 5 || tipoDir == 11)   // tipos de balizas que consideram a velocidade
              {      
                  if (groundspeed >= veloc)
                  {
                       //Serial.println("Latitude legal");
                       Buzzer();
                       //Serial.print("Velocidade: ");
                       //Serial.println(veloc);
                       //Serial.print("Angulo: ");
                       //Serial.println(ang);
                  }
                  else
                  {
                      toggleLED();
                  }
              }
              else
              {
                  toggleLED();
              }
          }    
        }        
      }
    }
    pg_df++;
    add_df = 0;  
  }
}  
  // *************** PARTE 2 - LEITURA E COMPARAÇÃO PARCIAL DA MEMÓRIA ******************
    
  //
  
  // Nesse ponto, já temos o index em que a longitude recebida pelo GPS parece com alguma armazenada na memória.
  
  // O que precisa ser feito agora é uma nova leitura, a partir do index identificado, para cima e para baixo,
  // comparando latitude e longitude, com as que chegam do GPS, com o objetivo de ver se tanto a longitude quanto a latitude estão 
  // dentro da faixa. Se tiverem, pisca o LED e verifica se groundspeed > veloc. Se sim, toca o buzzer. Tem que comparar o ângulo também.
  
              
                // Armazena o index -> idx_atual = idx;
                // Armazena a página -> pg_atual = pgnum;
                // Armazena o add -> Fazer o mod pra saber a baliza que parou
                // Fazer vasculhamento baseado em mod e div: index / 21 = página
                // index % 21 = ponto da página, de 0 a 21. Para chegar no ponto, basta multiplicar 21 pelo ponto. 
                /*
                Exemplo: $14453, 49270870, 16681679, 11, 50, 1, 298%
                
                pg_atual = index / 21 = 14453 / 21 = 688.
                posicao_atual = index % 21 = 14453 % 21 = 5.
                add = endereco_inicial = posicao_atual * 21 = 5*21 = 105. 
                Agora, ao fazer a leitura a partir de 105, o programa chegará justamente em $14453, 49270870, 16681679, 11, 50, 1, 298%,
                para poder ler os outros parâmetros, como latitude, velocidade e ângulo.
             
                Para aumentar ou diminuir as posições, basta   
                
                
                */
  
  digitalWrite(6,HIGH);
  //Serial.println("Terminou");
  //delay(1000);
}
//              ReadLat();
//              if ((latitude2 >= latit - tam_Baliza) && (latitude2 <= latit + tam_Baliza))
//              {
//                Buzzer();
//                digitalWrite(4, HIGH);
//              }
//            }
//            //add++;
//       }
//       add = 0;
//       pgnum++;
//       Serial.println(pgnum);
//  }
//  pgnum=0;
//}


//void VerifLat()
//{
//  while(pgnum < 867) // Varre a memória até achar um valor parecido com o dado que chega do GPS
//  {
//       while (addLong < 504)
//       {
//            readLong();// Lê a memória e armazena somente a longitude
//            if ((longitude2 >= longit - tam_Baliza) && (longitude2 <= longit + tam_Baliza)) // Se a longitude estiver dentro do intervalo
//            {
//              VerifLat();
//              break;
//            }
//       }
//       pgnum++;
//  }
//}

// Pisca o LED transparente 
void toggleLED()
{
//  unsigned long currentMillis = millis();
//  if(currentMillis - previousMillis > interval) 
//  {
//       // save the last time you blinked the LED 
//       previousMillis = currentMillis;   
//       // if the LED is off turn it on and vice-versa:
//       if (ledState == LOW)
//          ledState = HIGH;
//       else
//          ledState = LOW;
//       // set the LED with the ledState of the variable:
//       digitalWrite(4, ledState);
// }
  for(int j=0; j<3; j++)
   {
       digitalWrite(4,HIGH);
       //analogWrite(9,255);       
       delay(100);
       digitalWrite(4,LOW);
       //analogWrite(9,0);
       delay(100);
   }


}

// Toca a buzina quando houver velocidade envolvida (tipoBal == 1,5 ou 11)
void Buzzer()
{
// PI PI PI PI PI PI
   for(int w=0; w<3; w++)
   {
       digitalWrite(4,HIGH);
       analogWrite(9,255);       
       delay(100);
       digitalWrite(4,LOW);
       analogWrite(9,0);
       delay(100);
   }
}
  
// ####################### FUNÇÃO PARA LER OS DADOS ENTRE $ E % DA DATAFLASH ###########################

void readLong() // Lê idxLong e Longitude, armazenando-os em variáveis
{
  // Leitura de index Longitude(idxLong)
  addLong++; // Pula o $ e chega no primeiro byte de index (MSB)
  readFromMainMemory(pgnum, addLong); // mread = MSB index
//Serial.print("Index cru MSB: ");
//Serial.println(mread);
  idxLong = mread<<8; // converte mread pra int e armazena em idx
  addLong++;  // Pula para o segundo byte de index (LSB)
  readFromMainMemory(pgnum, addLong); // mread = LSB index
//Serial.print("Index cru LSB: ");
//Serial.println(mread);
  idxLong = idxLong + mread;
//Serial.print("Index: ");
//Serial.println(idx);
  
  addLong = addLong+2; // chega na vírgula e pula ela
  readFromMainMemory(pgnum, addLong); // mread = 1° MSB da Longitude
//Serial.print("Longitude crua MSB 1: ");
//Serial.println(mread);
  auxLon = (long int)mread; // Variável auxiliar para dessobrecarregar a RAM
  auxLon = auxLon << 24;
  longit = auxLon;
  addLong++; 
  readFromMainMemory(pgnum, addLong);
//Serial.print("Longitude crua MSB 2: ");
//Serial.println(mread);
  auxLon = (long int)mread; // Variável auxiliar para dessobrecarregar a RAM
  auxLon = auxLon << 16;
  longit = longit + auxLon; 
  addLong++;
  readFromMainMemory(pgnum, addLong);
//Serial.print("Longitude crua LSB 2: ");
//Serial.println(mread);
  auxLon = (long int)mread; // Variável auxiliar para dessobrecarregar a RAM
  auxLon = auxLon << 8;
  longit = longit + auxLon;
  addLong++;
  readFromMainMemory(pgnum, addLong);
//Serial.print("Longitude crua LSB 1: ");
//Serial.println(mread);
  auxLon = (long int)mread; // Variável auxiliar para dessobrecarregar a RAM
  longit = longit + auxLon;
//Serial.print("Longitude: ");
//Serial.println(longit);
  addLong = addLong + 17;
  //Serial.println(addLong);
}

void readDFPoint()
{
          // Leitura de index (idx)
          add_df++; // Pula o $ e chega no primeiro byte de index (MSB)
          readFromMainMemory(pg_df, add_df); // mread = MSB index
//          Serial.print("Index cru MSB: ");
//          Serial.println(mread);
          idx = mread<<8; // converte mread pra int e armazena em idx
          add_df++;  // Pula para o segundo byte de index (LSB)
          readFromMainMemory(pg_df, add_df); // mread = LSB index
//          Serial.print("Index cru LSB: ");
//          Serial.println(mread);
          idx = idx + mread;
//          Serial.print("Index: ");
//          Serial.println(idx);
           
          // Leitura de longitude (longit)
          add_df = add_df+2; // Pula a vírgula e passa para a Longitude
          readFromMainMemory(pg_df, add_df); // mread = 1° MSB da Longitude
//          Serial.print("Longitude crua MSB 1: ");
//          Serial.println(mread);
          auxLon = (long int)mread; // Variável auxiliar para dessobrecarregar a RAM
          auxLon = auxLon << 24;
          longit = auxLon;
          add_df++; 
          readFromMainMemory(pg_df, add_df);
//          Serial.print("Longitude crua MSB 2: ");
//          Serial.println(mread);
          auxLon = (long int)mread; // Variável auxiliar para dessobrecarregar a RAM
          auxLon = auxLon << 16;
          longit = longit + auxLon; 
          add_df++;
          readFromMainMemory(pg_df, add_df);
//          Serial.print("Longitude crua LSB 2: ");
//          Serial.println(mread);
          auxLon = (long int)mread; // Variável auxiliar para dessobrecarregar a RAM
          auxLon = auxLon << 8;
          longit = longit + auxLon;
          add_df++;
          readFromMainMemory(pg_df, add_df);
//          Serial.print("Longitude crua LSB 1: ");
//          Serial.println(mread);
          auxLon = (long int)mread; // Variável auxiliar para dessobrecarregar a RAM
          longit = longit + auxLon;
//          Serial.print("Longitude: ");
//          Serial.println(longit);
          
          // Leitura de latitude (latit)
          add_df = add_df+2; // Pula a vírgula e passa para a Latitude
          readFromMainMemory(pg_df, add_df); // mread = 1° MSB da Latitude
//          Serial.print("Latitude crua MSB 1: ");
//          Serial.println(mread);
          auxLat = (long int)mread; // Variável auxiliar para dessobrecarregar a RAM
          auxLat = auxLat << 24;
          latit = auxLat;
          add_df++; 
          readFromMainMemory(pg_df, add_df);
//          Serial.print("Latitude crua MSB 2: ");
//          Serial.println(mread);
          auxLat = (long int)mread; // Variável auxiliar para dessobrecarregar a RAM
          auxLat = auxLat << 16;
          latit = latit + auxLat; 
          add_df++;
          readFromMainMemory(pg_df, add_df);
//          Serial.print("Latitude crua LSB 2: ");
//          Serial.println(mread);
          auxLat = (long int)mread; // Variável auxiliar para dessobrecarregar a RAM
          auxLat = auxLat << 8;
          latit = latit + auxLat;
          add_df++;
          readFromMainMemory(pg_df, add_df);
//          Serial.print("Latitude crua LSB 1: ");
//          Serial.println(mread);
          auxLat = (long int)mread; // Variável auxiliar para dessobrecarregar a RAM
          latit = latit + auxLat;
//          Serial.print("Latitude: ");
//          Serial.println(latit);        
          
          // Leitura do tipo de baliza (tipoBal)
          add_df = add_df+2; // Pula a vírgula e passa para a Latitude
          readFromMainMemory(pg_df, add_df);
//          Serial.print("Tipo baliza crua: ");
//          Serial.println(mread);
          tipoBal = (int)(mread - '0');
//          Serial.print("Tipo de baliza: ");
//          Serial.println(tipoBal);
          
          // Leitura da velocidade (veloc)
          add_df = add_df+2; // Pula a vírgula e passa para velocidade
          readFromMainMemory(pg_df, add_df); // mread = MSB velocidade
//          Serial.print("Velocidade crua MSB: ");
//          Serial.println(mread);
          veloc = mread<<8; // converte mread pra int e armazena em veloc
          add_df++;  // Pula para o segundo byte de veloc (LSB)
          readFromMainMemory(pg_df, add_df); // mread = LSB veloc
//          Serial.print("Velocidade crua LSB: ");
//          Serial.println(mread);
          veloc = veloc + mread;
//          Serial.print("Velocidade: ");
//          Serial.println(veloc);
          
          // Leitura do tipo de direção (tipoDir)
          add_df = add_df+2; // Pula a vírgula e passa para o tipo de direção
          readFromMainMemory(pg_df, add_df);
//          Serial.print("Tipo de direcao crua: ");
//          Serial.println(mread);
          tipoDir = (int)(mread - '0');
//          Serial.print("Tipo direcao: ");
//          Serial.println(tipoDir);
          
          // Leitura do ângulo (ang)
          add_df = add_df+2; // Pula a vírgula e passa para o ângulo
          readFromMainMemory(pg_df, add_df); // mread = MSB ângulo
//          Serial.print("Angulo cru MSB: ");
//          Serial.println(mread);
          ang = mread<<8; // converte mread pra int e armazena no ângulo
          add_df++;  // Pula para o segundo byte de ângulo (LSB)
          readFromMainMemory(pg_df, add_df); // mread = LSB ângulo
//          Serial.print("Angulo cru LSB: ");
//          Serial.println(mread);
          ang = ang + mread;
//          Serial.print("Angulo: ");
//          Serial.println(ang);         
          add_df = add_df + 2; // Pra chegar no % e sair do while
//          Serial.println("*********");
          //i++;
}
  
// ####################### FUNÇÃO PARA LER OS DADOS ENTRE $ E % DA DATAFLASH ###########################
  
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
  
  
// ####################### FUNÇÃO PARA LER OS DADOS SALVOS NA MEMÓRIA ###########################  
//  if(millis() < 1000)
//  {
//      delay(1000);
//      readFromMainMemory(pgnum, add); // Lê o ponto zero
//      if(mread == '$') // Se estiver no início (36 = '$')
//      {
//        while(pgnum < 867)
//        {
//          while (add < 504 )//|| mread != '%')
//          {
//            //  readline();
//            //  parseGPS();
//            readDFPoint();
//            //delay(1000);
//          }
//          add = 0;
//          pgnum++;
//        }
//      }
//      digitalWrite(6,HIGH);
//  }
//  else
//  {
//  }  
// ####################### FUNÇÃO PARA LER OS DADOS SALVOS NA MEMÓRIA ###########################

// ####################### FUNÇÕES ###########################
void parseGPS()
{
  
  // Modelo de String que é parseado no GPS:
   
  // $GPRMC,195209.247,V,1641.0919,S,04900.2393,W,168.5,088.2,260712,,,N*73
  
  // check if $GPRMC (global positioning fixed data)
  
  uint32_t tmp;
  if (strncmp(buffer, "$GPRMC",6) == 0) 
  {
    // hhmmss time data
    parseptr = buffer+7; // chega na parte posterior à 1ª vírgula, para fazer a leitura da hora
    tmp = parsedecimal(parseptr);  // tmp recebe os próximos 9 digítos em formato decimal
    hour = tmp / 10000; // extrai a hora e salva em hour
    minute = (tmp / 100) % 100; // extrai os minutos e salva em minute
    second = tmp % 100; // extrai os segundos e salva em second
    
    parseptr = strchr(parseptr, ',') + 1; // localiza a próxima vírgula e avança uma casa depois dela
    status = parseptr[0]; // status recebe a posição exata depois da vírgula
    parseptr += 2; // o ponteiro avança duas casas (Por que duas casas?)
    
    // grab latitude & long data
    // latitude
    latitude = parsedecimal(parseptr); // Armazena a próxima sequência de dígitos em latitude
    
    // Converte a latitude para o formato de graus, minutos e segundos.
    if (latitude != 0) { // Se latitude for diferente de zero
      latitude *= 10000; // Multiplica latitude por 10000 para fazer ela ficar inteira
      parseptr = strchr(parseptr, '.')+1; // localiza o ponto decimal (vírgula) e avança uma casa
      latitude += parsedecimal(parseptr); // latitude = latitude + o que sobrou de decimal depois do ponto
      latitude2 = latitude + ((latitude - float(int(latitude/1000000))*1000000)/60); // Converte coordenadas para padrão do MAPARADAR - falta tirar o .00 
    }
    
    parseptr = strchr(parseptr, ',') + 1; // Avança 1 casa depois de achar a próxima vírgula
    // read latitude N/S data
    if (parseptr[0] != ',') {
      latdir = parseptr[0]; // latdir recebe S ou N (posição local de parseptr)
    }
    
    //Serial.println(latdir);
    
    // longitude
    parseptr = strchr(parseptr, ',')+1; // Acha a próxima vírgula e avança uma casa
    longitude = parsedecimal(parseptr); // // Armazena a próxima sequência de dígitos em longitude
    if (longitude != 0) {
      longitude *= 10000; // Multiplica longitude por 10000 para fazer ela ficar inteira
      parseptr = strchr(parseptr, '.')+1; // localiza o ponto decimal (vírgula) e avança uma casa
      longitude += parsedecimal(parseptr); // latitude = latitude + o que sobrou de decimal depois do ponto
      longitude2 = longitude + ((longitude - float(int(longitude/1000000))*1000000)/60); // Converte coordenadas para padrão do MAPARADAR - falta tirar o .00
    }
    
    parseptr = strchr(parseptr, ',')+1; // Avança 1 casa depois de achar a próxima vírgula
    // read longitude E/W data
    if (parseptr[0] != ',') {
      longdir = parseptr[0]; // longdir recebe E ou W (posição local de parseptr)
    }
    
    // groundspeed
    parseptr = strchr(parseptr, ',')+1; // Avança 1 casa depois de achar a próxima vírgula
    groundspeed = parsedecimal(parseptr); // groundspeed recebe o valor decimal após a vírgula
    groundspeed = (1.852 * groundspeed); // Velocidade em km/h

    // track angle
    parseptr = strchr(parseptr, ',')+1; // Avança 1 casa depois de achar a próxima vírgula
    trackangle = parsedecimal(parseptr); // track angle recebe o valor decimal após a vírgula


    // date
    parseptr = strchr(parseptr, ',')+1; // Avança 1 casa depois de achar a próxima vírgula
    tmp = parsedecimal(parseptr);  // date recebe o valor decimal após a vírgula
    date = tmp / 10000; // extrai o dia e salva em date
    month = (tmp / 100) % 100; // extrai o mês e salva em month
    year = tmp % 100; // extrai o ano e salva em year
  }
}

void imprim()
{
    Serial.print("\n\rRead: ");
    Serial.print("\n\tTime: "); // Pula uma linha, dá tab e escreve "Time: "
    Serial.print(hour, DEC); Serial.print(':'); // escreve a hora em formato decimal e ":"
    Serial.print(minute, DEC); Serial.print(':'); // escreve os minutos em formato decimal e ":"
    Serial.println(second, DEC); // escreve os segundos em formato decimal e pula a linha
    Serial.print("\tDate: "); // dá um tab e escreve "Date: "
    Serial.print(date, DEC); Serial.print('/'); // escreve o dia em formato decimal e escreve "/"
    Serial.print(month, DEC); Serial.print('/'); // escreve o mês em formato decimal e escreve "/"
    Serial.println(year, DEC); // escreve o ano em formato decimal e pula uma linha
    
    
    // Imprime Latitude
    Serial.print("\tLat: "); // dá um tab e escrevd "Lat: "
    //  if (latdir == 'S')
    //  latitude2 = latitude2*(-1);
    Serial.println(latitude2);
    
    
    //Imprime Longitude
    Serial.print("\tLong: ");  // dá um tab e escreve "Velocidade(km/h): "
    //if (longdir == 'W')
    //longitude2 = longitude2*(-1);
    Serial.println(longitude2);
    
    
    // Imprime velocidade
    Serial.print("\tVelocidade(km/h): ");  // dá um tab e escreve "Velocidade(km/h): "
    Serial.println(groundspeed);
}

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

// Função para ler a linha e armazená-la no buffer[BUFFSIZ]
void readline(void) {
  char c; // cria variável c de 8 bits
  
  buffidx = 0; // start at begninning
  while (1) {
      c=mySerial.read(); // Armazena a linha toda na variável c
      if (c == -1) // Se c == -1, sai do loop.
        continue;
      //Serial.print(c);
      if (c == '\n') // Se houver "new line", sai do loop
        continue;
      if ((buffidx == BUFFSIZ-1) || (c == '\r')) { // se tiver chegado a BUFFSIZ 89 ou carriage return (fim da linha) volta a buffer[buffidx]=0.
        buffer[buffidx] = 0;
        return;
      }
      buffer[buffidx++]= c;
  }
}

// ########################################################## INÍCIO FUNÇÕES PARA PROCESSAMENTO ##############################################################


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

void readFromMainMemory(uint16_t page_addr, uint16_t offset) // Entrega valor do endereço na variável char mread
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
//  mySerial.print("MemoryAdress: ");
//  mySerial.print(offset);
//  mySerial.print(" = ");
//  mySerial.println(mread,DEC);
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
