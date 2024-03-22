/*
 * 程序名：client.cpp，此程序用于演示socket的客户端
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
 
int main(int argc,char *argv[])
{
  if (argc!=3)
  {
    cout << "Using:./demo1 服务端的IP 服务端的端口\nExample:./demo1 192.168.101.139 5005\n\n"; 
    return -1;
  }

  // 第1步：创建客户端的socket。  
  int sockfd = socket(AF_INET,SOCK_STREAM,0); //第三个参数填0就可以。创建失败返回-1。
  if (sockfd==-1)
  {
    perror("socket"); return -1;
  }
 
  // 第2步：向服务器发起连接请求。 
  struct hostent* h;    // 用于存放服务端相关信息的结构体。该结构体中的ip是大端序的。通常设置为一个指针用来接收gethostbyname()的返回值。
  if ( (h = gethostbyname(argv[1])) == nullptr )  // 把字符串格式的IP转换成结构体。argv第0个参数是程序名，所以从1开始。如果调用失败，返回空指针。
  { 
    cout << "gethostbyname failed.\n" << endl; close(sockfd); return -1;
  }
  struct sockaddr_in servaddr;              // 用于存放服务端IP和端口的结构体。
  memset(&servaddr,0,sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  memcpy(&servaddr.sin_addr,h->h_addr,h->h_length); // 指定服务端的IP地址。h_addr是库函数定义的别名而已，实际上是h_addr_list[0]
  servaddr.sin_port = htons(atoi(argv[2]));         // 指定服务端的通信端口。先把参数转换成整数，然后转为大端序。
  
  if (connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr))!=0)  // 向服务端发起连接清求。
  { 
    perror("connect"); close(sockfd); return -1; 
  }
  
  // 第3步：与服务端通讯，客户发送一个请求报文后等待服务端的回复，收到回复后，再发下一个请求报文。
  char buffer[1024];
  for (int ii=0;ii<3;ii++)  // 循环3次，将与服务端进行三次通讯。
  {
    int iret;
    memset(buffer,0,sizeof(buffer));
    sprintf(buffer,"这是第%d个超级女生，编号%03d。",ii+1,ii+1);  // 生成请求报文内容。
    // 向服务端发送请求报文。
    if ( (iret=send(sockfd,buffer,strlen(buffer),0))<=0)
    { 
      perror("send"); break; 
    }
    cout << "发送：" << buffer << endl;

    memset(buffer,0,sizeof(buffer));
    // 接收服务端的回应报文，如果服务端没有发送回应报文，recv()函数将阻塞等待。
    if ( (iret=recv(sockfd,buffer,sizeof(buffer),0))<=0)
    {
       cout << "iret=" << iret << endl; break;
    }
    cout << "接收：" << buffer << endl;

    sleep(1);
  }
 
  // 第4步：关闭socket，释放资源。
  close(sockfd);
}
