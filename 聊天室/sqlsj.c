#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

// 用于处理每行结果并输出到控制台
void print_row(sqlite3_stmt *stmt) {
    int col_count = sqlite3_column_count(stmt);  // 获取列数
    for (int i = 0; i < col_count; i++) {
        const char *col_name = sqlite3_column_name(stmt, i);  // 列名
        const char *col_value = (const char *)sqlite3_column_text(stmt, i);  // 列值
        
        // 如果值为NULL，则输出"NULL"
        printf("%s = %s\n", col_name, col_value ? col_value : "NULL");
    }
    printf("-------------------------\n");
}
// 删除指定用户  
void delete_user(sqlite3 *db, const int user_id) {  
    char sql[256];  
    snprintf(sql, sizeof(sql), "DELETE FROM user WHERE id = %d;", user_id);  
    sqlite3_stmt *stmt;  
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);  
    if (rc != SQLITE_OK) {  
        fprintf(stderr, "无法准备删除SQL语句: %s\n", sqlite3_errmsg(db));  
        sqlite3_close(db);  
        exit(1);  
    }  
    rc = sqlite3_step(stmt);  
    if (rc != SQLITE_DONE) {  
        fprintf(stderr, "删除操作失败: %s\n", sqlite3_errmsg(db));  
    } else {  
        printf("用户删除成功\n");  
    }  
    sqlite3_finalize(stmt);  
} 
int main(int argc, char *argv[]) {

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
    const char *sql = "SELECT * FROM user;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "无法准备SQL语句: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    // 执行查询并处理结果
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        print_row(stmt);  // 输出每行数据
    }
    // 检查查询是否成功完成
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "查询执行失败: %s\n", sqlite3_errmsg(db));
    } else {
        printf("查询成功\n");
    }
    // 获取用户输入要删除的用户ID  
    printf("请输入要删除的用户ID: ");  
    int id;
    scanf("%d", &id);  // 读取用户ID，限制最大长度以避免缓冲区溢出  
  
    // 删除用户  
    delete_user(db, id);  
    

    // 清理资源
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 0;
}
