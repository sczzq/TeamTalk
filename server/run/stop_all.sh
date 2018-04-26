#/bin/sh
#stop the im-server

# sequence
SERVER='route_server'
SERVER+=' file_server'
SERVER+=' push_server'
SERVER+=' db_proxy_server'
SERVER+=' msfs'
SERVER+=' http_msg_server'
SERVER+=' login_server'
SERVER+=' msg_server'

for server in $SERVER
do
	pid=`pidof $server`
	if [ $pid'x' != 'x' ]; then
		echo $server $pid
		
		kill $pid
		while true
		do
			oldpid=`pidof $server`
			if [ $oldpid'x' == $pid'x' ]; then
				echo waiting...
				sleep 1
			else
				break
			fi
		done
	else 
		echo $server is stopped.
	fi
done
