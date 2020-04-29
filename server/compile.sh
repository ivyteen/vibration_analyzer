gcc -o server server_main.c db_inf.c data_handler.c -ljson-c -lm -lmysqlclient -lz -lhiredis
