.PHONY : clean, install, uninstall
P=ted
CFLAGS= -Wall -O0
OBJECTS=
LIBS=
$(P) : $(P).c
	$(CC) $(CFLAGS) $< -o $@
clean :
	rm -f $(P) *~ .*~ .*.swp
install : $(P).1.gz
	mv $(P) /usr/local/bin/
	mv ted.1.gz /usr/share/man/man1/

ted.1.gz : $(P).1
	gzip -k ted.1

uninstall :
	rm /usr/local/bin/$(P)
	rm /usr/share/man/man1/ted.1.gz
