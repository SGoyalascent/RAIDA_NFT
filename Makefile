all:
	gcc -DVERSION=\"`date +'%s'`\" -o raida_server raida_server.c udp_socket.c sky_wallet.c library.c aes.c -lm -pthread


deploy:
	scp -P 88 raida_server root@superraida:/home/sean/raida_deployment/raida
	ssh -p 88 root@superraida "/home/sean/deploy/go.sh"
