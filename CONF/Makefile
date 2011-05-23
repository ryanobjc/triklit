all:	out_file.o common.o main.o genheader.exe

main.o:	main.c config.h
	cc -c -o main.o main.c

common.o: common.c common.h config.h
	cc -c -o common.o common.c

out_file.o: out_file.c common.h config.h
	cc -c -o out_file.o out_file.c

genheader.exe:	genheader.c
	cc -o genheader.exe genheader.c

clean:
	rm -f *.o
