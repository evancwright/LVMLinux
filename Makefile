#makefile

lvm main.c:
	echo "linking"
	cc -c -lm main.c
	mv a.out lvm
	
clean:
	echo "cleaning up old files"
	rm -f lvm
	rm -f main.o
