/*================================================================
 *     Copyright (c) 2015年 lanhu. All rights reserved.
 *   
 *   文件名称：Client.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2015年01月20日
 *   描    述：
 *
 ================================================================*/
#include <algorithm>
#include "Client.h"
#include "HttpClient.h"
#include "Common.h"
#include "json/json.h"
#include "ClientConn.h"


static ClientConn*  g_pConn = NULL;

static bool         g_bLogined = false;

CClient::CClient(const string& strName, const string& strPass, const string strDomain):
	m_strName(strName),
	m_strPass(strPass),
	m_strLoginDomain(strDomain),
	m_nLastGetUser(0)
{
}

CClient::~CClient()
{
}

void CClient::TimerCallback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	if (g_bLogined) {
		uint64_t cur_time = get_tick_count();
		g_pConn->OnTimer(cur_time);
	}
}



void CClient::onError(uint32_t nSeqNo, uint32_t nCmd,const string& strMsg)
{
	g_imlog.Error("get error:%d, msg:%s", nCmd, strMsg.c_str());
}

void CClient::connect()
{
	CHttpClient httpClient;
	string strUrl = m_strLoginDomain + "/msg_server";
	string strResp;
	CURLcode nRet = httpClient.Get(strUrl, strResp);
	if(nRet != CURLE_OK)
	{
		printf("login falied. access url:%s error\n", strUrl.c_str());
		PROMPTION;
		return;
	}
	Json::Reader reader;
	Json::Value value;
	if(!reader.parse(strResp, value))
	{
		printf("login falied. parse response error:%s\n", strResp.c_str());
		PROMPTION;
		return;
	}
	string strPriorIp, strBackupIp;
	uint16_t nPort;
	try {
		uint32_t nRet = value["code"].asUInt();
		if(nRet != 0)
		{
			string strMsg = value["msg"].asString();
			printf("login falied. errorMsg:%s\n", strMsg.c_str());
			PROMPTION;
			return;
		}
		strPriorIp = value["priorIP"].asString();
		strBackupIp = value["backupIP"].asString();
		nPort = atoi(value["port"].asString().c_str());

	} catch (std::runtime_error msg) {
		printf("login falied. get json error:%s\n", strResp.c_str());
		PROMPTION;
		return;
	}

	g_pConn = new ClientConn(this);
	m_nHandle = g_pConn->connect(strPriorIp.c_str(), nPort, m_strName, m_strPass);
	if(m_nHandle != INVALID_SOCKET)
	{
		log("connect success");
		netlib_register_timer(CClient::TimerCallback, NULL, 1000);
		uint32_t ret = login();
		log("login seqNo %u", ret);
/*
		getChangedUser();
		getChangedDepart();
		getRecentSession();
		getUnreadMsgCnt();
*/
	}
	else
	{
		printf("invalid socket handle\n");
	}
}

void CClient::onConnect()
{
	login();
}


void CClient::close()
{
	g_pConn->Close();
}

void CClient::onClose()
{

}

uint32_t CClient::login()
{
	log("login");
	return g_pConn->login(m_strName, m_strPass);
}

void CClient::onLogin(uint32_t nSeqNo, uint32_t nResultCode, string& strMsg, IM::BaseDefine::UserInfo* pUser)
{
	if(nResultCode != 0)
	{
		printf("login failed.errorCode=%u, msg=%s\n",nResultCode, strMsg.c_str());
		return;
	}
	if(pUser)
	{
		m_cSelfInfo = *pUser;
		g_bLogined = true;
		printf("login success\n");
		printf("try to get info\n");
		getChangedUser();
		getChangedDepart();
		getRecentSession();
		getUnreadMsgCnt();
	}
	else
	{
		printf("pUser is null\n");
	}
}

uint32_t CClient::getAllMsg()
{
	for(auto it = m_unread_info.begin(); it != m_unread_info.end(); ++it){
		auto session_type = it->second->session_type(); 
		auto peerid = it->second->latest_msg_from_user_id();
		auto msgid = it->second->latest_msg_id();
		auto msgcnt = it->second->unread_cnt();
		getMsgList(session_type, peerid, msgid, msgcnt);
	}
	return 0;
}

uint32_t CClient::getChangedUser()
{
	uint32_t nUserId = m_cSelfInfo.user_id();
	log("try to get all changed user, this user_id %d.", nUserId);
	return g_pConn->getAllUser(nUserId, m_nLastGetUser);
}

