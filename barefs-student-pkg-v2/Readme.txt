			INSTRUÇÕES
			===========

pró gdb:
->>>>>>>>>>>>>>>>>>>>>>> gdb -args ./barefs -s -d my_dir <------------------------------

Após a descompatação do pacote base, instala o "barefs", executando os seguintes passos:
===============================================================================================

1) Na directoria principal, executa o comando "make" para a compilação e criação da aplicação "barefs";

2) Muda para a directoria "barefs" e cria uma <subdirectoria> onde o "barefs" será montado 
  (podes utilizar a directoria "my_dir" ai' ja' existente);
3) Escreve na linha de comandos:
            ./barefs  <subdirectoria>
   alternativamente para a obtenção de informação de debbuging:
             /barefs -d <subdirectoria>  

  caso se pretenda o funcionamento uni-tarefa, incluir a opção -s . Por exemplo:
            ./barefs  -s -d <subdirectoria>
4) Executar comandos que envolvam a <subdirectoria> , como por exemplo: 
            ls -alt > <subdirectoria>/dir.txt
            ls -alt <subdirectoria>
            etc...
   programas que utilizem as funções de sistema call, write e read...

5) Para desmontar o "barefs" do Fuse, executar:
            fusermount -u <subdirectoria>

6) para verificar se a desmontagem teve sucesso confirmar que o "barefs" não se encontra na lista que 
   resulta da execução do comando:
           mount
__________________________________________________________________________________________________
ATENÇÃO: Se pretendes utilizar o debugger gdb (ou o ddd) para a depuração de erros, deverás
         incluir a opção -s.  


