#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

// 服务器的ip和端口
#define SERV_IP "127.0.0.1"
#define SERV_PORT 6666

class ClientSocket {
    public:
        int cfd;// 用于写入数据传输给服务端的socket的文件描述符
        struct sockaddr_in serv_addr; // 因为要连接服务端，这里的sockadd_in是用于指定服务端的ip和端口
        void init() {
            cfd = socket(AF_INET, SOCK_STREAM, 0);
            // bind()  可以不调用bind()， linux会隐式地绑定
            bzero(&serv_addr, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(SERV_PORT);
            inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr.s_addr); // 调用ip转换函数，把字符串ip转化为网络字节序
            connect(cfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        }

        void inputAndSend(const char* buf) {
            //char buf[BUFSIZ];
            //fgets(buf, sizeof(buf), stdin);  // 读一行
            // 写入到cfd中，传输给服务端
            write(cfd, buf, strlen(buf)); // 注意不要写成sizeof(buf)，sizeof是在内存中所占的大小，strlen是到第一个'\0'位止。
            // read在读socket时默认时阻塞的，阻塞等待服务端传输数据
        }
        
        void over() {
            close(cfd);
        }
};

class ServerSocket {
    public:
        int lfd; // 用于监听的socket的文件描述符，真正用于通信的套接字是接下来accept函数返回的cfd套接字
        int cfd; // 已连接的客户端的socket的文件描述符, 以便一会儿read用
        struct sockaddr_in serv_addr;
        struct sockaddr_in clie_addr;
        socklen_t clie_addr_len;

        void init() {
            lfd = socket(AF_INET, SOCK_STREAM, 0);
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(SERV_PORT); // 注意这里，要把小端存储的端口号改为大端存储
            inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr.s_addr); // 调用ip转换函数，把字符串ip转化为网络字节序

            bind(lfd, (struct sockaddr * )&serv_addr, sizeof(serv_addr));

            listen(lfd, 128); // 最大连接与待连接数设为128

                            // 作为accept的第二个参数，为传出参数，传出的是客户端的sockadd_in
            clie_addr_len = sizeof(clie_addr); // 作为accept的第三个参数，为传入传出参数，之所以要单独定义出来是因为要传出
            cfd = accept(lfd, (struct sockaddr *)&clie_addr, &clie_addr_len);
        }
        char* recToArr() {
            char clie_IP[BUFSIZ];
            printf("Client IP: %s, client port: %d\n", 
                inet_ntop(AF_INET, &clie_addr.sin_addr.s_addr, clie_IP, sizeof(clie_IP)),
                ntohs(clie_addr.sin_port));

            char buf[BUFSIZ]; // 给read使用，存储读出的数据，BUFSIZ宏是系统用来专门给buf赋长度的宏，为8k（Default buffer size）

            int len; // read的返回值，是读入字符的长度
            len = read(cfd, buf, sizeof(buf));
            buf[len] = '\0';
            char* res = buf;
            printf("收到的数据是 %s\n", res);
            return res;
        }
        void over() {
            close(lfd);
            close(cfd);
        }
};
