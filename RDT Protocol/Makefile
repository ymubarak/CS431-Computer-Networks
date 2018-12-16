CC              = gcc
LD              = gcc
AR              = ar

CFLAGS          = -Wall #-ansi
LFLAGS          = -Wall #-ansi

SERVERJOBS	= tcp_server/server.o lib/tcp.o lib/fileHandler.o
SERVERJOBS_O = server.o tcp.o fileHandler.o
CLIENTJOBS	= tcp_client/client.o lib/tcp.o lib/fileHandler.o
CLIENTJOBS_O = client.o tcp.o fileHandler.o
ALLEXEC	= server client

.c.o:
	$(CC) $(CFLAGS) -c $<

all: $(ALLEXEC)

server: $(SERVERJOBS)
	$(LD) $(LFLAGS) -o $@ $(SERVERJOBS_O)

client: $(CLIENTJOBS)
	$(LD) $(LFLAGS) -o $@ $(CLIENTJOBS_O)

clean:
	rm -f *.o $(ALLEXEC)

realclean: clean
	rm -rf proj1.tar.gz

tarball: realclean
	tar cf - `ls -a | grep -v '^\.*$$' | grep -v '^proj[0-9].*\.tar\.gz'` | gzip > proj1-$(USER).tar.gz
