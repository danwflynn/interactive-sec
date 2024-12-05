mqtt:
	gcc -o rdr rd_receive_test.c -I../paho.mqtt.c/src -L../paho.mqtt.c/build/src -Wl,-rpath,../paho.mqtt.c/build/src -lpaho-mqtt3c

http:
	gcc -o http_server http_server.c -lmicrohttpd
