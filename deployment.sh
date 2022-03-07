echo "*Starting deployment process*"
input=""
read -p  "All running Raida's will be stopped and configurations will be replaced, Do you want to continue (y/n)"  input
if  [ $input ==  "n" ]
then
	echo "**Exiting deployment script**"
	exit
fi
echo "*Raida Code compilation started"
gcc -o raida_server raida_server.c udp_socket.c sky_wallet.c -lm -pthread 2>&1 | tee dev_log.txt 
if grep -q error log.txt
then
	echo "**Raida Code compilation failed**"
else
	pkill raida
	echo "***Raida Code compiled successfully***"
	source_path=""
	dest_path_bin=""	
	dest_path=""
	declare -i loop_cnt=0
	while [ $loop_cnt -lt 25 ]
	do 
		raida_no="raida"
		raida_no="$raida_no$loop_cnt"
		msg="**Creating configration for Raida Server"
		msg="$msg $raida_no"
		echo $msg
		source_path="deployment_data/" 
		source_path="$source_path$raida_no" 
		dest_path="/opt/"
		dest_path="$dest_path$raida_no"
		sudo rm -rf $dest_path
		sudo mkdir $dest_path
		sudo \cp raida_server $dest_path
		old_name=$dest_path
		old_name="$old_name/raida_server"
		new_name=$dest_path
		new_name="$new_name/"
		new_name="$new_name$raida_no"
		sudo mv $old_name $new_name
		dest_path_bin=$dest_path
		dest_path="$dest_path/Data"
		source_path="$source_path/Data"
		sudo \cp -rf $source_path $dest_path
		msg="***Creating configration for Raida Server "
		msg="$msg $raida_no"
		msg="$msg completed"
		echo $msg
		msg="**Starting Raida Server "
		msg="$msg $raida_no"
		echo $msg
		dest_path_bin="$dest_path_bin/"		
		dest_path_bin="$dest_path_bin$raida_no"		
		$dest_path_bin &
		loop_cnt=`expr $loop_cnt + 1`
	done
fi