CC=clang
CFLAGS=-I. -pipe -march=native -O3 -Wall
LMYSQL=-L/usr/lib/mysql -lmysqlclient
LMD5=-lcrypto

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
all: clean account_add char_export convert_quest convert_unitxt login_server make_key newtable patch_server ship_server
clean:
	rm -f src/*/*.o
account_add:
	$(CC) -o bin/account_add src/account_add/account_add.c $(LMYSQL) $(LMD5) $(CFLAGS)
char_export:
	$(CC) -o bin/char_export src/char_export/char_export.c $(LMYSQL) $(CFLAGS)
convert_quest:
	$(CC) -o bin/convert_quest src/convert_quest/convert_quest.c $(CFLAGS)
convert_unitxt:
	$(CC) -o bin/convert_unitxt src/convert_unitxt/convert_unitxt.c $(CFLAGS)
login_server:
	$(CC) -o bin/login_server src/login_server/login_server.c $(LMYSQL) $(LMD5) $(CFLAGS)
make_key:
	$(CC) -o bin/make_key src/make_key/make_key.c $(LMYSQL) $(CFLAGS)
newtable:
	$(CC) -o bin/newtable src/newtable/newtable.c $(CFLAGS)
patch_server:
	$(CC) -o bin/patch_server src/patch_server/patch_server.c $(CFLAGS)
ship_server:
	$(CC) -o bin/ship_server src/ship_server/ship_server.c $(CFLAGS)
