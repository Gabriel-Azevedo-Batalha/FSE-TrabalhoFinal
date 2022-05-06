# FSE-TrabalhoFinal

Repositório para o trabalho final de Fundamentos de Sistemas Embarcados. Enunciado do trabalho: https://gitlab.com/fse_fga/trabalhos-2021_2/trabalho-final-2021-2.

## Compilação e execução
### Central
Na pasta central, utilize *make* para compilar e para executar *make run*.

### ESPs
Na pasta ESP, após ter arrumado as variáveis de ambiente(export.sh) faça o seguinte:
  1) Utilize o comando ``` idf.py menuconfig ``` para configurar o Wifi e o modo da ESP
  2) Utilize o comando ``` idf.py build ``` para dar build no projeto
  3) Utilize o comando ``` idf.py -p <PORTA> flash monitor``` para executar
  4) Utilize o comando ``` idf.py -p <PORTA> erase-flash``` para resetar a memória se necessário

## Uso
### Central
Ao ser executado o servidor central aguarda por uma solicitação de cadastro, após isso todas os comandos são exibidos na tela.

Foi observado um bug ocasional, em que o servidor central não se inscreve corretamente(mesmo quando retorna sucesso) no tópico do cômodo da ESP e não recebe as mensagens. O bug acontece raramente e pode ser contornado descadastrando e recadastrando a esp ou reiniciando o servidor central.

### ESPs
Após a inicialização, manda uma mensagem solicitando cadastro para o servidor central. Ao encerrar o cadastro, o botão embutido representa uma modificação na entrada.

## Log
O servidor central mantém um log de certas ações que ocorrem durante a sua execução:
  - Mudanças nas saídas
  - Armes e desarmes do alarme
  - Ativações no alarme
  - Cadastros
  - Descadastros
  - Mudanças nas entradas que não ativam o alarme