void CClient::onGetChangedUser(uint32_t nSeqNo, const list<IM::BaseDefine::UserInfo> &lsUser)
{
	log("get changed user success.");
	for(auto it=lsUser.begin(); it!=lsUser.end(); ++it)
	{
		IM::BaseDefine::UserInfo* pUserInfo = new IM::BaseDefine::UserInfo();
		*pUserInfo = *it;
		IM::BaseDefine::UserInfo cUser = *pUserInfo;
		log("user_id: %u, user_gender: %u, user_nick_name: %s, avatar_url: %s, department_id: %u, email: %s,"
			"user_real_name: %s, user_tel: %s, user_domain: %s, status: %u, sign_info: %s",
			cUser.user_id(), cUser.user_gender(), cUser.user_nick_name().c_str(),
			cUser.avatar_url().c_str(), cUser.department_id(), cUser.email().c_str(),
			cUser.user_real_name().c_str(), cUser.user_tel().c_str(), cUser.user_domain().c_str(),
			cUser.status(), cUser.sign_info().c_str());
		uint32_t nUserId = pUserInfo->user_id();
		string strNick = pUserInfo->user_nick_name();
		if(it->status() != 3)
		{
			auto it1 = m_mapId2UserInfo.find(nUserId);
			if(it1 == m_mapId2UserInfo.end())
			{
				m_mapId2UserInfo.insert(pair<uint32_t, IM::BaseDefine::UserInfo*>(nUserId, pUserInfo));
			}
			else
			{
				delete it1->second;
				m_mapId2UserInfo[nUserId] = pUserInfo;
			}

			auto it2 = m_mapNick2UserInfo.find(strNick);
			if(it2 == m_mapNick2UserInfo.end())
			{
				m_mapNick2UserInfo.insert(pair<string, IM::BaseDefine::UserInfo*>(strNick, pUserInfo));
			}
			else
			{
				delete it1->second;
				m_mapNick2UserInfo[strNick] = pUserInfo;
			}
		}
	}
}

uint32_t CClient::getUserInfo(list<uint32_t>& lsUserId)
{
	uint32_t nUserId = m_cSelfInfo.user_id();
	return g_pConn->getUserInfo(nUserId, lsUserId);
}

void CClient::onGetUserInfo(uint32_t nSeqNo, const list<IM::BaseDefine::UserInfo> &lsUser)
{
	log("get user info success");
}

uint32_t CClient::getChangedDepart()
{
	uint32_t nUserId = m_cSelfInfo.user_id();
	log("try to get all changed department, this user_id %d.", nUserId);
	return g_pConn->getAllDepart(nUserId, m_nLastGetUser);
}

void CClient::onGetChangedDepart(uint32_t nSeqNo, const list<IM::BaseDefine::DepartInfo> &lsDepart)
{
	log("get all changed depart success.");
	for(auto it=lsDepart.begin(); it!=lsDepart.end(); ++it)
	{
		IM::BaseDefine::DepartInfo* pDepartInfo = new IM::BaseDefine::DepartInfo();
		*pDepartInfo = *it;
		IM::BaseDefine::DepartInfo info = *pDepartInfo;
		log("dept_id: %d, priority: %d, dept_name: %s, parent_dept_id: %d, dept_status: %d",
			info.dept_id(), info.priority(), info.dept_name().c_str(), info.dept_status());
		uint32_t nDepartId = pDepartInfo->dept_id();
		string strname = pDepartInfo->dept_name();
		auto it1 = m_mapId2DepartInfo.find(nDepartId);
		if(it1 == m_mapId2DepartInfo.end())
		{
			m_mapId2DepartInfo.insert(pair<uint32_t, IM::BaseDefine::DepartInfo*>(nDepartId, pDepartInfo));
		}
		else
		{
			delete it1->second;
			m_mapId2DepartInfo[nDepartId] = pDepartInfo;
		}

		auto it2 = m_mapNick2DepartInfo.find(strname);
		if(it2 == m_mapNick2DepartInfo.end())
		{
			m_mapNick2DepartInfo.insert(pair<string, IM::BaseDefine::DepartInfo*>(strname, pDepartInfo));
		}
		else
		{
			delete it1->second;
			m_mapNick2DepartInfo[strname] = pDepartInfo;
		}
	}
}

uint32_t CClient::sendMsg(uint32_t nToId, IM::BaseDefine::MsgType nType, const string &strMsg)
{
	uint32_t nFromId = m_cSelfInfo.user_id();
	log("try to send msg");
	return g_pConn->sendMessage(nFromId, nToId, nType, strMsg);
}

