bme280:main.o bme280.o
	gcc -Wall -o bme280 main.o bme280.o -std=gnu99
main.o: main.c bme280_lib/bme280.h bme280_lib/bme280_defs.h
	gcc -Wall -c main.c -std=gnu99
bme280.o: bme280_lib/bme280.c bme280_lib/bme280.h bme280_lib/bme280_defs.h
	gcc -Wall -c bme280_lib/bme280.c -std=gnu99
clean:
	rm main.o bme280.o bme280