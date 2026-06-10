CC     = gcc
TARGET = final-d2h
SRC    = final-d2h.c

CFLAGS_DEBUG   = -Wall -gdwarf-5 -std=c99 -m64 -O0
CFLAGS_RELEASE = -Wall -std=c99 -m64 -O2

all: debug

debug: $(SRC)
	$(CC) $(CFLAGS_DEBUG) -o $(TARGET) $(SRC)

release: $(SRC)
	$(CC) $(CFLAGS_RELEASE) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
