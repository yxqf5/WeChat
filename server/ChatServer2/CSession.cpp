#include "CSession.h"
#include "CServer.h"
#include<iostream>
#include<sstream>
#include<json/json.h>
#include<json/value.h>
#include<json/reader.h>
#include"LogicSystem.h"

using std::endl;
using std::cout;

CSession::CSession(boost::asio::io_context& io_context, CServer* server) :m_socket(io_context),
                    m_server(server), m_b_close(false), m_b_head_parse(false), m_user_uid(0)
{

	boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    m_session_id = boost::uuids::to_string(a_uuid);
    m_recv_head_node = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);

}



CSession::~CSession() {
	std::cout << "~CSession destruct" << std::endl;
}

tcp::socket& CSession::GetSocket() {
	return m_socket;
}

std::string& CSession::GetSessionId() {
	return m_session_id;
}

void CSession::SetUserId(int uid)
{
	m_user_uid = uid;
}

int CSession::GetUserId()
{
	return m_user_uid;
}


void CSession::Start() {
	AsyncReadHead(HEAD_TOTAL_LEN);
}


//void CSession::AsyncReadHead(int total_len)
//{
//    auto self = shared_from_this();
//    asyncReadFull(HEAD_TOTAL_LEN, [self, this](const boost::system::error_code& ec, std::size_t bytes_transfered) {
//        try {
//            if (ec) {
//                std::cout << "handle read failed, error is " << ec.what() << endl;
//                Close();
//                m_server->ClearSession(m_session_id);
//                return;
//            }
//
//            if (bytes_transfered < HEAD_TOTAL_LEN) {
//                std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
//                    << HEAD_TOTAL_LEN << "]" << endl;
//                Close();
//                m_server->ClearSession(m_session_id);
//                return;
//            }
//
//            m_recv_head_node->Clear();
//            memcpy(m_recv_head_node->_data, _data, bytes_transfered);
//
//            //��ȡͷ��MSGID����
//            short msg_id = 0;
//            memcpy(&msg_id, m_recv_head_node->_data, HEAD_ID_LEN);
//            //�����ֽ���ת��Ϊ�����ֽ���
//            msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
//            std::cout << "msg_id is " << msg_id << endl;
//            //id�Ƿ�
//            if (msg_id > MAX_LENGTH) {
//                std::cout << "invalid msg_id is " << msg_id << endl;
//                m_server->ClearSession(m_session_id);
//                return;
//            }
//            short msg_len = 0;
//            memcpy(&msg_len, m_recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
//            //�����ֽ���ת��Ϊ�����ֽ���
//            msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
//            std::cout << "msg_len is " << msg_len << endl;
//
//            //id�Ƿ�
//            if (msg_len > MAX_LENGTH) {
//                std::cout << "invalid data length is " << msg_len << endl;
//                m_server->ClearSession(m_session_id);
//                return;
//            }
//
//            m_recv_msg_node = make_shared<RecvNode>(msg_len, msg_id);
//            AsyncReadBody(msg_len);
//        }
//        catch (std::exception& e) {
//            std::cout << "Exception code is " << e.what() << endl;
//        }
//        });
//}



void CSession::Send(std::string msg, short msgid) {
	std::lock_guard<std::mutex> lock(m_send_lock);
	int send_que_size = m_send_que.size();
	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session: " << m_session_id << " send que fulled, size is " << MAX_SENDQUE << endl;
		return;
	}

	m_send_que.push(make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
	if (send_que_size > 0) {
		return;
	}
	auto& msgnode = m_send_que.front();
	boost::asio::async_write(m_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}


//���б�֤�첽����ִ�е�������
void CSession::Send(char* msg, short max_length, short msgid) {
	std::lock_guard<std::mutex> lock(m_send_lock);
	int send_que_size = m_send_que.size();
	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session: " << m_session_id << " send que fulled, size is " << MAX_SENDQUE << endl;
		return;
	}

	m_send_que.push(make_shared<SendNode>(msg, max_length, msgid));
	if (send_que_size > 0) {
		return;
	}
	auto& msgnode = m_send_que.front();
	boost::asio::async_write(m_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void CSession::Close() {
	m_socket.close();
	m_b_close = true;
}

std::shared_ptr<CSession>CSession::SharedSelf() {
	return shared_from_this();
}

void CSession::AsyncReadBody(int total_len)
{
	auto self = shared_from_this();
	asyncReadFull(total_len, [self, this, total_len](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				std::cout << "handle read failed, error is " << ec.what() << endl;
				Close();
				m_server->ClearSession(m_session_id);
				return;
			}

			if (bytes_transfered < total_len) {
				std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
					<< total_len << "]" << endl;
				Close();
				m_server->ClearSession(m_session_id);
				return;
			}

			memcpy(m_recv_msg_node->_data, _data, bytes_transfered);
			m_recv_msg_node->_cur_len += bytes_transfered;
			m_recv_msg_node->_data[m_recv_msg_node->_total_len] = '\0';
			cout << "receive data is " << m_recv_msg_node->_data << endl;
			//�˴�����ϢͶ�ݵ��߼�������
			LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), m_recv_msg_node));
			//��������ͷ�������¼�
			AsyncReadHead(HEAD_TOTAL_LEN);
		}
		catch (std::exception& e) {
			std::cout << "Exception code is " << e.what() << endl;
		}
		});
}

