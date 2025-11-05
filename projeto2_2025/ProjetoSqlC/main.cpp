#include <iostream>
#include <mysql.h>

int main(){
    MYSQL *connect = mysql_init(NULL);
    !connect ? std::cerr << "MYSQL Nao foi iniciado.\n"
             : std::cout << "MYSQL Conectado com sucesso!\n";
    mysql_close(connect);
}