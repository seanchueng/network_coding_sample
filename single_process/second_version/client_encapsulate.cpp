/*
 * 程序名：client_encapsulate.cpp，此程序用于演示socket的客户端的封装:
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

class tcp_client{
public:
  tcp_client():clientfd(-1){};
  ~tcp_client(){
    close();
  };


  //发起和服务器的连接，如果成功返回true,失败返回false。
  //第一个参数是服务器的ip，第二个是服务器的端口号。
  bool connect(const string & ip_addr_str, const unsigned short &port){
    if(clientfd != -1) return false; //如果该socket已经连接，则直接返回失败。
    this->ip_addr_str = ip_addr_str;this->port = port;
    clientfd = socket(AF_INET,SOCK_STREAM,0);
    if(clientfd==-1){
      perror("socket");return false;
    }
    sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    hostent* h = gethostbyname(ip_addr_str.c_str());
    if(!h){
      cout<<"get host by name failed."<<endl;
      ::close(clientfd);clientfd = -1 ; //这里要记得把clientfd 设为-1；
      return false;
    }
    memcpy(&server_addr.sin_addr, h->h_addr,h->h_length); //三个参数分布是目的地址，源地址和长度。

    if(::connect(clientfd,(sockaddr*)&server_addr,sizeof(server_addr))==-1){
      perror("connect");
      ::close(clientfd);clientfd = -1; //这里要记得把clientfd 设为-1；
      return false;
    }
    
    return true;
  };


  bool send( const string & buffer){  //注意这里不要使用const char *
    if(clientfd == -1) return false;
    
    if(::send(clientfd,buffer.data(), buffer.size(),0)<=0){  //这里string成员函数data()用于获取首地址。
      //c_str()可以替换data(),length()可以替换size(),但不推荐注释里面的这两种，因为他们只针对了文本而忽略了发生了内容可以是一块内存。
      perror("send");
      cout<<"发送失败..."<<endl;
      return false;
    }
    return true;
  };


 // buffer 存放接收到报文的内容，maxlen 本次接收报文的最大长度。
  bool recv( string & buffer ,const size_t maxlen){
    if(clientfd == -1 ) return false; //判断当前是否连接。
    buffer.clear(); //清空容器
    buffer.resize(maxlen);
    int receive_size = ::recv(clientfd,&buffer[0],buffer.size(),0); //recv的返回值。 -1 失败  0 socket连接已经断开  >0 成功的接收到了数据。
    if (receive_size<=0) {
      perror("receive");
      buffer.clear();  //返回前清空容器；
      return false;
    }
    buffer.resize(receive_size); //由maxlen变为实际大小。但是似乎没有考虑到buffer溢出的问题，即接收到的数据比缓冲区大。
    return true;
  }

  bool close(){
    if(clientfd==-1)return false;
    ::close(clientfd);
    clientfd = -1;
    return true;
  }

private:
  int clientfd ; //客户端socket,-1表示未连接或者已断开； >=0 表示有效的socket。
  string ip_addr_str ; //服务器端口的IP或者域名
  unsigned short port ; //端口号
};





int main(int argc,char *argv[])
{
  if (argc!=3)
  {
    cout << "Using:./demo1 服务端的IP 服务端的端口\nExample:./demo1 192.168.101.139 5005\n\n"; 
    return -1;
  }


  tcp_client client_0;
  if(client_0.connect(argv[1],atoi(argv[2]))==false){
    perror("connect()");
    return -1;
  }//这里还应该判断一下连接是否成功。


  string buffer;
  for (int ii=0;ii<5;ii++)  // 循环3次，将与服务端进行三次通讯。
  {
    buffer = "这是第"+to_string(ii)+"个超级女生，编号"+to_string(ii)+"。";  // 生成请求报文内容。
    // 向服务端发送请求报文。
    client_0.send(buffer);
    cout << "发送：" << buffer << endl;

    // 接收服务端的回应报文，如果服务端没有发送回应报文，recv()函数将阻塞等待。

    client_0.recv(buffer,1024);
    cout << "接收：" << buffer << endl;

    sleep(1);
  }
 
  // 第4步：关闭socket，释放资源。
  client_0.close();
}
