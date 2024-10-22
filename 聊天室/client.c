#include "header.h"

int flg = 0; // 私聊标志,置1为私聊

// 聊天界面显示函数
void display(char chat[][256], char *name)
{
    // 获取上线时间
    time_t tm1 = 0;
    char *timstr = NULL;
    tm1 = time(NULL);
    timstr = asctime(localtime(&tm1));
    system("clear");
    // 输出聊天界面头部
    printf("\t\033[33m\033[33m------------------------原神聊天室-------------------------\n\t\033[33m\033[31m            %s", timstr);
    // 打印聊天内容
    for (int i = 0; i < 20; i++)
    {
        if (strlen(chat[i]) != 0)
        {
            // 根据内容类型设置不同的颜色
            if (strstr(chat[i], "加入了聊天室") != NULL || strstr(chat[i], "退出了聊天室") != NULL)
            {
                printf("\033[33m\033[32m%s", chat[i]); // 加入或退出聊天室为绿色
            }
            else if (strstr(chat[i], "悄悄地对你说") != NULL || strstr(chat[i], "你悄悄地对") != NULL)
            {
                printf("\033[33m\033[36m%s", chat[i]); // 私聊为青色
            }
            else
            {
                printf("\033[33m\033[37m%s", chat[i]); // 公开为白色
            }
        }
        else
        {
            printf("\n");
        }
    }
    // 输出界面底部
    printf("\033[33m\033[33m---------------------------------------------------------\n");
    printf("\033[33m\033[31m%s>>:", name);
    fflush(stdout);
    sleep(1);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        perror("使用: ./client + 服务器IP地址 + 服务器端口号\n");
        return -1;
    }

    int ret = 0;

    // 1. 创建客户端套接字
    int tcp_client = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_client < 0)
    {
        perror("socket error\n");
        return -1;
    }

    // 2. 设置服务器地址
    struct sockaddr_in USER_ROLE;
    memset(&USER_ROLE, 0, sizeof(USER_ROLE));
    USER_ROLE.sin_family = AF_INET;
    USER_ROLE.sin_port = htons(atoi(argv[2]));
    USER_ROLE.sin_addr.s_addr = inet_addr(argv[1]);

    // 3. 连接服务器
    ret = connect(tcp_client, (struct sockaddr *)&USER_ROLE, sizeof(USER_ROLE));
    if (ret < 0)
    {
        printf("连接失败..\n");
        return -1;
    }
    // 4. 设置select相关变量
    fd_set rset = {0};
    struct timeval tm = {0};
    int count = 0;
    int len = 0;
    char recvbuf[256];
    char sendbuf[256];

    // 5. 聊天记录
    int i = 0, j = 0;
    char chat[20][256] = {0}; // 存储最多20条消息

    // 6. 输入用户名(登录注册功能)
    char choices[3];
    char name[16] = {0};
    char password[16] = {0};
    char msg_login[32] = {0};
    printf("请选择 (1)注册 (2)登录\n");
    scanf("%s", choices);
    send(tcp_client, choices, strlen(choices), 0);
    // memset(choices, 0, sizeof(choices));
    // recv(tcp_client, choices, sizeof choices, 0);
    if (atoi(choices) == 1)
    {
    USER_INSERT:
        // 注册
        memset(password, 0, sizeof(password));
        memset(msg_login, 0, sizeof(msg_login));
        memset(name, 0, sizeof(name));
        printf("请输入要注册的账号:");
        scanf("%s", name);
        printf("请输入要注册的密码:");
        scanf("%s", password);
        sprintf(msg_login, "%s+%s", name, password);
        send(tcp_client, msg_login, strlen(msg_login), 0);
        memset(choices, 0, sizeof(choices));
        recv(tcp_client, choices, sizeof choices, 0);
        if (atoi(choices) == 8)
            goto USER_INSERT;
        printf("注册成功请登录\n");
    }
USER_LOGIN:
    memset(password, 0, sizeof(password));
    memset(msg_login, 0, sizeof(msg_login));
    memset(name, 0, sizeof(name));
    printf("请输入账号:");
    scanf("%s", name);
    printf("请输入密码:");
    scanf("%s", password);
    sprintf(msg_login, "%s+%s", name, password);
    send(tcp_client, msg_login, strlen(msg_login), 0);
    memset(choices, 0, sizeof(choices));
    recv(tcp_client, choices, sizeof choices, 0);
    if (atoi(choices) == 8)
        goto USER_LOGIN;
    // scanf("%s", name);
    // send(tcp_client, name, strlen(name), 0);  // 发送用户名给服务器

    printf("欢迎来到原神聊天室..\n");
    sleep(1);
    char *loading[7] = {"风", "火", "水", "雷", "冰", "岩"};
    for (int i = 0; i < 6; i++)
    {
        printf("\r%s", loading[i]);
        fflush(stdout);
        usleep(1000000);
    }
    // 7. 开始处理聊天数据
    while (ret >= 0)
    {
        FD_SET(STDIN_FILENO, &rset); // 监控标准输入
        FD_SET(tcp_client, &rset);   // 监控客户端套接字
        tm.tv_sec = 5;               // 超时时间为5秒

        count = select(tcp_client + 1, &rset, NULL, NULL, &tm);

        /****** 处理接收的数据 ******/
        if (FD_ISSET(tcp_client, &rset))
        {
            memset(recvbuf, 0, 256);
            recv(tcp_client, recvbuf, 256, 0);

            if (i < 19)
            {
                strcpy(chat[i], recvbuf);
                i++;
            }
            else
            {
                for (j = 0; j < 19; j++)
                {
                    strcpy(chat[j], chat[j + 1]);
                }
                strcpy(chat[j], recvbuf);
            }
            display(chat, name); // 显示聊天界面
        }

        /****** 处理发送的数据 ******/
        if (FD_ISSET(STDIN_FILENO, &rset))
        {
            memset(sendbuf, 0, 256);
            read(STDIN_FILENO, sendbuf, sizeof(sendbuf)); // 从标准输入读取用户输入

            send(tcp_client, sendbuf, strlen(sendbuf), 0); // 发送给服务器

            if (strncasecmp(sendbuf, "quit", 4) == 0)
            { // 用户输入"quit"则退出
                break;
            }
        }

        if (ret < 0)
            break;
    }

    // 关闭客户端套接字
    close(tcp_client);
    return 0;
}
