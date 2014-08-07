			INSTRU��ES
			===========

pr� gdb:
->>>>>>>>>>>>>>>>>>>>>>> gdb -args ./barefs -s -d my_dir <------------------------------

Ap�s a descompata��o do pacote base, instala o "barefs", executando os seguintes passos:
===============================================================================================

1) Na directoria principal, executa o comando "make" para a compila��o e cria��o da aplica��o "barefs";

2) Muda para a directoria "barefs" e cria uma <subdirectoria> onde o "barefs" ser� montado 
  (podes utilizar a directoria "my_dir" ai' ja' existente);
3) Escreve na linha de comandos:
            ./barefs  <subdirectoria>
   alternativamente para a obten��o de informa��o de debbuging:
             /barefs -d <subdirectoria>  

  caso se pretenda o funcionamento uni-tarefa, incluir a op��o -s . Por exemplo:
            ./barefs  -s -d <subdirectoria>
4) Executar comandos que envolvam a <subdirectoria> , como por exemplo: 
            ls -alt > <subdirectoria>/dir.txt
            ls -alt <subdirectoria>
            etc...
   programas que utilizem as fun��es de sistema call, write e read...

5) Para desmontar o "barefs" do Fuse, executar:
            fusermount -u <subdirectoria>

6) para verificar se a desmontagem teve sucesso confirmar que o "barefs" n�o se encontra na lista que 
   resulta da execu��o do comando:
           mount
__________________________________________________________________________________________________
ATEN��O: Se pretendes utilizar o debugger gdb (ou o ddd) para a depura��o de erros, dever�s
         incluir a op��o -s.  


