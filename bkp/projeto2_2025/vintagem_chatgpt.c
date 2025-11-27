#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include <ctype.h>

#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    int _getch(void) {
        struct termios oldt, newt;
        int ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }
#endif

/* --------------------- Configuração de Conexão --------------------- */
/* Troque os valores conforme seu ambiente */
const char *DB_SERVER = "localhost";
const char *DB_USER   = "root";
const char *DB_PASS   = "@vintagem123";
const char *DB_NAME   = "vintagem";

/* --------------------- Estado Global (simples) --------------------- */
int g_id_usuario_logado = 0;
char g_nome_nivel_logado[64] = "";

/* --------------------- Utilitários de UI / Entrada --------------------- */
void limparBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
void pausarTela(const char *msg) {
    if (msg && msg[0] != '\0') printf("%s\n", msg);
    printf("Pressione qualquer tecla para continuar...");
    _getch();
}
void limparTela(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* --------------------- Funções DB básicas --------------------- */

/* Erro padronizado */
void erro_sql(MYSQL *con, const char *contexto) {
    if (con)
        fprintf(stderr, "[ERRO %s]: %s\n", contexto, mysql_error(con));
    else
        fprintf(stderr, "[ERRO %s]: (conexao nula)\n", contexto);
}

/* Conecta e configura charset utf8 */
MYSQL *conectar_db(void) {
    MYSQL *con = mysql_init(NULL);
    if (!con) {
        fprintf(stderr, "mysql_init() falhou\n");
        return NULL;
    }
    if (!mysql_real_connect(con, DB_SERVER, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        fprintf(stderr, "mysql_real_connect() falhou: %s\n", mysql_error(con));
        mysql_close(con);
        return NULL;
    }
    if (mysql_set_character_set(con, "utf8")) {
        fprintf(stderr, "Erro ao setar charset utf8: %s\n", mysql_error(con));
    }
    return con;
}

/* Wrapper para executar prepared statements simples sem resultado (INSERT/UPDATE/DELETE) */
int executar_stmt_simples(MYSQL *con, const char *query, MYSQL_BIND *params, unsigned int param_count) {
    MYSQL_STMT *stmt = NULL;
    int ok = 0;
    stmt = mysql_stmt_init(con);
    if (!stmt) {
        erro_sql(con, "stmt_init");
        return 0;
    }
    if (mysql_stmt_prepare(stmt, query, (unsigned long)strlen(query))) {
        erro_sql(con, "stmt_prepare");
        mysql_stmt_close(stmt);
        return 0;
    }
    if (param_count > 0) {
        if (mysql_stmt_bind_param(stmt, params)) {
            erro_sql(con, "stmt_bind_param");
            mysql_stmt_close(stmt);
            return 0;
        }
    }
    if (mysql_stmt_execute(stmt)) {
        erro_sql(con, "stmt_execute");
        mysql_stmt_close(stmt);
        return 0;
    }
    ok = 1;
    mysql_stmt_close(stmt);
    return ok;
}

/* Executa uma query simples (sem parâmetros) e retorna MYSQL_RES* (caller deve liberar) */
MYSQL_RES *executar_query_result(MYSQL *con, const char *query) {
    if (mysql_query(con, query)) {
        erro_sql(con, "query");
        return NULL;
    }
    MYSQL_RES *res = mysql_store_result(con);
    if (!res && mysql_field_count(con) != 0) {
        erro_sql(con, "store_result");
        return NULL;
    }
    return res;
}

/* --------------------- Funções Genéricas para CRUD/Consulta --------------------- */

/*
  criar_item_retornando_id:
  - Insere um registro em uma tabela com uma coluna-nome única e retorna o ID gerado.
  - Usa prepared statement para evitar injeção.
  - ATENÇÃO: a tabela deve ter auto-increment na PK.
*/
long long criar_item_retornando_id(MYSQL *con, const char *tabela, const char *col_nome, const char *valor_nome) {
    char query[512];
    MYSQL_BIND param[1];
    unsigned long len = (unsigned long)strlen(valor_nome);

    snprintf(query, sizeof(query), "INSERT INTO %s (%s, ativo) VALUES (?, 1)", tabela, col_nome);

    memset(&param, 0, sizeof(param));
    param[0].buffer_type = MYSQL_TYPE_STRING;
    param[0].buffer = (char *)valor_nome;
    param[0].buffer_length = len;

    if (!executar_stmt_simples(con, query, param, 1)) {
        return -1;
    }
    /* mysql_insert_id obtém o último ID da conexão */
    return mysql_insert_id(con);
}

/* verificar_id_ativo: verifica se existe um id e está ativo */
int verificar_id_ativo(MYSQL *con, const char *tabela, const char *col_id, int id_busca) {
    MYSQL_STMT *stmt = NULL;
    char query[256];
    MYSQL_BIND param[1];
    MYSQL_BIND result[1];
    my_bool exists = 0;
    int rc = 0;

    snprintf(query, sizeof(query), "SELECT 1 FROM %s WHERE %s = ? AND ativo = 1 LIMIT 1", tabela, col_id);

    stmt = mysql_stmt_init(con);
    if (!stmt) { erro_sql(con, "stmt_init"); return 0; }
    if (mysql_stmt_prepare(stmt, query, strlen(query))) { erro_sql(con, "stmt_prepare"); mysql_stmt_close(stmt); return 0; }

    memset(&param, 0, sizeof(param));
    param[0].buffer_type = MYSQL_TYPE_LONG;
    param[0].buffer = &id_busca;
    param[0].buffer_length = sizeof(id_busca);

    if (mysql_stmt_bind_param(stmt, param)) { erro_sql(con, "stmt_bind_param"); mysql_stmt_close(stmt); return 0; }
    if (mysql_stmt_execute(stmt)) { erro_sql(con, "stmt_execute"); mysql_stmt_close(stmt); return 0; }

    memset(&result, 0, sizeof(result));
    result[0].buffer_type = MYSQL_TYPE_TINY;
    result[0].buffer = &exists;
    result[0].buffer_length = sizeof(exists);

    if (mysql_stmt_bind_result(stmt, result)) { erro_sql(con, "stmt_bind_result"); mysql_stmt_close(stmt); return 0; }
    if (mysql_stmt_store_result(stmt)) { /* pode ser zero rows */ ; }

    if (mysql_stmt_fetch(stmt) == 0) rc = 1;

    mysql_stmt_close(stmt);
    return rc;
}

/* obter_id_pelo_nome: pesquisa pela coluna nome (apenas em itens ativos) usando prepared stmt */
int obter_id_pelo_nome(MYSQL *con, const char *tabela, const char *col_id, const char *col_nome, const char *nome_busca) {
    MYSQL_STMT *stmt = NULL;
    char query[512];
    MYSQL_BIND param[1];
    MYSQL_BIND result[1];
    int id_encontrado = -1;

    snprintf(query, sizeof(query), "SELECT %s FROM %s WHERE %s = ? AND ativo = 1 LIMIT 1", col_id, tabela, col_nome);

    stmt = mysql_stmt_init(con);
    if (!stmt) { erro_sql(con, "stmt_init"); return -1; }
    if (mysql_stmt_prepare(stmt, query, strlen(query))) { erro_sql(con, "stmt_prepare"); mysql_stmt_close(stmt); return -1; }

    memset(&param, 0, sizeof(param));
    param[0].buffer_type = MYSQL_TYPE_STRING;
    param[0].buffer = (char *)nome_busca;
    param[0].buffer_length = (unsigned long)strlen(nome_busca);

    if (mysql_stmt_bind_param(stmt, param)) { erro_sql(con, "stmt_bind_param"); mysql_stmt_close(stmt); return -1; }
    if (mysql_stmt_execute(stmt)) { erro_sql(con, "stmt_execute"); mysql_stmt_close(stmt); return -1; }

    memset(&result, 0, sizeof(result));
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &id_encontrado;
    result[0].buffer_length = sizeof(id_encontrado);

    if (mysql_stmt_bind_result(stmt, result)) { erro_sql(con, "stmt_bind_result"); mysql_stmt_close(stmt); return -1; }
    if (mysql_stmt_store_result(stmt)) { /* ok */ }

    if (mysql_stmt_fetch(stmt) != 0) id_encontrado = -1;

    mysql_stmt_close(stmt);
    return id_encontrado;
}

/* listarItensGenerico: lista ID e nome de qualquer tabela com coluna 'ativo' */
void listarItensGenerico(MYSQL *con, const char *tabela, const char *col_id, const char *col_nome, int estado_ativo) {
    char query[512];
    MYSQL_RES *res;
    MYSQL_ROW row;
    snprintf(query, sizeof(query), "SELECT %s, %s FROM %s WHERE ativo = %d ORDER BY %s", col_id, col_nome, tabela, estado_ativo, col_nome);

    res = executar_query_result(con, query);
    if (!res) {
        printf("Nenhum registro retornado ou erro.\n");
        return;
    }

    printf("\n--- %s (%s) ---\n", tabela, (estado_ativo==1 ? "Ativos" : "Inativos"));
    while ((row = mysql_fetch_row(res))) {
        printf("ID: %-5s | %s\n", row[0] ? row[0] : "NULL", row[1] ? row[1] : "NULL");
    }
    mysql_free_result(res);
}

/* alterarEstadoAtivo: reativar/desativar genérico */
void alterarEstadoAtivo(MYSQL *con, const char *tabela, const char *col_id, int id_para_alterar, int novo_estado) {
    char query[256];
    MYSQL_BIND param[2];
    snprintf(query, sizeof(query), "UPDATE %s SET ativo = ? WHERE %s = ?", tabela, col_id);

    memset(&param, 0, sizeof(param));
    param[0].buffer_type = MYSQL_TYPE_LONG;
    param[0].buffer = &novo_estado;
    param[0].buffer_length = sizeof(novo_estado);

    param[1].buffer_type = MYSQL_TYPE_LONG;
    param[1].buffer = &id_para_alterar;
    param[1].buffer_length = sizeof(id_para_alterar);

    if (!executar_stmt_simples(con, query, param, 2)) {
        printf("Falha ao alterar estado.\n");
        return;
    }

    if (novo_estado == 1)
        printf("Item (%s id=%d) reativado.\n", tabela, id_para_alterar);
    else
        printf("Item (%s id=%d) desativado.\n", tabela, id_para_alterar);
}

/* obterIdInterativo: permite que o usuário informe nome ou id; cria se não existir */
int obterIdInterativo(MYSQL *con, const char *nomeEntidade,
                      const char *tabela, const char *col_id, const char *col_nome) {
    char input[201];
    char opcao;
    int id = -1;
    limparTela();
    printf("--- Selecao de %s ---\n", nomeEntidade);
    listarItensGenerico(con, tabela, col_id, col_nome, 1);
    printf("Digite o NOME ou ID do %s (0 para cancelar): ", nomeEntidade);
    if (fgets(input, sizeof(input), stdin) == NULL) return -1;
    /* remove newline */
    input[strcspn(input, "\n")] = 0;
    if (strlen(input) == 0) return -1;
    /* se for numero puro */
    char *endptr;
    long val = strtol(input, &endptr, 10);
    if (*endptr == '\0') {
        if (val == 0) return -1;
        if (verificar_id_ativo(con, tabela, col_id, (int)val)) return (int)val;
        printf("ID nao existe ou inativo.\n");
        return -1;
    } else {
        /* trata como nome */
        id = obter_id_pelo_nome(con, tabela, col_id, col_nome, input);
        if (id != -1) return id;
        printf("%s '%s' nao encontrado. Deseja cadastrá-lo? (s/n): ", nomeEntidade, input);
        scanf(" %c", &opcao);
        limparBuffer();
        if (opcao == 's' || opcao == 'S') {
            long long new_id = criar_item_retornando_id(con, tabela, col_nome, input);
            if (new_id == -1) {
                printf("Falha ao criar %s.\n", nomeEntidade);
                return -1;
            }
            return (int)new_id;
        } else {
            return -1;
        }
    }
}

/* --------------------- Funções específicas de Entidades (exemplos) --------------------- */

/* Criar Vinho: coleta dados e insere registro em 'vinho' */
void criarVinho(MYSQL *con) {
    char nome[201];
    int safra;
    double preco;
    int quantidade;
    int id_tipo, id_pais;
    char query[512];
    MYSQL_BIND params[6];

    limparTela();
    printf("--- Cadastrar Novo Vinho ---\n");
    printf("Nome do vinho: ");
    if (fgets(nome, sizeof(nome), stdin) == NULL) return;
    nome[strcspn(nome, "\n")] = 0;
    if (strlen(nome) == 0) { printf("Nome vazio. Abortando.\n"); return; }

    printf("Safra (ex: 2020): ");
    if (scanf("%d", &safra) != 1) { limparBuffer(); printf("Safra invalida.\n"); return; }
    limparBuffer();

    printf("Preco (ex: 150.99): ");
    if (scanf("%lf", &preco) != 1) { limparBuffer(); printf("Preco invalido.\n"); return; }
    limparBuffer();

    printf("Quantidade em estoque: ");
    if (scanf("%d", &quantidade) != 1) { limparBuffer(); printf("Quantidade invalida.\n"); return; }
    limparBuffer();

    /* obter tipo e pais (interativo com possibilidade de criar) */
    id_tipo = obterIdInterativo(con, "Tipo", "tipo", "id_tipo", "nome_tipo");
    if (id_tipo == -1) { printf("Operacao abortada (tipo).\n"); return; }

    id_pais = obterIdInterativo(con, "Pais", "paisorigem", "id_paisorigem", "nome_pais");
    if (id_pais == -1) { printf("Operacao abortada (pais).\n"); return; }

    /* Inserir vinho */
    snprintf(query, sizeof(query),
             "INSERT INTO vinho (nome_vinho, safra, preco, quantidade, fk_tipo_id_tipo, fk_paisorigem_id_paisorigem, ativo) "
             "VALUES (?, ?, ?, ?, ?, ?, 1)");

    memset(&params, 0, sizeof(params));
    params[0].buffer_type = MYSQL_TYPE_STRING; params[0].buffer = nome; params[0].buffer_length = strlen(nome);
    params[1].buffer_type = MYSQL_TYPE_LONG; params[1].buffer = &safra;
    params[2].buffer_type = MYSQL_TYPE_DOUBLE; params[2].buffer = &preco;
    params[3].buffer_type = MYSQL_TYPE_LONG; params[3].buffer = &quantidade;
    params[4].buffer_type = MYSQL_TYPE_LONG; params[4].buffer = &id_tipo;
    params[5].buffer_type = MYSQL_TYPE_LONG; params[5].buffer = &id_pais;

    if (!executar_stmt_simples(con, query, params, 6)) {
        printf("Falha ao inserir vinho.\n");
        return;
    }
    printf("Vinho '%s' cadastrado com sucesso (ID: %lld).\n", nome, mysql_insert_id(con));
}

/* listarVinhos: SELECT com joins para apresentar info legível */
void listarVinhos(MYSQL *con) {
    const char *query =
        "SELECT v.id_vinho, v.nome_vinho, v.safra, v.preco, v.quantidade, t.nome_tipo, p.nome_pais "
        "FROM vinho v "
        "LEFT JOIN tipo t ON v.fk_tipo_id_tipo = t.id_tipo "
        "LEFT JOIN paisorigem p ON v.fk_paisorigem_id_paisorigem = p.id_paisorigem "
        "WHERE v.ativo = 1 ORDER BY v.nome_vinho;";

    MYSQL_RES *res = executar_query_result(con, query);
    if (!res) { printf("Nenhum vinho ou erro na consulta.\n"); return; }

    MYSQL_ROW row;
    printf("\nID  | Nome do Vinho                   | Safra | Preco    | Qtd | Tipo      | Pais\n");
    printf("----+---------------------------------+-------+----------+-----+-----------+----------\n");
    while ((row = mysql_fetch_row(res))) {
        printf("%-3s | %-31s | %-5s | R$ %-7s | %-3s | %-9s | %-8s\n",
               row[0] ? row[0] : "N/D",
               row[1] ? row[1] : "N/D",
               row[2] ? row[2] : "N/D",
               row[3] ? row[3] : "N/D",
               row[4] ? row[4] : "N/D",
               row[5] ? row[5] : "N/D",
               row[6] ? row[6] : "N/D");
    }
    mysql_free_result(res);
}

/* listarVinhosSimples: wrapper para uso em obterIdInterativo */
void listarVinhosSimples(MYSQL *con) {
    listarItensGenerico(con, "vinho", "id_vinho", "nome_vinho", 1);
}

/* listarDetalhesVinho: Exemplo de SELECT com GROUP_CONCAT para exibir uvas (se existir tabela associativa) */
/* Implementação simplificada: apenas exibe dados principais */
void listarDetalhesVinho(MYSQL *con) {
    int id = -1;
    id = obterIdInterativo(con, "Vinho", "vinho", "id_vinho", "nome_vinho");
    if (id == -1) { printf("Operacao abortada.\n"); return; }

    char query[512];
    snprintf(query, sizeof(query),
             "SELECT v.id_vinho, v.nome_vinho, v.safra, v.preco, v.quantidade, t.nome_tipo, p.nome_pais "
             "FROM vinho v LEFT JOIN tipo t ON v.fk_tipo_id_tipo = t.id_tipo LEFT JOIN paisorigem p ON v.fk_paisorigem_id_paisorigem = p.id_paisorigem "
             "WHERE v.id_vinho = %d LIMIT 1", id);

    MYSQL_RES *res = executar_query_result(con, query);
    if (!res) { printf("Erro na consulta.\n"); return; }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        printf("Vinho nao encontrado.\n");
    } else {
        printf("\nID: %s\nNome: %s\nSafra: %s\nPreco: %s\nQuantidade: %s\nTipo: %s\nPais: %s\n",
               row[0] ? row[0] : "N/D",
               row[1] ? row[1] : "N/D",
               row[2] ? row[2] : "N/D",
               row[3] ? row[3] : "N/D",
               row[4] ? row[4] : "N/D",
               row[5] ? row[5] : "N/D",
               row[6] ? row[6] : "N/D");
    }
    mysql_free_result(res);
}

/* atualizarVinho: exemplo básico que atualiza preço e quantidade */
void atualizarVinho(MYSQL *con) {
    int id = obterIdInterativo(con, "Vinho a atualizar", "vinho", "id_vinho", "nome_vinho");
    if (id == -1) { printf("Operacao cancelada.\n"); return; }
    double novo_preco;
    int nova_qtd;
    printf("Novo preco: ");
    if (scanf("%lf", &novo_preco) != 1) { limparBuffer(); printf("Preco invalido.\n"); return; }
    limparBuffer();
    printf("Nova quantidade: ");
    if (scanf("%d", &nova_qtd) != 1) { limparBuffer(); printf("Quantidade invalida.\n"); return; }
    limparBuffer();

    char query[256];
    MYSQL_BIND params[3];
    snprintf(query, sizeof(query), "UPDATE vinho SET preco = ?, quantidade = ? WHERE id_vinho = ?");

    memset(&params, 0, sizeof(params));
    params[0].buffer_type = MYSQL_TYPE_DOUBLE; params[0].buffer = &novo_preco;
    params[1].buffer_type = MYSQL_TYPE_LONG; params[1].buffer = &nova_qtd;
    params[2].buffer_type = MYSQL_TYPE_LONG; params[2].buffer = &id;

    if (!executar_stmt_simples(con, query, params, 3)) {
        printf("Falha ao atualizar vinho.\n");
        return;
    }
    printf("Vinho atualizado com sucesso.\n");
}

/* --------------------- Funções de Usuário: Login / Registro --------------------- */

/* registrar: cria usuário simples com nível opcional (assume nome_usuario e senha texto) */
void registrar(MYSQL *con) {
    char nome[101], email[101], senha[101];
    int id_nivel = -1;
    printf("--- Registro de Usuario ---\n");
    printf("Nome de usuario: ");
    if (fgets(nome, sizeof(nome), stdin) == NULL) return;
    nome[strcspn(nome, "\n")] = 0;
    if (strlen(nome) == 0) { printf("Nome vazio.\n"); return; }

    printf("Email: ");
    if (fgets(email, sizeof(email), stdin) == NULL) return;
    email[strcspn(email, "\n")] = 0;

    printf("Senha (texto): ");
    if (fgets(senha, sizeof(senha), stdin) == NULL) return;
    senha[strcspn(senha, "\n")] = 0;
    if (strlen(senha) == 0) { printf("Senha vazia.\n"); return; }

    /* opcional: escolher nível (padrão 1 se tiver) */
    printf("Deseja associar um nivel? (s/n): ");
    char opc; scanf(" %c", &opc); limparBuffer();
    if (opc == 's' || opc == 'S') {
        id_nivel = obterIdInterativo(con, "Nivel", "nivel", "id_nivel", "nome_nivel");
        if (id_nivel == -1) { printf("Nivel nao selecionado. Cadastro abortado.\n"); return; }
    }

    /* Inserir usuario */
    MYSQL_BIND params[4];
    char query[256];
    snprintf(query, sizeof(query), "INSERT INTO usuario (nome_usuario, email, senha, id_nivel, ativo) VALUES (?, ?, ?, ?, 1)");

    memset(&params, 0, sizeof(params));
    params[0].buffer_type = MYSQL_TYPE_STRING; params[0].buffer = nome; params[0].buffer_length = strlen(nome);
    params[1].buffer_type = MYSQL_TYPE_STRING; params[1].buffer = email; params[1].buffer_length = strlen(email);
    params[2].buffer_type = MYSQL_TYPE_STRING; params[2].buffer = senha; params[2].buffer_length = strlen(senha);
    params[3].buffer_type = MYSQL_TYPE_LONG;   params[3].buffer = &id_nivel;

    if (!executar_stmt_simples(con, query, params, 4)) {
        printf("Falha ao cadastrar usuario.\n");
        return;
    }
    printf("Usuario '%s' cadastrado com sucesso (ID: %lld).\n", nome, mysql_insert_id(con));
}

/* login: verifica usuario + senha em texto (simples) */
void login(MYSQL *con) {
    char usuario[101], senha[101];
    printf("--- Login ---\n");
    printf("Usuario (nome): ");
    if (fgets(usuario, sizeof(usuario), stdin) == NULL) return;
    usuario[strcspn(usuario, "\n")] = 0;
    printf("Senha: ");
    if (fgets(senha, sizeof(senha), stdin) == NULL) return;
    senha[strcspn(senha, "\n")] = 0;

    /* Prepared statement para buscar usuario */
    MYSQL_STMT *stmt = NULL;
    MYSQL_BIND param[2];
    MYSQL_BIND result[3];
    int id_usuario = 0;
    char nome_nivel[64] = "";
    int id_nivel = 0;

    const char *query = "SELECT u.id_usuario, n.nome_nivel FROM usuario u LEFT JOIN nivel n ON u.id_nivel = n.id_nivel WHERE u.nome_usuario = ? AND u.senha = ? AND u.ativo = 1 LIMIT 1";

    stmt = mysql_stmt_init(con);
    if (!stmt) { erro_sql(con, "stmt_init"); return; }
    if (mysql_stmt_prepare(stmt, query, strlen(query))) { erro_sql(con, "stmt_prepare"); mysql_stmt_close(stmt); return; }

    memset(&param, 0, sizeof(param));
    param[0].buffer_type = MYSQL_TYPE_STRING; param[0].buffer = usuario; param[0].buffer_length = strlen(usuario);
    param[1].buffer_type = MYSQL_TYPE_STRING; param[1].buffer = senha; param[1].buffer_length = strlen(senha);

    if (mysql_stmt_bind_param(stmt, param)) { erro_sql(con, "stmt_bind_param"); mysql_stmt_close(stmt); return; }
    if (mysql_stmt_execute(stmt)) { erro_sql(con, "stmt_execute"); mysql_stmt_close(stmt); return; }

    memset(&result, 0, sizeof(result));
    result[0].buffer_type = MYSQL_TYPE_LONG; result[0].buffer = &id_usuario;
    result[1].buffer_type = MYSQL_TYPE_STRING; result[1].buffer = nome_nivel; result[1].buffer_length = sizeof(nome_nivel);

    if (mysql_stmt_bind_result(stmt, result)) { erro_sql(con, "stmt_bind_result"); mysql_stmt_close(stmt); return; }

    if (mysql_stmt_store_result(stmt)) { /* ok */ }
    if (mysql_stmt_fetch(stmt) == 0) {
        g_id_usuario_logado = id_usuario;
        strncpy(g_nome_nivel_logado, nome_nivel, sizeof(g_nome_nivel_logado)-1);
        g_nome_nivel_logado[sizeof(g_nome_nivel_logado)-1] = '\0';
        printf("Login bem sucedido! ID: %d | Nivel: %s\n", g_id_usuario_logado, g_nome_nivel_logado[0] ? g_nome_nivel_logado : "Sem nivel");
    } else {
        printf("Usuario ou senha invalidos.\n");
    }

    mysql_stmt_close(stmt);
    pausarTela(NULL);
}

/* --------------------- Menus --------------------- */

void menuGerenciarVinhos(MYSQL *con) {
    int opc = -1;
    do {
        limparTela();
        printf("--- Gerenciar Vinhos ---\n");
        printf("1. Listar Vinhos\n");
        printf("2. Cadastrar Vinho\n");
        printf("3. Detalhes de um Vinho\n");
        printf("4. Atualizar Vinho (preco/qtd)\n");
        printf("0. Voltar\n");
        printf("Escolha: ");
        if (scanf("%d", &opc) != 1) { limparBuffer(); opc = -1; }
        limparBuffer();
        switch (opc) {
            case 1: listarVinhos(con); pausarTela(NULL); break;
            case 2: criarVinho(con); pausarTela(NULL); break;
            case 3: listarDetalhesVinho(con); pausarTela(NULL); break;
            case 4: atualizarVinho(con); pausarTela(NULL); break;
            case 0: break;
            default: printf("Opcao invalida.\n"); pausarTela(NULL);
        }
    } while (opc != 0);
}

void menuGerenciarTipos(MYSQL *con) {
    int opc = -1;
    do {
        limparTela();
        printf("--- Gerenciar Tipos ---\n");
        printf("1. Listar Tipos Ativos\n");
        printf("2. Cadastrar Tipo\n");
        printf("3. Desativar Tipo\n");
        printf("4. Reativar Tipo\n");
        printf("0. Voltar\n");
        printf("Escolha: ");
        if (scanf("%d", &opc) != 1) { limparBuffer(); opc = -1; }
        limparBuffer();
        switch (opc) {
            case 1: listarItensGenerico(con, "tipo", "id_tipo", "nome_tipo", 1); pausarTela(NULL); break;
            case 2: { char tmp[201]; printf("Nome do novo tipo: "); if (fgets(tmp, sizeof(tmp), stdin)) { tmp[strcspn(tmp, "\n")] = 0; if (strlen(tmp)) criar_item_retornando_id(con, "tipo", "nome_tipo", tmp); } pausarTela(NULL); } break;
            case 3: { int id = obterIdInterativo(con, "Tipo a desativar", "tipo", "id_tipo", "nome_tipo"); if (id!=-1) alterarEstadoAtivo(con,"tipo","id_tipo",id,0); pausarTela(NULL);} break;
            case 4: { int id = obterIdInterativo(con, "Tipo a reativar", "tipo", "id_tipo", "nome_tipo"); if (id!=-1) alterarEstadoAtivo(con,"tipo","id_tipo",id,1); pausarTela(NULL);} break;
            case 0: break;
            default: printf("Opcao invalida.\n"); pausarTela(NULL);
        }
    } while (opc != 0);
}

/* Menus simples para Pais, Uva e Usuarios usando as funções genericas */
void menuGerenciarPais(MYSQL *con) {
    int opc=-1;
    do {
        limparTela();
        printf("--- Gerenciar Paises ---\n");
        printf("1. Listar Paises Ativos\n2. Cadastrar Pais\n3. Desativar Pais\n4. Reativar Pais\n0. Voltar\nEscolha: ");
        if (scanf("%d", &opc) != 1) { limparBuffer(); opc=-1; }
        limparBuffer();
        switch(opc) {
            case 1: listarItensGenerico(con,"paisorigem","id_paisorigem","nome_pais",1); pausarTela(NULL); break;
            case 2: { char tmp[201]; printf("Nome do pais: "); if (fgets(tmp,sizeof(tmp),stdin)){ tmp[strcspn(tmp,"\n")]=0; if(strlen(tmp)) criar_item_retornando_id(con,"paisorigem","nome_pais",tmp);} pausarTela(NULL);} break;
            case 3: { int id = obterIdInterativo(con,"Pais a desativar","paisorigem","id_paisorigem","nome_pais"); if(id!=-1) alterarEstadoAtivo(con,"paisorigem","id_paisorigem",id,0); pausarTela(NULL);} break;
            case 4: { int id = obterIdInterativo(con,"Pais a reativar","paisorigem","id_paisorigem","nome_pais"); if(id!=-1) alterarEstadoAtivo(con,"paisorigem","id_paisorigem",id,1); pausarTela(NULL);} break;
            case 0: break;
            default: printf("Opcao invalida.\n"); pausarTela(NULL);
        }
    } while(opc!=0);
}

void menuGerenciarUvas(MYSQL *con) {
    int opc=-1;
    do {
        limparTela();
        printf("--- Gerenciar Uvas ---\n");
        printf("1. Listar Uvas Ativas\n2. Cadastrar Uva\n3. Desativar Uva\n4. Reativar Uva\n0. Voltar\nEscolha: ");
        if (scanf("%d", &opc) != 1) { limparBuffer(); opc=-1; }
        limparBuffer();
        switch(opc) {
            case 1: listarItensGenerico(con,"uva","id_uva","nome_uva",1); pausarTela(NULL); break;
            case 2: { char tmp[201]; printf("Nome da uva: "); if (fgets(tmp,sizeof(tmp),stdin)){ tmp[strcspn(tmp,"\n")]=0; if(strlen(tmp)) criar_item_retornando_id(con,"uva","nome_uva",tmp);} pausarTela(NULL);} break;
            case 3: { int id = obterIdInterativo(con,"Uva a desativar","uva","id_uva","nome_uva"); if(id!=-1) alterarEstadoAtivo(con,"uva","id_uva",id,0); pausarTela(NULL);} break;
            case 4: { int id = obterIdInterativo(con,"Uva a reativar","uva","id_uva","nome_uva"); if(id!=-1) alterarEstadoAtivo(con,"uva","id_uva",id,1); pausarTela(NULL);} break;
            case 0: break;
            default: printf("Opcao invalida.\n"); pausarTela(NULL);
        }
    } while(opc!=0);
}

void menuGerenciarUsuarios(MYSQL *con) {
    int opc=-1;
    do {
        limparTela();
        printf("--- Gerenciar Usuarios ---\n");
        printf("1. Listar Usuarios Ativos\n2. Cadastrar Usuario\n3. Desativar Usuario\n4. Reativar Usuario\n0. Voltar\nEscolha: ");
        if (scanf("%d", &opc) != 1) { limparBuffer(); opc=-1; }
        limparBuffer();
        switch(opc) {
            case 1: listarItensGenerico(con,"usuario","id_usuario","nome_usuario",1); pausarTela(NULL); break;
            case 2: registrar(con); break;
            case 3: { int id = obterIdInterativo(con,"Usuario a desativar","usuario","id_usuario","nome_usuario"); if(id!=-1) alterarEstadoAtivo(con,"usuario","id_usuario",id,0); pausarTela(NULL);} break;
            case 4: { int id = obterIdInterativo(con,"Usuario a reativar","usuario","id_usuario","nome_usuario"); if(id!=-1) alterarEstadoAtivo(con,"usuario","id_usuario",id,1); pausarTela(NULL);} break;
            case 0: break;
            default: printf("Opcao invalida.\n"); pausarTela(NULL);
        }
    } while(opc!=0);
}

/* Menu principal */
void menuPrincipal(MYSQL *con) {
    int opc = -1;
    do {
        limparTela();
        printf("=== VINTAGEM - Menu Principal ===\n");
        printf("Usuario logado: %d | Nivel: %s\n", g_id_usuario_logado, g_nome_nivel_logado[0] ? g_nome_nivel_logado : "N/A");
        printf("1. Gerenciar Vinhos\n2. Movimentacao Estoque (nao implementado)\n");
        printf("3. Gerenciar Tipos\n4. Gerenciar Paises\n5. Gerenciar Uvas\n6. Gerenciar Usuarios\n");
        printf("0. Sair\nEscolha: ");
        if (scanf("%d", &opc) != 1) { limparBuffer(); opc = -1; }
        limparBuffer();
        switch(opc) {
            case 1: menuGerenciarVinhos(con); break;
            case 2: printf("Funcao de movimentacao de estoque nao implementada nesta versao.\n"); pausarTela(NULL); break;
            case 3: menuGerenciarTipos(con); break;
            case 4: menuGerenciarPais(con); break;
            case 5: menuGerenciarUvas(con); break;
            case 6: menuGerenciarUsuarios(con); break;
            case 0: printf("Saindo...\n"); break;
            default: printf("Opcao invalida.\n"); pausarTela(NULL);
        }
    } while(opc != 0);
}

/* --------------------- MAIN --------------------- */
int main(void) {
    /* Inicializa lib mysql (opcional mas recomendado) */
    if (mysql_library_init(0, NULL, NULL)) {
        fprintf(stderr, "Falha em mysql_library_init\n");
        return 1;
    }

    MYSQL *con = conectar_db();
    if (!con) {
        fprintf(stderr, "Nao foi possivel conectar ao banco.\n");
        mysql_library_end();
        return 1;
    }

    printf("Conectado ao banco '%s' com sucesso.\n", DB_NAME);
    pausarTela(NULL);

    /* Login / Registro inicial */
    int opc = -1;
    do {
        limparTela();
        printf("=== VINTAGEM - Bem vindo ===\n");
        printf("1. Login\n2. Registrar\n0. Sair\nEscolha: ");
        if (scanf("%d", &opc) != 1) { limparBuffer(); opc = -1; }
        limparBuffer();
        switch(opc) {
            case 1: login(con); break;
            case 2: registrar(con); break;
            case 0: printf("Encerrando...\n"); break;
            default: printf("Opcao invalida.\n"); pausarTela(NULL);
        }
    } while(opc != 0 && g_id_usuario_logado == 0);

    if (g_id_usuario_logado == 0) {
        printf("Nenhum usuario logado. Encerrando.\n");
        mysql_close(con);
        mysql_library_end();
        return 0;
    }

    /* Abre o menu principal */
    menuPrincipal(con);

    /* Fecha tudo no fim */
    mysql_close(con);
    mysql_library_end();
    return 0;
}
