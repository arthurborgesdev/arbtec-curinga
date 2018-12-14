# arbtec-curinga

## Pra que serve?

É um dispositivo eletrônico que identifica a posição dos radares nas ruas e estradas e avisa ao motorista.


## Como funciona?

Coordenadas geográficas dos radares são gravadas de antemão em todos os aparelhos (utilizando um programa feito em linguagem Processing), em uma memória flash acoplada a ele. Durante a operação, o dispositivo compara as coordenadas e a velocidade recebidas por GPS com as gravadas na memória. Se houver um "match", o aparelho emite um sinal sonoro e luminoso para que o motorista reduza a velocidade.


## Arquivos principais e suas funções

**/Curinga 14h21 24.08.2012/Curing_gps_ladyada_v_03alpha/Curing_gps_ladyada_v_03alpha.ino**

Contém a declaração das variáveis, inicialização do buzzer, led, GPS e memória flash, e funções para varredura da memória até encontrar a coordenada e velocidade que batem com a que o GPS está recebendo. Ao encontrar, aciona o LED e o buzzer, e volta para o processo de varredura.

**/Curinga 14h21 24.08.2012/PontosArduino/**

Contém as coordenadas de GPS e suas velocidades permitidas, a serem gravadas na memória flash acoplada à placa principal.

**/Curinga 14h21 24.08.2012/dataflash_at45db161d_v2/dataflash_at45db161d_v2.ino**

Contém o código para utilizar a memória Dataflash da ATMEL, que está acoplada à placa principal.

**/Curinga 14h21 24.08.2012/PontosArduino/PontosArduino.pde**

Programa para abrir o arquivo PontosArduino.txt, separar as linhas, separar pedaços da linha, armazenar pedaços em variáveis, mostrar na tela/terminal, converter para byte, mostrar na tela, enviar via serial à placa para serem gravados na memória Dataflash, acoplada à placa principal.
