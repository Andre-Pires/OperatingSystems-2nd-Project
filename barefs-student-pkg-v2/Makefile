
SUBDIRS = barefs

#
# Dados sobre o grupo e turno frequentado 
# CAMPUS = preencher com A ou T consoante Alameda ou Tagus
# CURSO = indicar o curso do turno frequentado: LEIC ou LERC
# GRUPO = indicar o numero do grupo
# ALUNO1/ALUNO2/ALUNO3 = indicar os numeros dos alunos
#
CAMPUS = T
CURSO = LERC
GRUPO = 14
ALUNO1 = 59008
ALUNO2 = 68593
ALUNO3 = 68624


all: build

build:
	@list='$(SUBDIRS)'; for p in $$list; do \
	  echo "Building $$p"; \
	  $(MAKE) -C $$p; \
	done

clean:
	@list='$(SUBDIRS)'; for p in $$list; do \
	  echo "Cleaning $$p"; \
	  $(MAKE) clean -C $$p; \
	done

package: clean zip

zip:
ifndef CAMPUS
	@echo "ERROR: Must setup macro 'CAMPUS' correcly."
else
ifndef CURSO
	@echo "ERROR: Must setup macro 'CURSO' correcly."
else
ifndef GRUPO
	@echo "ERROR: Must setup macro 'GRUPO' correcly."
else
	tar -czf project-$(CAMPUS)-$(CURSO)-$(GRUPO)-$(ALUNO1)-$(ALUNO2)-$(ALUNO3).tgz * 
endif
endif
endif
