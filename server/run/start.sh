#/bin/sh
#start server


function start() {
	cd $1
	if [ ! -e *.conf  ]
	then
		echo "no config file"
		return
	fi

	pid=`pidof $1`
	if [ $pid'x' == 'x' ]; then
		echo ../daeml ./$1 to start 
		../daeml ./$1
	else
		echo $1 is running, please stop it first.
	fi
}

case $1 in
	login_server)
		start $1
		;;
	msg_server)
		start $1
		;;
	route_server)
		start $1
		;;
	http_msg_server)
		start $1
		;;
	file_server)
		start $1
		;;
	push_server)
		start $1
		;;
	db_proxy_server)
		start $1
		;;
	msfs)
		start $1
		;;
	*)
		echo "Usage: "
		echo "  ./start.sh (login_server|msg_server|route_server|http_msg_server|file_server|push_server|db_proxy_server|msfs)"
		;;
esac
