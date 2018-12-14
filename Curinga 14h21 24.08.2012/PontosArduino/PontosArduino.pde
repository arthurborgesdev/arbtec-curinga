/* 
Programa para abrir o arquivo PontosArduino.txt, 
separar as linhas, separar pedaços da linha, 
armazenar pedaços em variáveis, mostrar na tela/terminal,
converter para byte, mostrar na tela, enviar via serial.
Iniciado 24-07-2012
*/

Serial myPort;       

import processing.serial.*;

boolean cmptflag = true;
int i = 0; // número de linhas a serem gravadas
int l = 0;
int p = 0;
int ptotal = 0;
boolean cont = true;

void setup()
{
  //println(Serial.list());
  myPort = new Serial(this, Serial.list()[1], 9600);
  //Código raw para ler arquivo de texto pontosArduino.txt e formatar no arquivo GravArduino.txt.

//  String lines[] = loadStrings("PontosArduino4d - 16h48 - 22.08.12.txt");
//  String arduFinal[] = new String[lines.length];
//  for (int i=0; i < lines.length; i++) 
//  {
//    int index = i+1;
//    arduFinal[i] = ("$"+ index + "," + lines[i]+"%"); 
//    //println("$"+ index + "," + lines[i]+"%");
//    //saveStrings("GravArduino4d - 16h48 - 22.08.12.txt", arduFinal);
//  }
  String ardFile[] = loadStrings("GravArduino4d - 16h48 - 22.08.12.txt");
  print("Tamanho de arduFinal: ");
  println(ardFile.length);
  ptotal = ((ardFile.length)/21); // Faz com que a quantidade de páginas a serem gravadas não precise ser alterada manualmente
  println(ptotal);
}

void draw()
{
  if (cont == true)
  {
    while(p<ptotal)
    {
      GravRam();
      p++; 
      myPort.clear();
      cont = false;
      break;
    }
  }
 if (myPort.read() == 'A')
 {
  cont = true;
 } 
}

void GravRam() // Envia 
{
  for(int i = 0; i < 21; i++)
  {
    String ardFile[] = loadStrings("GravArduino4d - 16h48 - 22.08.12.txt");
    myPort.write('k');
    //myPort.write('\n'); // Corrige os erros que estava dando com o $ virando 35 em vez de 36 ASCII e bagunçando o código
    myPort.write(ardFile[l]);
    myPort.write("\r");
    println(ardFile[l]);
    myPort.clear();     
    //delay(1000);
    l++;
    println(l);
  }
}