void CClient::onSendMsg(uint32_t nSeqNo, uint32_t nSendId, uint32_t nRecvId, IM::BaseDefine::SessionType nType, uint32_t nMsgId)
{
	printf("send msg succes. seqNo:%u, msgId:%u\n", nSeqNo, nMsgId);
}


uint32_t CClient::getUnreadMsgCnt()
{
	uint32_t nUserId = m_cSelfInfo.user_id();
	log("try to get unread msg count");
	return g_pConn->getUnreadMsgCnt(nUserId);
}

void CClient::onGetUnreadMsgCnt(uint32_t nSeqNo, uint32_t nUserId, uint32_t nTotalCnt, const list<IM::BaseDefine::UnreadInfo>& lsUnreadCnt)
{
	log("get unread msg count success,")
	log("seqNo: %u, userId: %u, totalcnt: %u", nSeqNo, nUserId, nTotalCnt);
	for(auto it = lsUnreadCnt.begin(); it != lsUnreadCnt.end(); ++it){
		uint32_t id = it->session_id();
		IM::BaseDefine::UnreadInfo* info = new IM::BaseDefine::UnreadInfo(*it);
		auto it2 = m_unread_info.find(id);
		if(it2 == m_unread_info.end()){
			m_unread_info.insert(pair<uint32_t, IM::BaseDefine::UnreadInfo*>(id, info));
		} else {
			delete it2->second;
			m_unread_info[id] = info;
		}
	}
	log("then try to get this msg");
	getAllMsg();
}

uint32_t CClient::getRecentSession()
{
	uint32_t nUserId = m_cSelfInfo.user_id();
	return g_pConn->getRecentSession(nUserId, m_nLastGetSession);
}

void CClient::onGetRecentSession(uint32_t nSeqNo, uint32_t nUserId, const list<IM::BaseDefine::ContactSessionInfo> &lsSession)
{
	log("seqNo: %u, userId: %u", nSeqNo, nUserId);
	// someone need to confirm the unique of session_id.
	for(auto it = lsSession.begin(); it != lsSession.end(); ++it){
		uint32_t id = it->session_id();
		IM::BaseDefine::ContactSessionInfo* info = new IM::BaseDefine::ContactSessionInfo(*it);
		auto it2 = m_contact_session.find(id);
		if(it2 == m_contact_session.end()){
			m_contact_session.insert(pair<uint32_t, IM::BaseDefine::ContactSessionInfo*>(id, info));
		} else {
			delete it2->second;
			m_contact_session[id] = info;
		}
	}
}

uint32_t CClient::getMsgList(IM::BaseDefine::SessionType nType, uint32_t nPeerId, uint32_t nMsgId, uint32_t nMsgCnt)
{
	uint32_t nUserId = m_cSelfInfo.user_id();
	log("try to get msg list");
	return g_pConn->getMsgList(nUserId, nType, nPeerId, nMsgId, nMsgCnt);
}

void CClient::onGetMsgList(uint32_t nSeqNo, uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nType, uint32_t nMsgId, uint32_t nMsgCnt, const list<IM::BaseDefine::MsgInfo> &lsMsg)
{
	log("get msg list success");
	log("seqNo: %u, userId: %u, peer_id: %u, msg_id: %u, msgcnt: %u", nSeqNo, nUserId, nPeerId, nMsgId, nMsgCnt);
	for(auto it = lsMsg.begin(); it != lsMsg.end(); ++it){
		uint32_t id = it->msg_id();
		log("msgid: %u, FromID: %u, msg: %s", id, it->from_session_id(), it->msg_data().c_str());
		IM::BaseDefine::MsgInfo* info = new IM::BaseDefine::MsgInfo(*it);
		auto it2 = m_msg.find(id);
		if(it2 == m_msg.end()){
			m_msg.insert(pair<uint32_t, IM::BaseDefine::MsgInfo*>(id, info));
		} else {
			delete it2->second;
			m_msg[id] = info;
		}
	}
}

void CClient::onRecvMsg(uint32_t nSeqNo, uint32_t nFromId, uint32_t nToId, uint32_t nMsgId, uint32_t nCreateTime, IM::BaseDefine::MsgType nMsgType, const string &strMsgData)
{
	log("seqNo: %u, fromid: %u, toid: %u, msgid: %u, createtime: %u, msg: %s", nSeqNo, nFromId, nToId, nMsgId, nCreateTime, strMsgData.c_str());
}

