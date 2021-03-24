DIR_SRC = ./src
DIR_TMP = ./tmp
# Local project compiler soft link to /usr/local/Cellar/gcc/10.2.0_3/bin/g++-10
CC = ./g++

SRC = $(wildcard ${DIR_SRC}/*.cc)
TMP = $(wildcard $(DIR_TMP)/*.o)

all:
	${CC} -std=c++20 -c $(foreach i, $(SRC), $(i))

install:
	$(shell mkdir ${DIR_TMP})
	$(shell mv *.o ${DIR_TMP})

	${CC} $(foreach i, $(TMP), $(i)) -o drift

	@echo "<TARGET FILE GENERATED>: ./drift 🐇 🐰 🍻 "

run: 
	./test/run.sh

clean:
	rm -f *.o
	rm -rf ${DIR_TMP}
	rm -f drift