void CSession::AsyncReadHead(int total_len)
{
	auto self = shared_from_this();
	//asyncReadFull�ڶ���HEAD_TOTAL_LEN��Żᴥ���ص�����
	asyncReadFull(HEAD_TOTAL_LEN, [self, this](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				std::cout << "handle read failed, error is " << ec.what() << endl;
				Close();
				m_server->ClearSession(m_session_id);
				return;
			}

			if (bytes_transfered < HEAD_TOTAL_LEN) {
				std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
					<< HEAD_TOTAL_LEN << "]" << endl;
				Close();
				m_server->ClearSession(m_session_id);
				return;
			}

			m_recv_head_node->Clear();
			memcpy(m_recv_head_node->_data, _data, bytes_transfered);

			//��ȡͷ��MSGID����
			short msg_id = 0;
			memcpy(&msg_id, m_recv_head_node->_data, HEAD_ID_LEN);
			//�����ֽ���ת��Ϊ�����ֽ���
			msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
			std::cout << "msg_id is " << msg_id << endl;
			//id�Ƿ�
			if (msg_id > MAX_LENGTH) {
				std::cout << "invalid msg_id is " << msg_id << endl;
				m_server->ClearSession(m_session_id);
				return;
			}
			short msg_len = 0;
			memcpy(&msg_len, m_recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
			//�����ֽ���ת��Ϊ�����ֽ���
			msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
			std::cout << "msg_len is " << msg_len << endl;

			//id�Ƿ�
			if (msg_len > MAX_LENGTH) {
				std::cout << "invalid data length is " << msg_len << endl;
				m_server->ClearSession(m_session_id);
				return;
			}

			m_recv_msg_node = make_shared<RecvNode>(msg_len, msg_id);
			AsyncReadBody(msg_len);
		}
		catch (std::exception& e) {
			std::cout << "Exception code is " << e.what() << endl;
		}
		});
}

void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> shared_self) {
	//�����쳣����
	try {
		if (!error) {
			std::lock_guard<std::mutex> lock(m_send_lock);
			//cout << "send data " << m_send_que.front()->_data+HEAD_LENGTH << endl;
			m_send_que.pop();
			if (!m_send_que.empty()) {
				auto& msgnode = m_send_que.front();
				boost::asio::async_write(m_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
					std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_self));
			}
		}
		else {
			std::cout << "handle write failed, error is " << error.what() << endl;
			Close();
			m_server->ClearSession(m_session_id);
		}
	}
	catch (std::exception& e) {
		std::cerr << "Exception code : " << e.what() << endl;
	}

}

//��ȡ��������
void CSession::asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	//�����Ҫ�������ݵ�����
	::memset(_data, 0, MAX_LENGTH);
	asyncReadLen(0, maxLength, handler);
}

//��ȡָ���ֽ���
void CSession::asyncReadLen(std::size_t read_len, std::size_t total_len,
	std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	auto self = shared_from_this();
	m_socket.async_read_some(boost::asio::buffer(_data + read_len, total_len - read_len),
		[read_len, total_len, handler, self](const boost::system::error_code& ec, std::size_t  bytesTransfered) {
			if (ec) {
				// ���ִ��󣬵��ûص�����
				handler(ec, read_len + bytesTransfered);
				return;
			}

			if (read_len + bytesTransfered >= total_len) {
				//���ȹ��˾͵��ûص�����
				handler(ec, read_len + bytesTransfered);
				return;
			}

			// û�д����ҳ��Ȳ����������ȡ
			self->asyncReadLen(read_len + bytesTransfered, total_len, handler);
		});
}

LogicNode::LogicNode(shared_ptr<CSession>  session,shared_ptr<RecvNode> recvnode) 
					:_session(session), _recvnode(recvnode) 

{


}