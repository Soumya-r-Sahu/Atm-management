#include <mysql/mysql.h>

// Example: Log transaction
bool logTransaction(int cardNumber, const char* transactionType, float amount, bool success) {
    MYSQL* conn = initMySQLConnection();
    if (conn == NULL) {
        return false;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "INSERT INTO Transactions (cardNumber, transactionType, amount, success, timestamp) "
             "VALUES (%d, '%s', %.2f, %d, NOW())",
             cardNumber, transactionType, amount, success);

    if (mysql_query(conn, query)) {
        write_error_log(mysql_error(conn));
        mysql_close(conn);
        return false;
    }

    mysql_close(conn);
    return true;
}