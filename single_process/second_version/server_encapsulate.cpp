/*
 * 程序名：server_encapsulate，此程序用于演示封装socket通信的服务端
*/
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
using namespace std;

class tcp_server {
    int listenfd;  //监听的socket，即监听端口。-1表示没有初始化。
    bool isBind = false;
    bool isListen = false;
    int clientfd;  //客户连上来的socket，-1表示客户端未连接。
    bool accepted = false;
    string client_ip; //记录客户端的ip地址。
    unsigned short port; //记录服务端的端口号

public:
    tcp_server() :listenfd(-1), clientfd(-1) {
        listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (listenfd == -1) {
            perror("socket");
        }
    };

    ~tcp_server() {
        clientfd_close();
        listenfd_close();
        isBind = false;
        isListen = false;
        accepted = false;
        client_ip.clear();
        port = 0;
    };

    //默认开启所有ip，但仍需指明端口号
    bool bind(const char* port) {
        if (listenfd == -1) {
            cout << "socket创建失败，无法绑定" << endl;
            return false;
        }
        if (isBind) {
            cout << "socket已经绑定其他端口" << endl;
            return false;
        }
        sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(atoi(port));
        if (::bind(listenfd, (sockaddr*)(&server_addr), sizeof(server_addr)) != 0) {
            perror("bind"); return false;
        }
        isBind = true;
        this->port =  atoi(port);
        return true;
    };
    bool listen() {
        if (listenfd == -1) {
            cout << "socket创建失败，无法监听" << endl;
            return false;
        }
        if (!isBind) {
            cout << "ip还未绑定，无法监听" << endl;
            return false;
        }
        if (isListen) {
            cout << "正在监听中..." << endl;
            return false;
        }
        if (::listen(listenfd, 5)) {
            perror("listen");
            return false;
        }
        isListen = true;
        return true;
    };
    bool accept() {
        if (listenfd == -1) {
            cout << "socket创建失败，无法与客户端连接" << endl;
            return false;
        }
        if (!isBind) {
            cout << "ip还未绑定，无法与客户端连接" << endl;
            return false;
        }
        if (!isListen) {
            cout << "还未监听，无法与客户端连接" << endl;
            return false;
        }
        //客户端的信息，里面的都是大端序的。
        sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        // accept 函数的第二第三个参数如果填0，表示不关心客户端的IP地址等信息。
        clientfd = ::accept(listenfd, (sockaddr*)(&client_addr), &addr_len);
        if (clientfd == -1) {
            perror("accept");
            return false;
        }
        client_ip = inet_ntoa(client_addr.sin_addr); //把客户端的地址从大端序转换成字符串。
        cout << "客户端已连接！" << endl;
        accepted = true;
        return true;
    }
    bool send(const string& buffer) {
        if(!accepted)return false;
        if (::send(clientfd, buffer.data(), buffer.size(), 0) <= 0) {
            perror("send");
            return false;
        }
        //cout << "发送: " << buffer << endl;
        return true;
    };
    bool recv(string& buffer, const size_t maxlen) {
        if (!accepted) {
            cout << "客户端还没有连接" << endl;
            return false;
        }
        buffer.clear();
        buffer.resize(maxlen);
        int readn = ::recv(clientfd, &buffer[0], buffer.size(), 0);
        if (readn <= 0) {
            perror("receive");
            buffer.clear();
            return false;
        }
        buffer.resize(readn);
        return true;
    };
    bool clientfd_close() {
        if(clientfd==-1)return false;
        ::close(clientfd);
        clientfd = -1;
        return true;
    };
    bool listenfd_close(){
        if(listenfd==-1){
            return false;
        }
        ::close(listenfd);
        listenfd = -1;
        return true;

    }
    const string & get_client_ip() const
    {
        return client_ip;
    }
};



int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cout << "Using:./demo2 通讯端口\nExample:./demo2 5005\n\n";   // 端口大于1024，不与其它的重复。
        cout << "注意：运行服务端程序的Linux系统的防火墙必须要开通5005端口。\n";
        cout << "      如果是云服务器，还要开通云平台的访问策略。\n\n";
        return -1;
    }

    tcp_server server_0;
    server_0.bind(argv[1]);
    server_0.listen();
    server_0.accept();
    cout<<server_0.get_client_ip()<<endl;
    // 第5步：与客户端通信，接收客户端发过来的报文后，回复ok。
    string buffer;
    while (true)
    {

        if (server_0.recv(buffer, 1024))
        {
            cout << "接收：" << buffer << endl;
        }
        else
        {
          break;
        }

        buffer = "已收到";  // 生成回应报文内容。
        // 向客户端发送回应报文。
        if (server_0.send(buffer))
        {
            cout << "发送：" << buffer << endl;
        }
    }
}

