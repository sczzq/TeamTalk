#/bin/sh
#start all server

# start sequence
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
	./start.sh $server
done
