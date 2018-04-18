#/bin/sh
#start or stop the im-server


function restart() {
	cd $1
	if [ ! -e *.conf  ]
	then
		echo "no config file"
		return
	fi

#	pid=`cat server.pid`
#	do not use server.pid to detect process status.
	pid=`pidof $1`
	if [ $pid'x' != 'x' ]; then
		echo "kill pid=$pid"
		kill $pid
		while true
		do
			oldpid=`ps -ef|grep $1|grep -o $pid`;
			if [ $oldpid"x" == $pid"x" ]; then
				echo $oldpid" "$pid
				sleep 1
			else
				break
			fi
		done
		../daeml ./$1
	else
		../daeml ./$1
	fi
}

case $1 in
	login_server)
		restart $1
		;;
	msg_server)
		restart $1
		;;
	route_server)
		restart $1
		;;
	http_msg_server)
		restart $1
		;;
	file_server)
		restart $1
		;;
	push_server)
		restart $1
		;;
	db_proxy_server)
		restart $1
		;;
	msfs)
		restart $1
		;;
	*)
		echo "Usage: "
		echo "  ./restart.sh (login_server|msg_server|route_server|http_msg_server|file_server|push_server|db_proxy_server|msfs)"
		;;
esac
