#include "header.h"
#include <sqlite3.h>
// 全局变量
client_t clients[20] = {0}; // 客户端数组
int client_count = 0;       // 在线人数
int is_private_chat = 0;    // 私聊标志，置1为私聊

// 转发消息给所有客户端，并保存到文件
void broadcast_message(char *message)
{
    for (int i = 0; i < 20; i++)
    {
        if (clients[i].client_socket > 0)
        {
            send(clients[i].client_socket, message, strlen(message), 0);
        }
    }
    // 记录聊天记录
    int fd = open("chat.txt", O_RDWR | O_CREAT | O_APPEND, 0666);
    write(fd, message, strlen(message));
    close(fd);
}
// 私聊信息保存
void save_private_chat(char *sender, char *receiver, char *message)
{
    char filename[50] = {0};
    char filename1[50] = {0};
    int fd=0;
    sprintf(filename, "%s与%s私聊记录.txt", sender, receiver);
    sprintf(filename1, "%s与%s私聊记录.txt", receiver, sender);
    if (access(filename1, F_OK) == 0)
    {
        fd = 
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        (filename1, O_RDWR | O_CREAT | O_APPEND, 0666);
        if(fd == -1)
        {
            perror("open error");
            return;
        }
    }
    else
    {
        fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0666);
        if(fd == -1)
        {
            perror("open1 error");
            return;
        }
    }

    char log_entry[512] = {0};
    snprintf(log_entry, sizeof(log_entry), "私聊 [%s -> %s]: %s\n", sender, receiver, message);
    write(fd, log_entry, strlen(log_entry));
    close(fd);
}
// 添加用户
void add_user(int client_socket, char *name)
{
    for (int i = 0; i < 20; i++)
    {
        if (clients[i].client_socket == client_socket)
        {
            strcpy(clients[i].name, name);
            printf("用户已添加到数组：%d %s\n", clients[i].client_socket, clients[i].name);
            break;
        }
    }
}
/*数据库登录注册的操作*/
// 插入新用户到数据库  
int insert_user(const char *rolename, const char *password) {  

    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    // 打开数据库
    rc = sqlite3_open("user.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "无法打开数据库: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    const char *sql = "INSERT INTO user (rolename, password) VALUES (?, ?);";  
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);  
    if (rc != SQLITE_OK) {  
        fprintf(stderr, "无法准备SQL插入语句: %s\n", sqlite3_errmsg(db));  
        return 1;  
    }  
    rc = sqlite3_bind_text(stmt, 1, rolename, -1, SQLITE_STATIC);  
    if (rc != SQLITE_OK) {  
        fprintf(stderr, "无法绑定rolename参数: %s\n", sqlite3_errmsg(db));  
        sqlite3_finalize(stmt);  
        return 1;  
    }  
    rc = sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);  
    if (rc != SQLITE_OK) {  
        fprintf(stderr, "无法绑定password参数: %s\n", sqlite3_errmsg(db));  
        sqlite3_finalize(stmt);  
        return 1;  
    }  
    rc = sqlite3_step(stmt);  
    if (rc != SQLITE_DONE) {  
        fprintf(stderr, "插入用户失败: %s\n", sqlite3_errmsg(db));  
        sqlite3_finalize(stmt);  
        return 1;  
    }  
    // 清理资源
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}
int sql_login_server(char *username, char *password)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    // 打开数据库
    rc = sqlite3_open("user.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "无法打开数据库: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    // 准备SQL语句（使用占位符）
    const char *sql = "SELECT * FROM user WHERE rolename = ? AND password = ?;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "无法准备SQL语句: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    // 绑定username参数
    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "无法绑定name参数: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 绑定password参数
    rc = sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "无法绑定name参数: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 执行查询并处理结果
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // print_row(stmt);  // 输出每行数据
        printf("登陆成功\n");
        // 清理资源
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }
    // 检查查询是否成功完成
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "查询执行失败: %s\n", sqlite3_errmsg(db));
        return 1;
    } 

    // 清理资源
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 1;
}
// 处理客户端通信的线程函数
void *client_handler(void *arg)
{
    int client_socket = *(int *)arg;
    /*
        数据库信息比对登录或注册
    */
    char sendbuf[512] = {0};
    char recvbuf[256] = {0};
    char buf_len[3]={0};
    recv(client_socket, buf_len, sizeof(buf_len), 0);
    if(atoi(buf_len)==1)
    {
    USER_INSERT:
        //注册
        recv(client_socket, recvbuf, sizeof(recvbuf), 0);
        char name[16] ={0};
        strcpy(name,strtok(recvbuf,"+"));
        if(insert_user(name,strtok(NULL,"+"))!=0)
        {
            strcpy(buf_len,"8");
            send(client_socket, buf_len, strlen(buf_len), 0);
            goto USER_INSERT;
        }
        strcpy(buf_len,"1");
        send(client_socket, buf_len, strlen(buf_len), 0);
        printf("注册成功\n");
    }
    USER_LOGIN:
    // 接收客户端发来的用户名
    recv(client_socket, recvbuf, sizeof(recvbuf), 0);
    char name[16] ={0};
    strcpy(name,strtok(recvbuf,"+"));
    if(sql_login_server(name,strtok(NULL,"+"))!=0)
    {
        strcpy(buf_len,"8");
        send(client_socket, buf_len, strlen(buf_len), 0);
        goto USER_LOGIN;
        sleep(1);
    }
    client_count++;
    printf("在线人数: %d\n", client_count);
    add_user(client_socket, name);
    printf("%s 已上线\n", name);
    // 通知所有用户新用户加入
    char notification[256] = {0};
    snprintf(notification, sizeof(notification), "%s 加入了聊天室, 当前在线用户：", name);
    for (int i = 0; i < client_count; i++)
    {
        strcat(notification, clients[i].name);
        strcat(notification, " ");
    }
    strcat(notification, "\n");
    broadcast_message(notification);

    // 添加消息头（用户名：）
    char name_header[256] = {0};
    snprintf(name_header, sizeof(name_header), "%s: ", name);

    int len = 0;
    while (1)
    {
        bzero(recvbuf, sizeof(recvbuf));
        len = recv(client_socket, recvbuf, sizeof(recvbuf), 0);

        if (len <= 0)
        {
            // 处理客户端断开连接
            for (int i = 0; i < 20; i++)
            {
                if (strcmp(clients[i].name, name) == 0)
                {
                    clients[i].client_socket = 0;
                    bzero(clients[i].name, 16);
                    break;
                }
            }
            snprintf(notification, sizeof(notification), "%s 退出了聊天室, 当前在线用户：", name);
            for (int i = 0; i < 20; i++)
            {
                if (clients[i].client_socket > 0)
                {
                    strcat(notification, clients[i].name);
                    strcat(notification, " ");
                }
            }
            strcat(notification, "\n");
            broadcast_message(notification);
            printf("%s 退出了聊天室\n", name);
            client_count--;
            close(client_socket);
            break;
        }

        // 检测是否私聊
        if (strncmp(recvbuf, "@", 1) == 0)
        {
            is_private_chat = 1;
        }

        // 私聊处理
        if (is_private_chat == 1)
        {
            char prefix[16] = "";
            char target_name[16] = "";
            char private_msg[256] = "";
            int ret = sscanf(recvbuf, "%s %s %s", prefix, target_name, private_msg);

            if (ret != 3)
            {
                char error[256] = "@ 名字 要发送的消息\n";
                send(client_socket, error, strlen(error), 0);
            }
            else
            {
                printf("%s 进行私聊\n", name);
                snprintf(sendbuf, sizeof(sendbuf), "%s 悄悄地对你说: %s\n", name, private_msg);
                for (int i = 0; i < 20; i++)
                {
                    if (strcmp(clients[i].name, target_name) == 0)
                    {
                        send(clients[i].client_socket, sendbuf, strlen(sendbuf), 0);
                        snprintf(sendbuf, sizeof(sendbuf), "你悄悄地对 %s 说: %s\n", clients[i].name, private_msg);
                        save_private_chat(name, clients[i].name, private_msg);
                        send(client_socket, sendbuf, strlen(sendbuf), 0);
                        break;
                    }
                }
                is_private_chat = 0;
            }
        }
        else
        {
            // 群发消息
            if (strncasecmp(recvbuf, "quit", 4) == 0)
            {
                printf("[%s] 退出了聊天室\n", name);
                client_count--;
                for (int i = 0; i < 20; i++)
                {
                    if (strcmp(clients[i].name, name) == 0)
                    {
                        clients[i].client_socket = 0;
                        bzero(clients[i].name, 16);
                        break;
                    }
                }
                snprintf(notification, sizeof(notification), "%s 退出了聊天室, 当前在线用户：", name);
                for (int i = 0; i < 20; i++)
                {
                    if (clients[i].client_socket > 0)
                    {
                        strcat(notification, clients[i].name);
                        strcat(notification, " ");
                    }
                }
                strcat(notification, "\n");
                broadcast_message(notification);
                printf("当前在线人数: %d\n", client_count);
                close(client_socket);
                break;
            }

            // 发送群聊消息
            snprintf(sendbuf, sizeof(sendbuf), "%s %s", name_header, recvbuf);
            broadcast_message(sendbuf);
            printf("%s\n", sendbuf);
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    system("clear");
    int ret;

    // 1. 创建套接字
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("创建套接字失败");
        return -1;
    }
    // 2. 绑定IP和端口
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(55555);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0)
    {
        perror("绑定失败");
        return -1;
    }
    // 3. 开始监听
    listen(server_socket, 10);
    printf("原神聊天室启动成功，等待用户连接...\n");

    pthread_t thread_id;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    while (1)
    {
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket > 0)
        {
            printf("人物 [%s:%d] 已上线\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            for (int i = 0; i < 20; i++)
            {
                if (clients[i].client_socket == 0)
                {
                    clients[i].client_socket = client_socket;
                    break;
                }
            }

            pthread_create(&thread_id, NULL, client_handler, &client_socket);
            pthread_detach(thread_id);
        }
    }
    close(server_socket);
    return 0;
}
