#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h> //conexao com o MYSQL
#include <ctype.h> // Para toupper()

// --- MUDANÇA: 'conio.h' é específico do Windows. Vamos usar #ifdef ---
#ifdef _WIN32
    #include <conio.h> // Para _getch()
#else
    // Em Linux/Mac, precisamos de uma alternativa simples para _getch()
    #include <termios.h>
    #include <unistd.h>
    int _getch() {
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


// --- Informações de Conexão ---
// NOTA DE SEGURANÇA: Senha no código é uma má prática. 
// O ideal é ler de um arquivo de configuração.
const char *server = "localhost";
const char *user = "root";
const char *password = "@vintagem123";
const char *database = "vintagem";

// Globais (mantidas para simplicidade do login)
int g_id_usuario_logado = 0;
char g_nome_nivel_logado[51] = "";

//Definindo a estrutura para um Vinho
typedef struct {
    int id_vinho; //usado apenas para armazenar dados lidos
    char nome_vinho[151];
    int safra;
    double preco;
    int quantidade;
    int fk_tipo_id_tipo;
    int fk_paisorigem_id_paisorigem;
} Vinho;

// --- MUDANÇA: PROTÓTIPOS ATUALIZADOS E NOVOS ---
// Todas as funções que usam o DB agora recebem 'MYSQL *con'

// Funções de Utilidade
void finish_with_error(MYSQL *con);
MYSQL *conectar_db();
void limparBuffer(void);
void pausarTela(void);
void limparTela(void);

// Funções Auxiliares de Lógica
int obter_id_pelo_nome(MYSQL *con, const char *tabela, const char *col_id, const char *col_nome, const char *nome_busca);
int verificar_id_ativo(MYSQL *con, const char *tabela, const char *col_id, int id_busca);
long long criar_item_retornando_id(MYSQL *con, const char *tabela, const char *col_nome, const char *nome_valor);

// --- MUDANÇA: Nova função centralizada para buscar IDs ---
int obterIdInterativo(MYSQL *con, const char *nomeEntidade, 
                      const char *tabela, const char *col_id, const char *col_nome,
                      void (*funcaoListar)(MYSQL *con));

// --- MUDANÇA: Novas funções genéricas ---
void alterarEstadoAtivo(MYSQL *con, const char *tabela, const char *col_id, int id_para_alterar, int novo_estado);
void listarItensGenerico(MYSQL *con, const char *tabela, const char *col_id, const char *col_nome, int estado_ativo);


// Prototipos dos Menus
void menuPrincipal(MYSQL *con);
void menuGerenciarVinhos(MYSQL *con);
void menuGerenciarTipos(MYSQL *con);
void menuGerenciarPais(MYSQL *con);
void menuGerenciarUvas(MYSQL *con);
void menuGerenciarUsuarios(MYSQL *con);
void menuMovimentacaoEstoque(MYSQL *con);

// Prototipos das funcoes do CRUD
void criarVinho(MYSQL *con);
void listarVinhos(MYSQL *con);
void listarVinhosSimples(MYSQL *con);
void listarDetalhesVinho(MYSQL *con);
void atualizarVinho(MYSQL *con);

// Wrappers de Listagem (para a função obterIdInterativo)
void listarTiposAtivos(MYSQL *con);
void listarTiposInativos(MYSQL *con);
void listarPaisesAtivos(MYSQL *con);
void listarPaisesInativos(MYSQL *con);
void listarUvasAtivas(MYSQL *con);
void listarUvasInativas(MYSQL *con);
void listarUsuariosAtivos(MYSQL *con);
void listarUsuariosInativos(MYSQL *con);
void listarNiveisAtivos(MYSQL *con);

// Funções de Lógica (Login, etc. - não fornecidas no original, exceto login)
void login(MYSQL *con);
void registrar(MYSQL *con);
// ... (outros protótipos do seu código original)


// --- IMPLEMENTAÇÃO DAS FUNÇÕES ---

void finish_with_error(MYSQL *con) {
    fprintf(stderr, "%s\n", mysql_error(con));
    if (con) { // Só fecha se a conexão foi inicializada
        mysql_close(con);
    }
    mysql_library_end(); // Libera recursos da biblioteca
    pausarTela();
    exit(1);
}

// --- MUDANÇA: Nova função de limpar buffer ---
void limparBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// --- MUDANÇA: Função de pausar cross-platform ---
void pausarTela(void) {
    printf("\nPressione qualquer tecla para continuar...");
    _getch(); // _getch() agora é cross-platform (definido no topo)
}

// --- MUDANÇA: Função de limpar tela cross-platform ---
void limparTela(void) {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

MYSQL *conectar_db() {
    MYSQL *con = mysql_init(NULL);
    if (con == NULL) {
        fprintf(stderr, "mysql_init() falhou\n");
        return NULL;
    }

    if (mysql_real_connect(con, server, user, password, database, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() falhou: %s\n", mysql_error(con));
        mysql_close(con);
        return NULL;
    }

    // Habilita UTF-8
    if (mysql_set_character_set(con, "utf8")) {
        fprintf(stderr, "Erro ao definir charset: %s\n", mysql_error(con));
    }
    
    return con;
}


// --- MUDANÇA: Refatorada para receber 'con' ---
int obter_id_pelo_nome(MYSQL *con, const char *tabela, const char *col_id, const char *col_nome, const char *nome_busca) {
    MYSQL_STMT *stmt;
    char query[512];
    int id_encontrado = -1;
    MYSQL_BIND param[1];
    MYSQL_BIND result[1];
    int res_id;
    unsigned long length;

    // Removemos conectar_db() e mysql_close()
    
    sprintf(query, "SELECT %s FROM %s WHERE %s = ? AND ativo = 1", col_id, tabela, col_nome);
    
    stmt = mysql_stmt_init(con);
    if (!stmt) {
        fprintf(stderr, " (obter_id) mysql_stmt_init() falhou\n");
        return -1;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, " (obter_id) mysql_stmt_prepare() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    memset(param, 0, sizeof(param));
    param[0].buffer_type = MYSQL_TYPE_STRING;
    param[0].buffer = (char *)nome_busca;
    param[0].buffer_length = strlen(nome_busca);

    if (mysql_stmt_bind_param(stmt, param)) {
        fprintf(stderr, " (obter_id) mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " (obter_id) mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    // Bind do resultado
    memset(result, 0, sizeof(result));
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &res_id;
    result[0].length = &length;

    if (mysql_stmt_bind_result(stmt, result)) {
        fprintf(stderr, " (obter_id) mysql_stmt_bind_result() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }
    
    if (mysql_stmt_store_result(stmt)) {
        fprintf(stderr, " (obter_id) mysql_stmt_store_result() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    // Se achou (fetch == 0)
    if (mysql_stmt_fetch(stmt) == 0) {
        id_encontrado = res_id;
    }

    mysql_stmt_close(stmt);
    return id_encontrado;
}


// --- MUDANÇA: Refatorada para receber 'con' ---
int verificar_id_ativo(MYSQL *con, const char *tabela, const char *col_id, int id_busca) {
    MYSQL_STMT *stmt;
    char query[512];
    int id_existe = 0; // 0 = não existe, 1 = existe
    MYSQL_BIND param[1];
    
    // Removemos conectar_db() e mysql_close()

    sprintf(query, "SELECT 1 FROM %s WHERE %s = ? AND ativo = 1 LIMIT 1", tabela, col_id);
    
    stmt = mysql_stmt_init(con);
    if (!stmt) {
        fprintf(stderr, " (verificar_id) mysql_stmt_init() falhou\n");
        return 0;
    }
    
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, " (verificar_id) mysql_stmt_prepare() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    memset(param, 0, sizeof(param));
    param[0].buffer_type = MYSQL_TYPE_LONG;
    param[0].buffer = &id_busca;
    param[0].buffer_length = sizeof(id_busca);

    if (mysql_stmt_bind_param(stmt, param)) {
        fprintf(stderr, " (verificar_id) mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " (verificar_id) mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }
    
    if (mysql_stmt_store_result(stmt)) {
         fprintf(stderr, " (verificar_id) mysql_stmt_store_result() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    // Se encontrou (fetch == 0)
    if (mysql_stmt_fetch(stmt) == 0) {
        id_existe = 1;
    }

    mysql_stmt_close(stmt);
    return id_existe;
}

// --- MUDANÇA: Refatorada para receber 'con' ---
long long criar_item_retornando_id(MYSQL *con, const char *tabela, const char *col_nome, const char *nome_valor) {
    MYSQL_STMT *stmt;
    char query[512];
    long long novo_id = -1;
    MYSQL_BIND param[1];

    // Removemos conectar_db() e mysql_close()

    sprintf(query, "INSERT INTO %s (%s) VALUES (?)", tabela, col_nome);
    
    stmt = mysql_stmt_init(con);
    if (!stmt) {
        fprintf(stderr, " (criar_item) mysql_stmt_init() falhou\n");
        return -1;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, " (criar_item) mysql_stmt_prepare() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    memset(param, 0, sizeof(param));
    param[0].buffer_type = MYSQL_TYPE_STRING;
    param[0].buffer = (char *)nome_valor;
    param[0].buffer_length = strlen(nome_valor);

    if (mysql_stmt_bind_param(stmt, param)) {
        fprintf(stderr, " (criar_item) mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " (criar_item) mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    // Se teve sucesso, pega o ID
    novo_id = mysql_stmt_insert_id(stmt);
    printf("Novo item '%s' criado com ID: %lld\n", nome_valor, novo_id);

    mysql_stmt_close(stmt);
    return novo_id;
}


// --- MUDANÇA: NOVA FUNÇÃO CENTRALIZADA ---
/**
 * @brief Solicita interativamente ao usuário um ID, permitindo entrada por nome ou ID.
 * Valida a entrada, verifica se existe, e oferece criação se não for encontrado.
 * * @param con Ponteiro da conexão MySQL.
 * @param nomeEntidade Nome amigável da entidade (ex: "Tipo", "País").
 * @param tabela Nome da tabela no banco (ex: "tipo", "paisorigem").
 * @param col_id Nome da coluna de ID (ex: "id_tipo").
 * @param col_nome Nome da coluna de nome (ex: "nome_tipo").
 * @param funcaoListar Ponteiro para a função que lista os itens (ex: listarTiposAtivos).
 * @return O ID válido encontrado ou -1 se o usuário cancelar ou a operação falhar.
 */
int obterIdInterativo(MYSQL *con, const char *nomeEntidade, 
                      const char *tabela, const char *col_id, const char *col_nome,
                      void (*funcaoListar)(MYSQL *con)) 
{
    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;
    int id_encontrado = -1;
    char resposta;

    limparTela();
    printf("\n--- Selecao de %s ---\n", nomeEntidade);
    
    // 1. Lista os itens disponíveis
    if (funcaoListar != NULL) {
        funcaoListar(con);
    }
    
    printf("Digite o NOME ou o ID do %s (ou 0 para cancelar): ", nomeEntidade);
    
    // Leitura segura da linha
    if (scanf("%100[^\n]", input_buffer) != 1) {
        // Se a entrada estiver vazia (só pressionou Enter), limpa e retorna falha
        limparBuffer();
        if (strlen(input_buffer) == 0) {
            printf("Entrada vazia. Operacao cancelada.\n");
            return -1;
        }
    }
    limparBuffer();
    
    // 2. Tenta converter para número
    id_digitado = strtol(input_buffer, &end_ptr, 10);
    
    if (id_digitado == 0 && end_ptr != input_buffer) { 
        // Usuário digitou "0"
        printf("Operacao cancelada.\n");
        return -1;
    }

    // 3. Verifica o tipo de entrada (Nome ou ID)
    if (end_ptr == input_buffer) { 
        // É um NOME (nenhum dígito foi lido)
        id_encontrado = obter_id_pelo_nome(con, tabela, col_id, col_nome, input_buffer);
        
        if (id_encontrado == -1) {
            printf("%s '%s' nao encontrado.\n", nomeEntidade, input_buffer);
            printf("Deseja cadastra-lo agora? (s/n): ");
            scanf(" %c", &resposta);
            limparBuffer();
            
            if (resposta == 's' || resposta == 'S') {
                id_encontrado = (int)criar_item_retornando_id(con, tabela, col_nome, input_buffer);
                if (id_encontrado == -1) {
                    printf("Nao foi possivel criar o novo %s.\n", nomeEntidade);
                }
            }
        }
    } else if (*end_ptr == '\0') { 
        // É um ID (string inteira foi convertida para número)
        if (verificar_id_ativo(con, tabela, col_id, (int)id_digitado)) {
            id_encontrado = (int)id_digitado;
        } else {
            printf("\nErro: O ID de %s '%ld' nao existe ou esta inativo.\n", nomeEntidade, id_digitado);
            id_encontrado = -1;
        }
    } else { 
        // Misto (inválido, ex: "123abc")
        printf("\nErro: Entrada '%s' invalida. Nao e nem um nome nem um ID valido.\n", input_buffer);
        id_encontrado = -1;
    }

    return id_encontrado; // Retorna o ID ou -1 (falha/cancelou)
}


// --- MUDANÇA: NOVA FUNÇÃO GENÉRICA (ATIVAR/DESATIVAR) ---
/**
 * @brief Altera o estado 'ativo' (0 ou 1) de um item em qualquer tabela.
 */
void alterarEstadoAtivo(MYSQL *con, const char *tabela, const char *col_id, int id_para_alterar, int novo_estado) {
    MYSQL_STMT *stmt;
    char query[256];
    MYSQL_BIND param[2];

    sprintf(query, "UPDATE %s SET ativo = ? WHERE %s = ?", tabela, col_id);
    
    stmt = mysql_stmt_init(con);
    if (!stmt) {
        fprintf(stderr, " (alterarEstado) mysql_stmt_init() falhou\n");
        return;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, " (alterarEstado) mysql_stmt_prepare() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    memset(param, 0, sizeof(param));
    param[0].buffer_type = MYSQL_TYPE_LONG;
    param[0].buffer = &novo_estado;
    param[0].buffer_length = sizeof(novo_estado);
    
    param[1].buffer_type = MYSQL_TYPE_LONG;
    param[1].buffer = &id_para_alterar;
    param[1].buffer_length = sizeof(id_para_alterar);

    if (mysql_stmt_bind_param(stmt, param)) {
        fprintf(stderr, " (alterarEstado) mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " (alterarEstado) mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    if (mysql_stmt_affected_rows(stmt) > 0) {
        printf("\nSUCESSO: %s ID %d foi %s.\n", 
            tabela, id_para_alterar, (novo_estado == 1 ? "REATIVADO" : "DESATIVADO"));
    } else {
        printf("\nAVISO: Nenhum item alterado. O ID %d pode nao existir ou ja estar nesse estado.\n", id_para_alterar);
    }

    mysql_stmt_close(stmt);
}


// --- MUDANÇA: NOVA FUNÇÃO GENÉRICA (LISTAR) ---
/**
 * @brief Lista ID e Nome de qualquer tabela que tenha colunas 'ativo', 'col_id' e 'col_nome'.
 */
void listarItensGenerico(MYSQL *con, const char *tabela, const char *col_id, const char *col_nome, int estado_ativo) {
    char query[512];
    
    sprintf(query, "SELECT %s, %s FROM %s WHERE ativo = %d ORDER BY %s", 
            col_id, col_nome, tabela, estado_ativo, col_nome);
            
    printf("\n--- Lista de %s %s ---\n", tabela, (estado_ativo == 1 ? "Ativos" : "Inativos"));

    if (mysql_query(con, query)) {
        finish_with_error(con);
        return;
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        finish_with_error(con);
        return;
    }

    MYSQL_ROW row;
    int num_fields = mysql_num_fields(result);
    int count = 0;

    while ((row = mysql_fetch_row(result))) {
        count++;
        // Assumindo ID no campo 0 e NOME no campo 1
        printf("ID: %-5s | Nome: %s\n", row[0] ? row[0] : "NULL", row[1] ? row[1] : "NULL");
    }
    
    if (count == 0) {
        printf("Nenhum item encontrado.\n");
    }
    printf("-----------------------------------------\n");

    mysql_free_result(result);
}


// --- Wrappers de Listagem (para passar para a função obterIdInterativo) ---
void listarTiposAtivos(MYSQL *con) {
    listarItensGenerico(con, "tipo", "id_tipo", "nome_tipo", 1);
}
void listarTiposInativos(MYSQL *con) {
    listarItensGenerico(con, "tipo", "id_tipo", "nome_tipo", 0);
}
void listarPaisesAtivos(MYSQL *con) {
    listarItensGenerico(con, "paisorigem", "id_paisorigem", "nome_pais", 1);
}
void listarPaisesInativos(MYSQL *con) {
    listarItensGenerico(con, "paisorigem", "id_paisorigem", "nome_pais", 0);
}
void listarUvasAtivas(MYSQL *con) {
    listarItensGenerico(con, "uva", "id_uva", "nome_uva", 1);
}
void listarUvasInativas(MYSQL *con) {
    listarItensGenerico(con, "uva", "id_uva", "nome_uva", 0);
}
void listarUsuariosAtivos(MYSQL *con) {
    listarItensGenerico(con, "usuario", "id_usuario", "nome_usuario", 1);
}
void listarUsuariosInativos(MYSQL *con) {
    listarItensGenerico(con, "usuario", "id_usuario", "nome_usuario", 0);
}
void listarNiveisAtivos(MYSQL *con) {
    listarItensGenerico(con, "nivel", "id_nivel", "nome_nivel", 1);
}
// (Não precisamos de listarVinhosSimples como wrapper, 
// pois ela já tem a assinatura correta 'void (MYSQL *con)')


// --- FUNÇÕES DE LÓGICA (EXEMPLOS REFAKORADOS) ---

// --- MUDANÇA: Refatorada para receber 'con' e usar obterIdInterativo ---
void criarVinho(MYSQL *con) {
    Vinho v; // Struct para armazenar os dados
    MYSQL_STMT *stmt;
    MYSQL_BIND param[6];
    char query[512];

    // 1. Coletar dados simples
    limparTela();
    printf("--- Cadastro de Novo Vinho ---\n");
    printf("Digite o nome do vinho: ");
    scanf("%150[^\n]", v.nome_vinho);
    limparBuffer();
    
    printf("Digite a safra (ex: 2020): ");
    scanf("%d", &v.safra);
    limparBuffer();
    
    printf("Digite o preco (ex: 150.99): ");
    scanf("%lf", &v.preco); // Use %lf para double
    limparBuffer();
    
    printf("Digite a quantidade em estoque: ");
    scanf("%d", &v.quantidade);
    limparBuffer();

    // 2. Coletar Tipo (usando a nova função auxiliar)
    v.fk_tipo_id_tipo = obterIdInterativo(con, "Tipo", "tipo", "id_tipo", "nome_tipo", listarTiposAtivos);
    if (v.fk_tipo_id_tipo == -1) {
        printf("Criacao de vinho abortada (Tipo invalido ou cancelado).\n");
        return; 
    }

    // 3. Coletar País (usando a nova função auxiliar)
    v.fk_paisorigem_id_paisorigem = obterIdInterativo(con, "Pais", "paisorigem", "id_paisorigem", "nome_pais", listarPaisesAtivos);
    if (v.fk_paisorigem_id_paisorigem == -1) {
        printf("Criacao de vinho abortada (Pais invalido ou cancelado).\n");
        return;
    }
    
    // (Opcional: Adicionar lógica similar para UVAS aqui, 
    //  que seria mais complexa por ser N:M e exigir um loop)

    // 4. Preparar e Executar o INSERT
    sprintf(query, "INSERT INTO vinho (nome_vinho, safra, preco, quantidade, fk_tipo_id_tipo, fk_paisorigem_id_paisorigem, ativo) "
                   "VALUES (?, ?, ?, ?, ?, ?, 1)");
                   
    stmt = mysql_stmt_init(con);
    if (!stmt) {
        fprintf(stderr, " (criarVinho) mysql_stmt_init() falhou\n");
        return;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, " (criarVinho) mysql_stmt_prepare() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    memset(param, 0, sizeof(param));
    
    param[0].buffer_type = MYSQL_TYPE_STRING;
    param[0].buffer = v.nome_vinho;
    param[0].buffer_length = strlen(v.nome_vinho);
    
    param[1].buffer_type = MYSQL_TYPE_LONG;
    param[1].buffer = &v.safra;
    
    param[2].buffer_type = MYSQL_TYPE_DOUBLE;
    param[2].buffer = &v.preco;

    param[3].buffer_type = MYSQL_TYPE_LONG;
    param[3].buffer = &v.quantidade;
    
    param[4].buffer_type = MYSQL_TYPE_LONG;
    param[4].buffer = &v.fk_tipo_id_tipo;
    
    param[5].buffer_type = MYSQL_TYPE_LONG;
    param[5].buffer = &v.fk_paisorigem_id_paisorigem;

    if (mysql_stmt_bind_param(stmt, param)) {
        fprintf(stderr, " (criarVinho) mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " (criarVinho) mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    printf("\nSUCESSO: Vinho '%s' (ID: %lld) cadastrado.\n", v.nome_vinho, mysql_stmt_insert_id(stmt));
    mysql_stmt_close(stmt);
}

// --- MUDANÇA: Refatorada para receber 'con' ---
void listarVinhos(MYSQL *con) {
    // Esta função usa mysql_query, que é mais simples para SELECTs complexos
    const char *query = "SELECT v.id_vinho, v.nome_vinho, v.safra, v.preco, v.quantidade, t.nome_tipo, p.nome_pais "
                        "FROM vinho v "
                        "LEFT JOIN tipo t ON v.fk_tipo_id_tipo = t.id_tipo "
                        "LEFT JOIN paisorigem p ON v.fk_paisorigem_id_paisorigem = p.id_paisorigem "
                        "WHERE v.ativo = 1 "
                        "ORDER BY v.nome_vinho;";
    
    limparTela();
    printf("\n================================== LISTA DE VINHOS ATIVOS ==================================\n");
    
    if (mysql_query(con, query)) {
        finish_with_error(con);
        return;
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        finish_with_error(con);
        return;
    }

    MYSQL_ROW row;
    int count = 0;

    printf("ID   | Nome Vinho                      | Safra | Preco      | Qtd. | Tipo       | Pais      \n");
    printf("-----+---------------------------------+-------+------------+------+------------+-----------\n");

    while ((row = mysql_fetch_row(result))) {
        count++;
        printf("%-4s | %-31s | %-5s | R$ %-8s | %-4s | %-10s | %-10s\n",
               row[0] ? row[0] : "N/D",  // ID
               row[1] ? row[1] : "N/D",  // Nome
               row[2] ? row[2] : "N/D",  // Safra
               row[3] ? row[3] : "N/D",  // Preco
               row[4] ? row[4] : "N/D",  // Qtd
               row[5] ? row[5] : "N/D",  // Tipo
               row[6] ? row[6] : "N/D"); // Pais
    }
    
    if (count == 0) {
        printf("Nenhum vinho ativo encontrado.\n");
    }
    printf("==============================================================================================\n");

    mysql_free_result(result);
}

// --- MUDANÇA: Refatorada para receber 'con' ---
// (Esta é a função 'listarVinhos' original, renomeada para 'Simples' 
//  e usada como wrapper para obterIdInterativo)
void listarVinhosSimples(MYSQL *con) {
    listarItensGenerico(con, "vinho", "id_vinho", "nome_vinho", 1);
}

// (Função listarDetalhesVinho - permanece a mesma, mas recebe 'con')
void listarDetalhesVinho(MYSQL *con) {
    int id_vinho = obterIdInterativo(con, "Vinho", "vinho", "id_vinho", "nome_vinho", listarVinhosSimples);
    if (id_vinho == -1) {
        printf("Operacao abortada.\n");
        return;
    }

    // ... (Sua lógica original de SELECT com GROUP_CONCAT e prepared statements)
    // Lembre-se de remover conectar_db() e mysql_close() dela
    // e adicionar checagem de erro em stmt_prepare, etc.
    printf("Logica de listarDetalhesVinho (ID: %d) nao implementada nesta refatoracao.\n", id_vinho);
}


// --- MUDANÇA: Exemplo de menu refatorado ---
// Recebe 'con' e usa as novas funções genéricas
void menuGerenciarTipos(MYSQL *con) {
    int opcao = -1;
    int id_busca = -1;

    do {
        limparTela();
        printf("--- Gerenciar Tipos de Vinho ---\n");
        printf("1. Listar Tipos Ativos\n");
        printf("2. Cadastrar Novo Tipo\n");
        printf("3. Atualizar Nome de Tipo\n");
        printf("4. Desativar Tipo\n");
        printf("5. Listar Tipos Inativos\n");
        printf("6. Reativar Tipo\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("----------------------------------\n");
        printf("Escolha uma opcao: ");
        
        scanf("%d", &opcao);
        limparBuffer();

        switch (opcao) {
            case 1:
                listarTiposAtivos(con);
                pausarTela();
                break;
            case 2:
                // A função 'obterIdInterativo' já lida com a criação
                // se o nome digitado não existir, então podemos
                // simplesmente chamar a função de busca e deixar o usuário
                // digitar o nome do novo tipo.
                printf("Digite o nome do NOVO Tipo (ou 0 para cancelar):\n");
                obterIdInterativo(con, "Tipo", "tipo", "id_tipo", "nome_tipo", NULL); // Sem listar
                pausarTela();
                break;
            case 3:
                // Atualizar é mais complexo e mantém a lógica original
                // Lembre-se de refatorar 'atualizarTipo' para receber 'con'
                // atualizarTipo(con); 
                printf("Funcao 'Atualizar' nao implementada nesta refatoracao.\n");
                pausarTela();
                break;
            case 4: // Desativar
                id_busca = obterIdInterativo(con, "Tipo a DESATIVAR", "tipo", "id_tipo", "nome_tipo", listarTiposAtivos);
                if (id_busca != -1) {
                    alterarEstadoAtivo(con, "tipo", "id_tipo", id_busca, 0); // 0 = Desativar
                }
                pausarTela();
                break;
            case 5: // Listar Inativos
                listarTiposInativos(con);
                pausarTela();
                break;
            case 6: // Reativar
                id_busca = obterIdInterativo(con, "Tipo a REATIVAR", "tipo", "id_tipo", "nome_tipo", listarTiposInativos);
                if (id_busca != -1) {
                    alterarEstadoAtivo(con, "tipo", "id_tipo", id_busca, 1); // 1 = Ativar
                }
                pausarTela();
                break;
            case 0:
                break;
            default:
                printf("Opcao invalida!\n");
                pausarTela();
        }

    } while (opcao != 0);
}

// (Implemente os outros menus: menuGerenciarPais, menuGerenciarUvas, etc.
//  seguindo o MESMO PADRÃO do menuGerenciarTipos)
void menuGerenciarPais(MYSQL *con) { printf("Nao implementado.\n"); pausarTela(); }
void menuGerenciarUvas(MYSQL *con) { printf("Nao implementado.\n"); pausarTela(); }
void menuGerenciarUsuarios(MYSQL *con) { printf("Nao implementado.\n"); pausarTela(); }
void menuMovimentacaoEstoque(MYSQL *con) { printf("Nao implementado.\n"); pausarTela(); }


void menuPrincipal(MYSQL *con) {
    int opcao = -1;
    do {
        limparTela();
        printf("--- VINTAGEM - Menu Principal ---\n");
        printf("Usuario: %d | Nivel: %s\n", g_id_usuario_logado, g_nome_nivel_logado);
        printf("----------------------------------\n");
        printf("1. Gerenciar Vinhos\n");
        printf("2. Gerenciar Estoque (Movimentacao)\n");
        printf("----------------------------------\n");
        printf("--- Administracao ---\n");
        printf("5. Gerenciar Tipos\n");
        printf("6. Gerenciar Paises de Origem\n");
        printf("7. Gerenciar Uvas\n");
        printf("8. Gerenciar Usuarios\n");
        printf("----------------------------------\n");
        printf("0. Sair\n");
        printf("Escolha uma opcao: ");

        scanf("%d", &opcao);
        limparBuffer();

        switch (opcao) {
            case 1:
                menuGerenciarVinhos(con);
                break;
            case 2:
                menuMovimentacaoEstoque(con);
                break;
            case 5:
                menuGerenciarTipos(con);
                break;
            case 6:
                menuGerenciarPais(con);
                break;
            case 7:
                menuGerenciarUvas(con);
                break;
            case 8:
                menuGerenciarUsuarios(con);
                break;
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opcao invalida!\n");
                pausarTela();
        }
    } while (opcao != 0);
}

// (Suas funções de login e registro - não fornecidas no original, 
//  mas devem ser adaptadas para receber 'con')
void login(MYSQL *con) {
    // ... Sua lógica de login ...
    // Exemplo de como você definiria as globais
    g_id_usuario_logado = 1; // ID fixo para teste
    strcpy(g_nome_nivel_logado, "Admin"); // Nível fixo para teste
    printf("Login (teste) bem-sucedido!\n");
    pausarTela();
}

int main() {
    // Inicializa a biblioteca MySQL
    if (mysql_library_init(0, NULL, NULL)) {
        fprintf(stderr, "mysql_library_init() falhou\n");
        exit(1);
    }

    // --- MUDANÇA: Conexão única ---
    MYSQL *con = conectar_db();
    if (con == NULL) {
        // conectar_db() já imprimiu o erro
        pausarTela();
        mysql_library_end();
        return 1;
    }
    printf("Conexao com o banco '%s' estabelecida com sucesso!\n", database);
    pausarTela();


    // --- 1. LOGIN ---
    // (O login precisa da conexão para verificar o usuário e senha)
    login(con); // Assumindo que a função login foi refatorada
    
    // Se o login falhar (global não definida)
    if (g_id_usuario_logado == 0) {
        printf("Falha no login. Encerrando.\n");
        mysql_close(con);
        mysql_library_end();
        return 1;
    }

    // --- 2. MENU PRINCIPAL ---
    // (Passa a conexão única para o menu)
    menuPrincipal(con);

    // --- 3. FIM ---
    // (Fecha a conexão uma única vez ao sair do programa)
    printf("Fechando conexao com o banco...\n");
    mysql_close(con);
    mysql_library_end(); // Libera a biblioteca

    return 0;
}