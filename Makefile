output: vmmgr.o
	gcc vmmgr.o -o vmmgr

fork_bomb.o: vmmgr.c
	gcc -c -g vmmgr.c

clean:
	rm *.o vmmgr
