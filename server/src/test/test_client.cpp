/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：test_client.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月30日
 *   描    述：
 *
 ================================================================*/

#include <vector>
#include <iostream>
#include "ClientConn.h"
#include "netlib.h"
#include "TokenValidator.h"
#include "Thread.h"
#include "IM.BaseDefine.pb.h"
#include "IM.Buddy.pb.h"
#include "playsound.h"
#include "Common.h"
#include "Client.h"
#include "security.h"
using namespace std;

#define MAX_LINE_LEN	1024
string g_login_domain = "192.168.0.106:8080";
string g_cmd_string[10];
int g_cmd_num;
CClient* g_pClient = NULL;
string myname {"null"}; 
string hername {"null"};
uint32_t herid {0};
string last_command;

void split_cmd(char* buf)
{
	int len = strlen(buf);
	string element;

	g_cmd_num = 0;
	for (int i = 0; i < len; i++) {
		if (buf[i] == ' ' || buf[i] == '\t') {
			if (!element.empty()) {
				g_cmd_string[g_cmd_num++] = element;
				element.clear();
			}
		} else {
			element += buf[i];
		}
	}

	// put the last one
	if (!element.empty()) {
		g_cmd_string[g_cmd_num++] = element;
	}
}

void print_help()
{
	printf("Usage:\n");
	printf("login user_name user_pass\n");
	printf("getalluser\n");
	printf("getdepart\n");
	printf("getuserdepart\n");
	printf("getmsg   to get all unread msg\n");
	printf("send <id> msg\n");
	printf("send     enter last user contact session");
	printf("listuserid\n");
	printf("close\n");
	printf("quit\n");
}

void doLogin(const string& strName, const string& strPass)
{
	try
	{
		g_pClient = new CClient(strName, strPass, g_login_domain);
	}
	catch(...)
	{
		printf("get error while alloc memory\n");
		PROMPTION(myname.c_str());
		return;
	}
	try{
		g_pClient->connect();
	} catch (...) {
		printf("connect error\n");
		return;
	}
	return;
}
	
