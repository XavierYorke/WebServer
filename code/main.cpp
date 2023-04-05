#include "server/webserver.h"

//需要在命令行指定端口号，通过参数传递
int main(int argc, char* argv[]) {
    if (argc <= 1) {
        printf("请按照如下格式运行：./%s port_number\n", basename(argv[0]));
        exit(-1);
    }

    // 获取端口号
    int port = atoi(argv[1]);

    WebServer webserver(port);
    printf("webserver init\n");
    webserver.start();

    return 0;
}
