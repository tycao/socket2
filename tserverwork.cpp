#include "tserverwork.h"

TServerWork::TServerWork() : m_ep(asio::ip::tcp::v4(), 20175), m_acc(m_iosrv, m_ep), m_userID(100)
{

}

void TServerWork::Run(void)
{
    TClnLink *pCln = new TClnLink(m_iosrv);
    m_acc.async_accept(pCln->m_sock, m_ep, bind(&TServerWork::Accept, this, TClnLinkPtr(pCln), _1));
    m_iosrv.run();
}

void TServerWork::Accept(TClnLinkPtr cln, const system::error_code &ec)
{
    if (ec)
    {
        cout << ec.message() << endl;
        return; //只是代表让函数直接结束，且不返回任何值
    }
    TClnLink *pCln = new TClnLink(m_iosrv);
    m_acc.async_accept(pCln->m_sock, m_ep, bind(&TServerWork::Accept, this, TClnLinkPtr(pCln), _1));

    //业务处理
    Work(cln);
}

void TServerWork::Work(TClnLinkPtr cln)
{
    cout << cln->m_sock.remote_endpoint().address() << " : "
         << cln->m_sock.remote_endpoint().port() << " ";

    char buf[g_bufsize + 1] = {0}; //不初始化的话，全是乱七八糟的值。不会默认初始化该字符数组
    cln->m_sock.read_some(asio::buffer(buf, g_bufsize));
    string sFlag = buf;
    auto it = m_users.find(sFlag);
    if (m_users.end() == it)
    {
        TClnUser *pUser = new TClnUser;
        TClnUserPtr UserPtr(pUser);
        pUser->m_name = "Guest" + lexical_cast<string>(m_userID++);
        pUser->m_flag = sFlag;
        cout << sFlag << " - " << pUser->m_name << endl;
        pUser->m_send = cln;
        m_users.insert(make_pair(sFlag, UserPtr));

        //等待客户端发消息
        cln->m_sock.async_read_some(asio::buffer(cln->m_buf, g_bufsize),
                                        bind(&TServerWork::Read, this, UserPtr, _1, _2));
    }
    else
    {
        it->second->m_recv = cln;
    }

}

void TServerWork::Read(TClnUserPtr cln, const system::error_code &ec, size_t sz)
{
    if (ec)
    {
        cout << ec.message() << endl;
        return;
    }
    //再次新增一个读操作
    string sMsg = cln->m_send->m_buf;
    memset(cln->m_send->m_buf, 0, g_bufsize);
    cln->m_send->m_sock.async_read_some(asio::buffer(cln->m_send->m_buf, g_bufsize),
                                        bind(&TServerWork::Read, this, cln, _1, _2));

    //具体处理 用户发消息
    string sFullMsg = cln->m_name + ": " + sMsg;
    cout << sFullMsg << endl;
    for (pair<string, TClnUserPtr> u : m_users)
    {
        std::function<void(const system::error_code&, size_t)> fWrite = [&](const system::error_code &ec, size_t sz)
        {

        }; //lambda 表达式
        if (nullptr != u.second->m_recv.get())
            u.second->m_recv->m_sock.async_write_some(asio::buffer(sFullMsg), fWrite);
    }
}
