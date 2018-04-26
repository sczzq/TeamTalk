原官方说明请见 -- -- 行

--------------------------------------------------------
这里是此处维护的说明 
TeamTalk项目是消息通信的一个解决方案，以下简称项目， 
项目中包含服务器端、客户端，客户端包含 Windows, Android, iOS, MAC平台子项目。 

以下主要介绍服务器端。 

服务器端由几个独立运行的程序构成， 
每个程序监听一个或多个地址，地址由IP/PORT组成 
有几个程序监听客户端的数据请求，然后向其他程序交互，进行消息处理和数据处理， 


使用方式：

运行环境：Linux各发行版 
已验证环境：CentOS-7-1608

依赖：
	基本编译环境
	mariadb
	redis
	log4cxx
	nginx
	php
	protobuf

建议将上述依赖安装完成再进行下一步

此工程, 服务器端的使用分为以下几步：
第一步，编译：
	1，进入server/src目录，
	2，在终端下输入./build.sh version 1，回车以执行编译
第二步，部署：
	0, 将 server 目录下的 im-server* 压缩包文件拷贝到 auto_setup/im_server 目录下，
	   将 pb 目录改名为 tt，压缩到 tt.zip，拷贝到 auto_setup/im_web 目录下，
	1，进入 auto_setup 目录，
	2，在终端输入./setup.sh check，回车以检查环境
	3，在终端输入./setup.sh install，回车以安装、运行

配置说明：
	1，IP/PORT配置，见deploy_for_ip.md
	2，


--------------------------------------------------------

# TeamTalk
	TeamTalk is a solution for enterprise IM
	
	具体文档见doc目录下,安装之前请仔细阅读相关文档。
	
# 交流
		建议大家在开发过程中遇到问题,提交issues到https://github.com/mogujie/TeamTalk/issues  
		
		我们的官方维护人员会抽时间解答,谢谢大家的理解.
		* qq群:341273218
