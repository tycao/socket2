#pragma once

#include "stdtalk.h"
//定义一个常量
constexpr int g_bufsize = 1024 * 16;

class TClnLink
{
public:
    TClnLink(asio::io_service &iosrv) : m_sock(iosrv) {}
    asio::ip::tcp::socket m_sock;

    //定义缓冲区
    char m_buf[g_bufsize + 1] = {0};
};
//定义TClLink类的类型别名
typedef std::shared_ptr<TClnLink> TClnLinkPtr; //using 是定义模版别名， typedef是类型别名
//template<typename T>
//using MyVector = vector<T>; //这两句是模版别名,用using


class TClnUser
{
public:
    string m_flag, m_name;
    TClnLinkPtr m_send, m_recv;
};
//定义TClnUser类的类型别名
typedef std::shared_ptr<TClnUser> TClnUserPtr;

class TServerWork
{
public:
    TServerWork();

    void Run(void);

protected:
    void Accept(TClnLinkPtr cln, const system::error_code &ec);

    void Work(TClnLinkPtr cln);

    void Read(TClnUserPtr cln, const system::error_code &ec, size_t sz);

private:
    asio::io_service m_iosrv;
    asio::ip::tcp::endpoint m_ep;
    asio::ip::tcp::acceptor m_acc;
    map<string, TClnUserPtr> m_users;
    std::atomic_int m_userID;
};
