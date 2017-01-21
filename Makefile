# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

CFLAGS = -Wall -g
DEPS =

%.o: %.c $(DEPS)
	gcc -c -o $@ $< $(CFLAGS)

pascaltri: pascaltri.o
	gcc -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f pascaltri *.o *~ core