void exec_cmd()
{
	if (g_cmd_num == 0) {
		return;
	}

	if(last_command == "send") {
		if(g_cmd_num == 1 && g_cmd_string[0] == "quit"){
			last_command = "";
		} else if(hername != "" and herid != 0) {
			char *enc_msg = NULL;
			uint32_t enc_msg_len = 0;
			string message;
			for(int i=0; i<g_cmd_num; i++){
				message += g_cmd_string[i];
				message += " ";
			}
			EncryptMsg(message.c_str(), message.length(), &enc_msg, enc_msg_len);
			string emsg {enc_msg, enc_msg_len};
			IM::BaseDefine::MsgType mtype = IM::BaseDefine::MSG_TYPE_SINGLE_TEXT;
			g_pClient->sendMsg(herid, mtype, emsg);
		} else {
			printf("waiting reply...\n");
		}
	}
	else if(g_cmd_string[0] == "login")
	{
		if(g_cmd_num == 3)
		{
			char *enc_pass = NULL;
			unsigned int enc_pass_len = 0;
			EncryptPass(g_cmd_string[2].c_str(), g_cmd_string[2].length(), &enc_pass, enc_pass_len);
			string password_enc {enc_pass, enc_pass_len};
			free(enc_pass);
			myname = g_cmd_string[1];
			doLogin(g_cmd_string[1], password_enc);
		}
		else
		{
			print_help();
		}
	}
	else if (strcmp(g_cmd_string[0].c_str(), "close") == 0) {
		g_pClient->close();
	}
	else if (strcmp(g_cmd_string[0].c_str(), "quit") == 0) {
		exit(0);
	}
	else if(strcmp(g_cmd_string[0].c_str(), "list") == 0)
	{
		printf("+-------+---------------------+\n");
		printf("| id    |        用户名       |\n");
		printf("+-------+---------------------+\n");
		CMapId2User_t mapUser = g_pClient->getMapId2UserMap();
		for(auto it = mapUser.begin(); it!=mapUser.end(); ++it)
		{
			printf("|%-7u|%21s|\n", it->first, it->second->user_nick_name().c_str());
			printf("+-------+---------------------+\n");
		}
		printf("| id    |        部门名       |\n");
		printf("+-------+---------------------+\n");
		CMapId2Depart_t mapdepart = g_pClient->getMapId2DepartMap();
		for(auto it=mapdepart.begin(); it!=mapdepart.end(); ++it){
			printf("|%-7u|%21s|\n", it->first, it->second->dept_name().c_str());
			printf("+-------+---------------------+\n");
		}
	}
	else if(g_cmd_string[0] == "getalluser") {
		uint32_t ret = g_pClient->getChangedUser();
		log("result %u", ret);
	}
	else if(g_cmd_string[0] == "getdepart") {
		uint32_t ret = g_pClient->getChangedDepart();
		log("getdepart seqNo %u", ret);
	}
	else if(g_cmd_string[0] == "getuserdepart") {
		uint32_t ret = g_pClient->getChangedUser();
		log("getuser seqNo %u", ret);
		ret = g_pClient->getChangedDepart();
		log("getdepart seqNo %u", ret);
	}
	else if(g_cmd_string[0] == "getsession") {
		uint32_t ret = g_pClient->getRecentSession();
		log("get session seqNo %u", ret);
	}
	else if(g_cmd_string[0] == "listsession") {
		printf("---------------------------------------\n");
		ContactSession&	session = g_pClient->get_contact_session();
		for(auto it = session.begin(); it != session.end(); ++it){
			printf("session id: %u, latest_msg_id: %u, latest_msg_from_user_id: %u\n", it->first, it->second->latest_msg_id(), it->second->latest_msg_from_user_id());
			printf("---------------------------------------\n");
		}
	}
	else if(g_cmd_string[0] == "getmsg") {
		g_pClient->getUnreadMsgCnt();
	}
	else if(g_cmd_string[0] == "showmsg") {
		Msg & msg = g_pClient->get_msg();
		printf("---------------------------------------\n");
		for(auto it = msg.begin(); it != msg.end(); ++it){
			printf("msg_id: %u, from_session_id: %u, msg_data: %s\n", it->first, it->second->from_session_id(), it->second->msg_data().c_str());
			printf("---------------------------------------\n");
		}
	}
	else if(g_cmd_string[0] == "send") {
		if(g_cmd_num >= 3) {
			uint32_t toID = atoi(g_cmd_string[1].c_str());
			char *enc_msg = NULL;
			uint32_t enc_msg_len = 0;
			string message;
			for(int i=2; i<g_cmd_num; i++){
				message += g_cmd_string[i];
			}
			EncryptMsg(message.c_str(), message.length(), &enc_msg, enc_msg_len);
			string emsg {enc_msg, enc_msg_len};
			IM::BaseDefine::MsgType mtype = IM::BaseDefine::MSG_TYPE_SINGLE_TEXT;
			g_pClient->sendMsg(toID, mtype, emsg);
			last_command = "send";
		} else {
			print_help();
		}
	}
	else {
		print_help();
	}
}

class CmdThread : public CThread
{
	public:
		void OnThreadRun()
		{
			while (true)
			{
				fprintf(stderr, "%s>", myname.c_str());	// print to error will not buffer the printed message

				if (fgets(m_buf, MAX_LINE_LEN - 1, stdin) == NULL)
				{
					fprintf(stderr, "fgets failed: %d\n", errno);
					continue;
				}

				m_buf[strlen(m_buf) - 1] = '\0';	// remove newline character

				split_cmd(m_buf);

				exec_cmd();
			}
		}
	private:
		char	m_buf[MAX_LINE_LEN];
};

CmdThread g_cmd_thread;

int main(int argc, char* argv[])
{
	//    play("message.wav");
	g_cmd_thread.StartThread();

	signal(SIGPIPE, SIG_IGN);

	int ret = netlib_init();

	if (ret == NETLIB_ERROR)
		return ret;

	netlib_eventloop();

	return 0;
}
