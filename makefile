#To build the lantern interpreter type make<enter>
lvm : main.o
	echo linking
	cc main.o -o lvm
	chmod +x ./lvm
	echo Done. 
	echo Please report any issues to evancwright@yahoo.com

main.o: 
	echo Building lvm...
	cc main.c -c -o main.o 

