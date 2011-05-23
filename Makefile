all:	off.exe flash.exe wheel.exe sequence.exe dim.exe

off.exe: off.c CONF/common.h CONF/config.h CONF/common.o CONF/main.o CONF/out_file.o
	cc -o off.exe off.c CONF/main.o CONF/common.o CONF/out_file.o

flash.exe: flash.c CONF/common.h CONF/config.h CONF/common.o CONF/main.o  CONF/out_file.o
	cc -o flash.exe flash.c CONF/main.o CONF/common.o CONF/out_file.o

wheel.exe: wheel.c CONF/common.h CONF/config.h CONF/common.o CONF/main.o  CONF/out_file.o
	cc -o wheel.exe wheel.c CONF/main.o CONF/common.o CONF/out_file.o

sequence.exe: sequence.c CONF/common.h CONF/config.h CONF/common.o CONF/main.o  CONF/out_file.o
	cc -o sequence.exe sequence.c CONF/main.o CONF/common.o CONF/out_file.o

dim.exe: dim.c CONF/common.h CONF/config.h CONF/common.o CONF/main.o  CONF/out_file.o
	cc -o dim.exe dim.c CONF/main.o CONF/common.o CONF/out_file.o

clean:
	rm -f *.o
