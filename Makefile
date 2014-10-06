SOURCES = \
	  lib/polarssl/base64.c \
	  lib/polarssl/aes.c \
	  lib/polarssl/sha1.c \
	  secrecy.c \
	  log.c \
	  main.c

OUTPUT = secrecy

$(OUTPUT) : $(SOURCES)
	gcc -o $(OUTPUT) $(SOURCES)
	chmod +x $(OUTPUT)

.PHONY: clean install uninstall
clean:
	-rm -f *.o *.out $(OUTPUT)

install:
	-sudo cp $(OUTPUT) /usr/bin/$(OUTPUT)

uninstall:
	-sudo rm -f /usr/bin/$(OUTPUT)
