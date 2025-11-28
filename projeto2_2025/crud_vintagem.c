#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h> //conexao com o MYSQL
#include <conio.h>
#include <limits.h>

// --- Informações de Conexão ---
const char *server = "localhost";
const char *user = "root";
const char *password = "@vintagem123";
const char *database = "vintagem";

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

//PROTOTIPOS
//Prototipos das funcoes do CRUD
void criarVinho();
void listarVinhos();
void listarVinhosSimples(MYSQL *con);
void listarTodosVinhosSimples(MYSQL *con);
void listarDetalhesVinho(MYSQL *con, int id_vinho);
void atualizarVinho();
void desativarVinho();
int verificar_duplicidade_nome_vinho(MYSQL *con, const char *nome, int id_ignorar);
// Prototipos Genericos
void criarEntidadeSimples(const char* nomeEntidade, const char* nomeTabela, const char* nomeColuna, int maxLen);
void atualizarEntidadeSimples(const char* nomeEntidade, const char* nomeTabela, const char* nomeColunaId, const char* nomeColunaNome, int maxLen);
void listarEntidadeSimples(MYSQL *con, const char* nomeEntidade, const char* nomeTabela, const char* nomeColunaId, const char* nomeColunaNome);
// Prototipos para Gerenciar Tipos
void menuGerenciarTipos();
// Prototipos para Gerenciar Paises
void menuGerenciarPaises();
//prototipos para gerenciar uvas
void menuGerenciarUva();
void gerenciarUvasDoVinho(MYSQL *con, int id_vinho);
void limparUvasDoVinho(MYSQL *con, int id_vinho);
// Prototipo funcao de retorno de id
long long criar_item_retornando_id(MYSQL *con, const char *tabela, const char *coluna_nome, const char *nome_valor);
// Prototipo funcao de busca de id
int obter_id_pelo_nome(MYSQL *con, const char *tabela, const char *coluna_id, const char *coluna_nome, const char *nome_busca);
// prototipo funcao de busca de id ativo
int verificar_id_existe(MYSQL *con, const char *tabela, const char *coluna_id, int id_busca);
// Prototipo da limpeza de terminal
void limparTela();
// Prototipo para pause do terminal
void pausarTela();
//Prototipo login
int realizarLogin();
//Prototipo das movimentacoes
void menuMovimentacoes();
void registrarSaida();
void registrarEntrada();
//Prototipos de LOGs
void menuLogs();
void listarLogs();
void listarTodosLogs();
void registrarLog(const char *acao);
//Prototipo Leitura de caracteres
void lerStringSegura(char *buffer, int tamanho);
int lerInteiroSeguro();

// Funcao auxiliar para tratar erros do MySQL
void finish_with_error(MYSQL *con) {
  fprintf(stderr, "Erro Fatal: %s\n", mysql_error(con));
  mysql_close(con);
  exit(1); // Sai do programa em caso de erro fatal de BD
}

// Funcoes de atualizacao
void atualizarTipo() {
    atualizarEntidadeSimples("Tipo", "tipo", "id_tipo", "nome_tipo", 50);
}
void atualizarPais() {
    atualizarEntidadeSimples("Pais", "paisorigem", "id_paisorigem", "nome_pais", 100);
}
void atualizarUva() {
    atualizarEntidadeSimples("Uva", "uva", "id_uva", "nome_uva", 100);
}

// Funcoes Criar
void criarTipo() {
    criarEntidadeSimples("Tipo", "tipo", "nome_tipo", 50);
}
void criarPais() {
    criarEntidadeSimples("Pais", "paisorigem", "nome_pais", 100);
}
void criarUva() {
    criarEntidadeSimples("Uva", "uva", "nome_uva", 100);
}

int main() {
    int opcao;
    int tentativas;
    int id_logado = 0; // Flag para o ID do usuário

    mysql_library_init(0, NULL, NULL);

    // --- Bloco de Login com 3 Tentativas (Preenchido) ---
    for (tentativas = 1; tentativas <= 3; tentativas++) {
        id_logado = realizarLogin(); // Recebe 0 (falha) ou o ID (sucesso)
        
        if (id_logado > 0) { // Checa se o login foi bem-sucedido
            g_id_usuario_logado = id_logado; // Salva o ID globalmente
            printf("\nLogin realizado com sucesso!\n");
            //registro de LOG
            char log_msg[256];
            sprintf(log_msg, "LOGIN SUCESSO: Usuario %s (ID: %d) logou.", g_nome_nivel_logado, g_id_usuario_logado);
            registrarLog(log_msg);
            pausarTela();
            break; // Sai do loop 'for'
        } else {
            // Lógica de Falha
            limparTela();
            fprintf(stderr, "\n>>> LOGIN FALHOU (Tentativa %d de 3) <<<\n", tentativas);
            fprintf(stderr, "Email ou senha incorretos, ou usuario inativo.\n");
            pausarTela();
            // O loop continuará para a próxima tentativa
        }
    }

    // --- Verificação Pós-Loop ---
    if (g_id_usuario_logado == 0) { // Se a flag global ainda é 0
        // Lógica de 3 Falhas
        limparTela();
        fprintf(stderr, "\n>>> ACESSO NEGADO <<<\n");
        fprintf(stderr, "Voce falhou 3 tentativas de login. O programa sera encerrado.\n");
        pausarTela();
        mysql_library_end(); // Limpa a biblioteca antes de sair
        return 1; // Encerra o programa com erro
    }
    
    // Se o login foi bem-sucedido, o loop principal começa
    do {
        limparTela();
        printf("\n--- ADEGA VINTAGEM ---\n");
        printf("--- Vinhos ---\n");
        printf("1. Adicionar Vinho\n");
        printf("2. Listar Vinhos em Estoque\n");
        printf("3. Atualizar Vinho\n");
        
        printf("\n--- Gerenciamento ---\n");
        printf("4. Gerenciar Tipos\n");
        printf("5. Gerenciar Paises\n");
        printf("6. Gerenciar Uvas\n");
        
        // REMOVIDO: Lista de Inativos
        
        printf("7. Movimentar Estoque\n"); // Renumerado de 8 para 7

        if (strcmp(g_nome_nivel_logado, "Admin") == 0) {
            printf("8. Gerenciar Logs\n"); // Renumerado de 9 para 8
        }
        
        printf("\n0. Sair\n");
        printf("Escolha uma das opcoes acima: ");
        
        opcao = lerInteiroSeguro();

        switch (opcao) {
            case 1: criarVinho(); pausarTela(); break;
            case 2: listarVinhos(); break;
            case 3: atualizarVinho(); pausarTela(); break;
            
            case 4: menuGerenciarTipos(); break;
            case 5: menuGerenciarPaises(); break;
            case 6: menuGerenciarUva(); break;
            
            case 7: menuMovimentacoes(); break; // Agora é o 7
            
            case 8: // Agora é o 8
                if (strcmp(g_nome_nivel_logado, "Admin") == 0) menuLogs();
                else { printf("Opcao invalida!\n"); pausarTela(); }
                break;

            case 0: printf("Saindo do Programa...\n"); break;
            default: printf("Opcao invalida! Tente Novamente.\n"); pausarTela(); break;    
        }
    } while (opcao != 0);

    mysql_library_end(); // Limpa a biblioteca ao sair normalmente
    return 0; // Termina o programa com sucesso
}

// Conecta ao banco de dados
MYSQL* conectar_db() {
    MYSQL *con = mysql_init(NULL);
    const char *plugin_dir = ".\\plugin";

    if (con == NULL) {
        fprintf(stderr, "mysql_init() falhou\n");
        return NULL; // Retorna NULL em vez de exit()
    }

    // Diz explicitamente à biblioteca onde encontrar os plugins
    if (mysql_options(con, MYSQL_PLUGIN_DIR, plugin_dir)) {
        fprintf(stderr, "Erro ao configurar o diretorio de plugins: %s\n", mysql_error(con));
        mysql_close(con);
        return NULL; // Retorna NULL em vez de exit()
    }

    if (mysql_real_connect(con, server, user, password, database, 3306, NULL, 0) == NULL) {
        // Apenas imprime o erro, mas não encerra o programa
        fprintf(stderr, "Erro ao conectar ao banco de dados: %s\n", mysql_error(con));
        mysql_close(con);
        return NULL; // Retorna NULL em vez de exit()
    }
    
    return con;
}


/*
 * Pede email e senha, verifica no banco.
 * Salva o ID e o NOME_NIVEL do usuário nas variáveis globais.
 * Retorna o ID do usuário (ex: 1, 2, 3...) se o login for bem-sucedido.
 * Retorna 0 (false) se o login falhar.
 */
int realizarLogin() {
    char email[101];
    char senha_digitada[256];
    char ch;
    int i = 0;
    
    int id_usuario_out = 0;
    char nome_nivel_out[51] = "";
    
    MYSQL *con = conectar_db();
    
    if (con == NULL) {
        fprintf(stderr, "\nErro: Nao foi possivel conectar ao banco. Tente novamente mais tarde.\n");
        return 0; // <-- CORRIGIDO (retorna int 0)
    }

    MYSQL_STMT *stmt;
    MYSQL_BIND bind_in[2]; 
    MYSQL_BIND bind_out[2];
    
    char query[] = "SELECT u.id_usuario, n.nome_nivel "
                   "FROM usuario AS u "
                   "JOIN nivel AS n ON u.fk_nivel_id_nivel = n.id_nivel "
                   "WHERE u.email = ? AND u.senha = SHA2(?, 256) AND u.ativo = 1";

    limparTela();
    printf("\n--- LOGIN VINTAGEM ---\n");
    printf("Digite seu Email: ");
    
    lerStringSegura(email, 101);

    printf("Digite sua Senha: ");
    i = 0;
    while (1) {
        ch = _getch();
        if (ch == 13) { senha_digitada[i] = '\0'; break; }
        if (ch == 8) { if (i > 0) { i--; printf("\b \b"); } }
        else { if (i < 255) { senha_digitada[i] = ch; i++; printf("*"); } }
    }
    printf("\nVerificando...\n");

    // --- 3. Verificar no Banco ---
    stmt = mysql_stmt_init(con);
    if (!stmt) { 
        fprintf(stderr, " mysql_stmt_init() falhou\n");
        mysql_close(con); 
        return 0; 
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, " mysql_stmt_prepare() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt); 
        mysql_close(con); 
        return 0;
    }

    // Bind dos 2 parâmetros (email e senha)
    memset(bind_in, 0, sizeof(bind_in));
    bind_in[0].buffer_type = MYSQL_TYPE_STRING;
    bind_in[0].buffer = (char *)email;
    bind_in[0].buffer_length = strlen(email);
    
    bind_in[1].buffer_type = MYSQL_TYPE_STRING;
    bind_in[1].buffer = (char *)senha_digitada;
    bind_in[1].buffer_length = strlen(senha_digitada);

    if (mysql_stmt_bind_param(stmt, bind_in)) { 
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return 0;
    }
    
    if (mysql_stmt_execute(stmt)) { 
        fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return 0;
    }

    if (mysql_stmt_store_result(stmt)) { 
        fprintf(stderr, " mysql_stmt_store_result() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return 0;
    }

    // Agora verificamos se a query retornou 1 linha
    if (mysql_stmt_num_rows(stmt) == 1) {
        // Se encontrou, buscamos o ID e o Nível
        memset(bind_out, 0, sizeof(bind_out));
        
        bind_out[0].buffer_type = MYSQL_TYPE_LONG;
        bind_out[0].buffer = (char *)&id_usuario_out;
        
        bind_out[1].buffer_type = MYSQL_TYPE_STRING;
        bind_out[1].buffer = nome_nivel_out;
        bind_out[1].buffer_length = 51;
        
        if (mysql_stmt_bind_result(stmt, bind_out)) {
            fprintf(stderr, " mysql_stmt_bind_result() falhou: %s\n", mysql_stmt_error(stmt));
        } else {
            if (mysql_stmt_fetch(stmt) == 0) {
                // SUCESSO! Salva nas variáveis globais
                g_id_usuario_logado = id_usuario_out;
                strcpy(g_nome_nivel_logado, nome_nivel_out);
            }
        }
    }

    mysql_stmt_close(stmt);
    mysql_close(con);

    return g_id_usuario_logado; // Retorna o ID (ou 0 se falhou)
}

//funcao para buscar ID do tipo e pais
//Retorna o ID se encontrado, ou -1 se não encontrado
int obter_id_pelo_nome(MYSQL *con, const char *tabela, const char *coluna_id, const char *coluna_nome, const char *nome_busca) {
    MYSQL_STMT *stmt;
    MYSQL_BIND bind_in[1]; 
    MYSQL_BIND bind_out[1];
    int id_encontrado = -1;
    char query[512];

    // ALTERAÇÃO: Removido "AND ativo = 1"
    sprintf(query, "SELECT %s FROM %s WHERE %s = ?", coluna_id, tabela, coluna_nome);

    stmt = mysql_stmt_init(con);
    if (!stmt) return -1;

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        mysql_stmt_close(stmt); return -1;
    }

    memset(bind_in, 0, sizeof(bind_in));
    bind_in[0].buffer_type = MYSQL_TYPE_STRING;
    bind_in[0].buffer = (char *)nome_busca;
    bind_in[0].buffer_length = strlen(nome_busca);

    if (mysql_stmt_bind_param(stmt, bind_in)) { mysql_stmt_close(stmt); return -1; }
    if (mysql_stmt_execute(stmt)) { mysql_stmt_close(stmt); return -1; }

    memset(bind_out, 0, sizeof(bind_out));
    bind_out[0].buffer_type = MYSQL_TYPE_LONG;
    bind_out[0].buffer = (char *)&id_encontrado;

    if (mysql_stmt_bind_result(stmt, bind_out)) { mysql_stmt_close(stmt); return -1; }

    if (mysql_stmt_fetch(stmt) != 0) {
        id_encontrado = -1;
    }

    mysql_stmt_close(stmt);
    return id_encontrado;
}

/*
 * RENOMEADA: Verifica apenas se o ID existe na tabela.
 * Não verifica mais se está ativo.
 */
int verificar_id_existe(MYSQL *con, const char *tabela, const char *coluna_id, int id_busca) {
    MYSQL_STMT *stmt;
    MYSQL_BIND bind_in[1];
    MYSQL_BIND bind_out[1];
    int id_encontrado_dummy;
    char query[512];
    int sucesso = 0;

    // ALTERAÇÃO: Removido "AND ativo = 1"
    sprintf(query, "SELECT %s FROM %s WHERE %s = ?", coluna_id, tabela, coluna_id);

    stmt = mysql_stmt_init(con);
    if (!stmt) return 0;

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        mysql_stmt_close(stmt); return 0;
    }

    memset(bind_in, 0, sizeof(bind_in));
    bind_in[0].buffer_type = MYSQL_TYPE_LONG;
    bind_in[0].buffer = (char *)&id_busca;

    if (mysql_stmt_bind_param(stmt, bind_in)) { mysql_stmt_close(stmt); return 0; }
    if (mysql_stmt_execute(stmt)) { mysql_stmt_close(stmt); return 0; }

    memset(bind_out, 0, sizeof(bind_out));
    bind_out[0].buffer_type = MYSQL_TYPE_LONG;
    bind_out[0].buffer = (char *)&id_encontrado_dummy;
    
    if (mysql_stmt_bind_result(stmt, bind_out)) { mysql_stmt_close(stmt); return 0; }

    if (mysql_stmt_fetch(stmt) == 0) {
        sucesso = 1;
    }

    mysql_stmt_close(stmt);
    return sucesso;
}

/*
 * Função auxiliar que insere um item (ex: tipo, pais) e retorna
 * o ID auto-incrementado que acabou de ser gerado.
 * Retorna o ID em caso de sucesso, ou -1 em caso de falha.
 */
long long criar_item_retornando_id(MYSQL *con, const char *tabela, const char *coluna_nome, const char *nome_valor) {
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    char query[256];

    sprintf(query, "INSERT INTO %s (%s) VALUES (?)", tabela, coluna_nome);

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

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)nome_valor;
    bind[0].buffer_length = strlen(nome_valor);

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " (criar_item) mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " (criar_item) mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    long long new_id = mysql_insert_id(con);
    mysql_stmt_close(stmt);
    
    if (new_id == 0) {
        fprintf(stderr, " (criar_item) Nao foi possivel obter o ID inserido.\n");
        return -1;
    }
    
    printf("... Item '%s' cadastrado com ID %lld.\n", nome_valor, new_id);

    // --- ADICIONE O LOG AQUI ---
    char log_msg[512];
    sprintf(log_msg, "CRIOU (On-the-fly) %s: '%s' (ID: %lld)", tabela, nome_valor, new_id);
    registrarLog(log_msg);
    // --- FIM DO LOG ---

    return new_id;
}

/*
 * Função auxiliar que verifica se um ID existe e está ativo em uma tabela.
 * Retorna 1 (true) se o ID for válido e ativo.
 * Retorna 0 (false) se o ID não existir ou estiver inativo.
 */
int verificar_id_ativo(MYSQL *con, const char *tabela, const char *coluna_id, int id_busca) {
    MYSQL_STMT *stmt;
    MYSQL_BIND bind_in[1];
    MYSQL_BIND bind_out[1]; // Precisamos disso para o fetch funcionar
    int id_encontrado_dummy; // Variável "fantasma" só para o bind
    char query[512];
    int sucesso = 0; // 0 = false (não encontrado ou inativo)

    // Query: "SELECT id_tipo FROM tipo WHERE id_tipo = ? AND ativo = 1"
    sprintf(query, "SELECT %s FROM %s WHERE %s = ? AND ativo = 1", coluna_id, tabela, coluna_id);

    stmt = mysql_stmt_init(con);
    // Erro no INIT
    if (!stmt) {
        // Falha em alocar o statement, não podemos fechar o que não existe.
        // Imprimir este erro é útil, pois é um erro de alocação, não de query.
        fprintf(stderr, " (verificar_id) mysql_stmt_init() falhou\n");
        return 0;
    }

    // Erro no PREPARE
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        // Não imprimir erro, apenas falhar silenciosamente
        mysql_stmt_close(stmt);
        return 0;
    }

    // Bind do parâmetro de ENTRADA (o ID que buscamos)
    memset(bind_in, 0, sizeof(bind_in));
    bind_in[0].buffer_type = MYSQL_TYPE_LONG;
    bind_in[0].buffer = (char *)&id_busca;

    // Erro no BIND
    if (mysql_stmt_bind_param(stmt, bind_in)) {
        mysql_stmt_close(stmt);
        return 0;
    }
    
    // Erro no EXECUTE
    if (mysql_stmt_execute(stmt)) {
        mysql_stmt_close(stmt);
        return 0;
    }

    // Bind do parâmetro de SAÍDA (necessário para o fetch)
    memset(bind_out, 0, sizeof(bind_out));
    bind_out[0].buffer_type = MYSQL_TYPE_LONG;
    bind_out[0].buffer = (char *)&id_encontrado_dummy;
    
    // Erro no BIND RESULT
    if (mysql_stmt_bind_result(stmt, bind_out)) {
        mysql_stmt_close(stmt);
        return 0;
    }

    if (mysql_stmt_store_result(stmt)) {
        fprintf(stderr, " (verificar_id) mysql_stmt_store_result() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    // Tenta buscar o resultado
    if (mysql_stmt_fetch(stmt) == 0) {
        // Sucesso! Encontrou um registro.
        sucesso = 1;
    }

    mysql_stmt_close(stmt);
    return sucesso;
}

void criarVinho() {
    Vinho v;
    MYSQL *con = conectar_db();
    if (con == NULL) return;

    MYSQL_STMT *stmt;
    MYSQL_BIND bind[6];
    char query[] = "INSERT INTO vinho(nome_vinho, safra, preco, quantidade, fk_tipo_id_tipo, fk_paisorigem_id_paisorigem, ativo) VALUES(?, ?, ?, ?, ?, ?, 0)";

    char input_buffer[101];
    char preco_buffer[32];
    char resposta_buffer[5];
    long id_digitado = 0;
    char *end_ptr;
    int id_tipo_encontrado = -1;
    int id_pais_encontrado = -1;

    limparTela();
    printf("\n>>> Adicionar Novo Vinho <<<\n");
    
    // 1. Nome (com verificação)
    printf("Nome: ");
    lerStringSegura(v.nome_vinho, 151);
    if (v.nome_vinho[0] == '\0') {
        printf("O nome nao pode ser vazio.\n"); mysql_close(con); return;
    }
    if (verificar_duplicidade_nome_vinho(con, v.nome_vinho, 0)) {
        printf("\nERRO: Ja existe um vinho com o nome '%s'.\n", v.nome_vinho);
        mysql_close(con);
        return;
    }

    printf("Safra (Ano): ");
    v.safra = lerInteiroSeguro();
    if (v.safra == INT_MIN) v.safra = 0;

    printf("Preco (ex: 59.99): ");
    lerStringSegura(preco_buffer, 32);
    v.preco = strtod(preco_buffer, NULL); 
    if (v.preco < 0) v.preco = 0;

    v.quantidade = 0; 
    printf("Quantidade inicial: 0 (Vinho criado como SEM ESTOQUE).\n");

    // --- 2. Tipo (ATUALIZADO) ---
    printf("\n--- Selecao de Tipo ---\n");
    // ALTERAÇÃO: Removido parâmetro '1'
    listarEntidadeSimples(con, "Tipo", "tipo", "id_tipo", "nome_tipo"); 
    printf("Digite o NOME ou o ID do Tipo: ");
    lerStringSegura(input_buffer, 101);
    
    id_digitado = strtol(input_buffer, &end_ptr, 10);
    if (end_ptr == input_buffer) {
        id_tipo_encontrado = obter_id_pelo_nome(con, "tipo", "id_tipo", "nome_tipo", input_buffer);
    } else if (*end_ptr == '\0') {
        // ALTERAÇÃO: verificar_id_existe
        if (verificar_id_existe(con, "tipo", "id_tipo", (int)id_digitado)) id_tipo_encontrado = (int)id_digitado; 
        else id_tipo_encontrado = -1; 
    } else id_tipo_encontrado = -1;

    if (id_tipo_encontrado == -1) {
        if (end_ptr == input_buffer && input_buffer[0] != '\0') { 
            printf("Tipo '%s' nao encontrado. Cadastrar? (s/n): ", input_buffer);
            lerStringSegura(resposta_buffer, 5);
            if (resposta_buffer[0] == 's' || resposta_buffer[0] == 'S') {
                id_tipo_encontrado = (int)criar_item_retornando_id(con, "tipo", "nome_tipo", input_buffer);
                if (id_tipo_encontrado == -1) { mysql_close(con); return; }
            } else { mysql_close(con); return; }
        } else { mysql_close(con); return; }
    }
    v.fk_tipo_id_tipo = id_tipo_encontrado;

    // --- 3. País (ATUALIZADO) ---
    printf("\n--- Selecao de Pais ---\n");
    // ALTERAÇÃO: Removido parâmetro '1'
    listarEntidadeSimples(con, "Pais", "paisorigem", "id_paisorigem", "nome_pais"); 
    printf("Digite o NOME ou o ID do Pais: ");
    lerStringSegura(input_buffer, 101); 
    id_digitado = strtol(input_buffer, &end_ptr, 10);
    
    if (end_ptr == input_buffer) {
        id_pais_encontrado = obter_id_pelo_nome(con, "paisorigem", "id_paisorigem", "nome_pais", input_buffer);
    } else if (*end_ptr == '\0') {
        // ALTERAÇÃO: verificar_id_existe
        if (verificar_id_existe(con, "paisorigem", "id_paisorigem", (int)id_digitado)) id_pais_encontrado = (int)id_digitado; 
        else id_pais_encontrado = -1; 
    } else id_pais_encontrado = -1;
    
    if (id_pais_encontrado == -1) {
        if (end_ptr == input_buffer && input_buffer[0] != '\0') { 
            printf("Pais '%s' nao encontrado. Cadastrar? (s/n): ", input_buffer);
            lerStringSegura(resposta_buffer, 5);
            if (resposta_buffer[0] == 's' || resposta_buffer[0] == 'S') {
                id_pais_encontrado = (int)criar_item_retornando_id(con, "paisorigem", "nome_pais", input_buffer);
                if (id_pais_encontrado == -1) { mysql_close(con); return; }
            } else { mysql_close(con); return; }
        } else { mysql_close(con); return; }
    }
    v.fk_paisorigem_id_paisorigem = id_pais_encontrado;

    // --- 4. INSERT ---
    stmt = mysql_stmt_init(con);
    if (!stmt) { mysql_close(con); return; }
    mysql_stmt_prepare(stmt, query, strlen(query));

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING; bind[0].buffer = (char *)v.nome_vinho; bind[0].buffer_length = strlen(v.nome_vinho);
    bind[1].buffer_type = MYSQL_TYPE_LONG; bind[1].buffer = (char *)&v.safra;
    bind[2].buffer_type = MYSQL_TYPE_DOUBLE; bind[2].buffer = (char *)&v.preco;
    bind[3].buffer_type = MYSQL_TYPE_LONG; bind[3].buffer = (char *)&v.quantidade;
    bind[4].buffer_type = MYSQL_TYPE_LONG; bind[4].buffer = (char *)&v.fk_tipo_id_tipo;
    bind[5].buffer_type = MYSQL_TYPE_LONG; bind[5].buffer = (char *)&v.fk_paisorigem_id_paisorigem;

    mysql_stmt_bind_param(stmt, bind);
    
    if (mysql_stmt_execute(stmt)) { 
        fprintf(stderr, "Erro ao inserir: %s\n", mysql_stmt_error(stmt));
    } else {
        printf("\n>>> Vinho '%s' criado com sucesso! <<<\n", v.nome_vinho);
        long long novo_id_vinho = mysql_insert_id(con);
        char log_msg[256];
        sprintf(log_msg, "CRIOU VINHO: ID %lld (%s)", novo_id_vinho, v.nome_vinho);
        registrarLog(log_msg);

        mysql_stmt_close(stmt);
        if (novo_id_vinho > 0) gerenciarUvasDoVinho(con, (int)novo_id_vinho);
        mysql_close(con);
        return;
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}

//READ
/*
 * (NOVA FUNÇÃO PRINCIPAL)
 * Menu interativo para listar vinhos e ver detalhes.
 * Esta função abre e fecha a própria conexão.
 */
void listarVinhos() {
    MYSQL *con = conectar_db();
    if (con == NULL) {
        fprintf(stderr, "\nErro: Nao foi possivel conectar ao banco. Tente novamente mais tarde.\n");
        return;
    }    
    
    int id_digitado = 0;
    
    do {
        limparTela();
        listarVinhosSimples(con);
        
        printf("\nDigite o ID do vinho para ver detalhes (ou 0 para voltar): ");
        
        id_digitado = lerInteiroSeguro();

        if (id_digitado == INT_MIN) { // INT_MIN é o código de erro
            printf("\nErro: Entrada invalida. Apenas numeros sao permitidos.\n");
            pausarTela();
            continue; // Reinicia o loop
        }
        
        if (id_digitado == 0) {
            break; // Sai do loop
        } else {
            listarDetalhesVinho(con, id_digitado);
            pausarTela();
        }
        
    } while (id_digitado != 0);
    
    mysql_close(con);
    printf("Voltando ao menu principal...\n");
}

void listarTodosVinhosSimples(MYSQL *con) {
    // ALTERAÇÃO: ORDER BY v.id_vinho
    char query[] = 
        "SELECT v.id_vinho, v.nome_vinho, v.quantidade, IFNULL(t.nome_tipo, 'N/A'), "
        "CASE WHEN v.ativo = 1 THEN 'EM ESTOQUE' ELSE 'SEM ESTOQUE' END as status "
        "FROM vinho AS v "
        "LEFT JOIN tipo AS t ON v.fk_tipo_id_tipo = t.id_tipo "
        "ORDER BY v.id_vinho"; 

    if (mysql_query(con, query)) {
        fprintf(stderr, "Erro ao listar todos vinhos: %s\n", mysql_error(con));
        return;
    }
    MYSQL_RES *result = mysql_store_result(con);
    if (!result) return;

    MYSQL_ROW row;
    printf("\n>>> Selecao de Vinho (Todos) <<<\n");
    printf("%-3s | %-35s | %-5s | %-15s | %-10s\n", "ID", "Nome", "Qtd", "Tipo", "Status");
    printf("--------------------------------------------------------------------------------\n");
    while ((row = mysql_fetch_row(result))) {
        printf("%-3s | %-35s | %-5s | %-15s | %-10s\n",
               row[0], row[1], row[2], row[3], row[4]);
    }
    printf("--------------------------------------------------------------------------------\n");
    mysql_free_result(result);
}

void atualizarVinho() {
    int id_busca = 0; 
    Vinho v;
    MYSQL *con = conectar_db();
    if (con == NULL) return;
    
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[6]; 
    char query[] = "UPDATE vinho SET nome_vinho = ?, safra = ?, preco = ?, fk_tipo_id_tipo = ?, fk_paisorigem_id_paisorigem = ? WHERE id_vinho = ?";

    char input_buffer[101];
    char preco_buffer[32];
    char resposta_buffer[5];
    char *end_ptr; 
    long id_digitado = 0;
    int id_tipo_encontrado = -1;
    int id_pais_encontrado = -1;

    limparTela();
    printf("\n--- ATUALIZAR VINHO ---\n");
    listarTodosVinhosSimples(con); 

    printf("\nDigite o ID do Vinho que deseja atualizar (ou 0 para cancelar): ");
    id_busca = lerInteiroSeguro();

    if (id_busca <= 0) {
        printf("Operacao cancelada.\n");
        mysql_close(con);
        return; 
    }

    printf("\n>>> Insira os NOVOS dados para o Vinho ID %d <<<\n", id_busca);
    
    printf("Novo Nome: ");
    lerStringSegura(v.nome_vinho, 151);
    if (v.nome_vinho[0] == '\0') {
        printf("Nome nao pode ser vazio.\n"); mysql_close(con); return;
    }

    if (verificar_duplicidade_nome_vinho(con, v.nome_vinho, id_busca)) {
        printf("\nERRO: Ja existe OUTRO vinho com o nome '%s'.\n", v.nome_vinho);
        mysql_close(con);
        return;
    }

    printf("Nova Safra (Ano): ");
    v.safra = lerInteiroSeguro();
    if (v.safra == INT_MIN) v.safra = 0;

    printf("Novo Preco (ex: 59.99): ");
    lerStringSegura(preco_buffer, 32);
    v.preco = strtod(preco_buffer, NULL);
    if (v.preco < 0) v.preco = 0;

    // --- Selecao de Tipo (ATUALIZADO) ---
    printf("\n--- Selecao do NOVO Tipo ---\n");
    // ALTERAÇÃO: Removido parâmetro '1'
    listarEntidadeSimples(con, "Tipo", "tipo", "id_tipo", "nome_tipo");
    printf("Digite o NOME ou o ID do novo Tipo: ");
    lerStringSegura(input_buffer, 101); 
    
    id_digitado = strtol(input_buffer, &end_ptr, 10);
    if (end_ptr == input_buffer) id_tipo_encontrado = obter_id_pelo_nome(con, "tipo", "id_tipo", "nome_tipo", input_buffer);
    else if (*end_ptr == '\0') {
        // ALTERAÇÃO: verificar_id_existe
        if (verificar_id_existe(con, "tipo", "id_tipo", (int)id_digitado)) id_tipo_encontrado = (int)id_digitado;
        else id_tipo_encontrado = -1;
    } else id_tipo_encontrado = -1;

    if (id_tipo_encontrado == -1) {
        if (end_ptr == input_buffer && input_buffer[0] != '\0') { 
            printf("Tipo '%s' nao encontrado. Cadastrar? (s/n): ", input_buffer);
            lerStringSegura(resposta_buffer, 5);
            if (resposta_buffer[0] == 's' || resposta_buffer[0] == 'S') {
                id_tipo_encontrado = (int)criar_item_retornando_id(con, "tipo", "nome_tipo", input_buffer);
                if (id_tipo_encontrado == -1) { mysql_close(con); return; }
            } else { mysql_close(con); return; }
        } else { mysql_close(con); return; }
    }
    v.fk_tipo_id_tipo = id_tipo_encontrado;

    // --- Selecao de País (ATUALIZADO) ---
    printf("\n--- Selecao do NOVO Pais ---\n");
    // ALTERAÇÃO: Removido parâmetro '1'
    listarEntidadeSimples(con, "Pais", "paisorigem", "id_paisorigem", "nome_pais");
    printf("Digite o NOME ou o ID do novo Pais: ");
    lerStringSegura(input_buffer, 101);
    
    id_digitado = strtol(input_buffer, &end_ptr, 10);
    if (end_ptr == input_buffer) id_pais_encontrado = obter_id_pelo_nome(con, "paisorigem", "id_paisorigem", "nome_pais", input_buffer);
    else if (*end_ptr == '\0') {
        // ALTERAÇÃO: verificar_id_existe
        if (verificar_id_existe(con, "paisorigem", "id_paisorigem", (int)id_digitado)) id_pais_encontrado = (int)id_digitado;
        else id_pais_encontrado = -1;
    } else id_pais_encontrado = -1;

    if (id_pais_encontrado == -1) {
        if (end_ptr == input_buffer && input_buffer[0] != '\0') { 
            printf("Pais '%s' nao encontrado. Cadastrar? (s/n): ", input_buffer);
            lerStringSegura(resposta_buffer, 5);
            if (resposta_buffer[0] == 's' || resposta_buffer[0] == 'S') {
                id_pais_encontrado = (int)criar_item_retornando_id(con, "paisorigem", "nome_pais", input_buffer);
                if (id_pais_encontrado == -1) { mysql_close(con); return; }
            } else { mysql_close(con); return; }
        } else { mysql_close(con); return; }
    }
    v.fk_paisorigem_id_paisorigem = id_pais_encontrado;

    // --- UPDATE ---
    stmt = mysql_stmt_init(con);
    if (!stmt) { mysql_close(con); return; }
    mysql_stmt_prepare(stmt, query, strlen(query));

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING; bind[0].buffer = (char *)v.nome_vinho; bind[0].buffer_length = strlen(v.nome_vinho);
    bind[1].buffer_type = MYSQL_TYPE_LONG; bind[1].buffer = (char *)&v.safra;
    bind[2].buffer_type = MYSQL_TYPE_DOUBLE; bind[2].buffer = (char *)&v.preco;
    bind[3].buffer_type = MYSQL_TYPE_LONG; bind[3].buffer = (char *)&v.fk_tipo_id_tipo;
    bind[4].buffer_type = MYSQL_TYPE_LONG; bind[4].buffer = (char *)&v.fk_paisorigem_id_paisorigem;
    bind[5].buffer_type = MYSQL_TYPE_LONG; bind[5].buffer = (char *)&id_busca;

    mysql_stmt_bind_param(stmt, bind);
    mysql_stmt_execute(stmt);

    if (mysql_stmt_affected_rows(stmt) > 0) {
        printf("\n>>> Vinho ID %d atualizado! <<<\n", id_busca);
        char log_msg[256];
        sprintf(log_msg, "ATUALIZOU VINHO (DADOS): ID %d para Nome: %s, Safra: %d, Preco: %.2f", 
                id_busca, v.nome_vinho, v.safra, v.preco);
        registrarLog(log_msg);
    } else {
        printf("\n>>> Nenhum dado foi alterado ou ID nao encontrado. <<<\n");
    }

    mysql_stmt_close(stmt);

    printf("\n--- Gerenciamento de Uvas ---\n");
    limparUvasDoVinho(con, id_busca); 
    gerenciarUvasDoVinho(con, id_busca); 

    mysql_close(con);
}

void listarEntidadeSimples(MYSQL *con, const char* nomeEntidade, const char* nomeTabela, const char* nomeColunaId, const char* nomeColunaNome) {
    char query[512];
    
    // ALTERAÇÃO: ORDER BY nomeColunaId (ordem numérica) em vez de nomeColunaNome
    sprintf(query, "SELECT %s, %s FROM %s ORDER BY %s", 
            nomeColunaId, nomeColunaNome, nomeTabela, nomeColunaId); 

    if (mysql_query(con, query)) {
        fprintf(stderr, "Erro ao listar %s: %s\n", nomeTabela, mysql_error(con));
        return; 
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        fprintf(stderr, "Erro ao obter resultados: %s\n", mysql_error(con));
        return;
    }

    MYSQL_ROW row;
    int num_rows = mysql_num_rows(result);

    printf("\n--- Lista de %s ---\n", nomeEntidade);
    if (num_rows == 0) {
        printf("Nenhum %s cadastrado.\n", nomeEntidade);
    } else {
        printf("%-5s %-30s\n", "ID", "Nome");
        printf("-------------------------------------\n");
        while ((row = mysql_fetch_row(result))) {
            printf("%-5s %-30s\n", row[0], row[1]);
        }
        printf("-------------------------------------\n");
    }
    mysql_free_result(result);
}

/*
 * (NOVA FUNÇÃO GENÉRICA DE CRIAÇÃO)
 * Cria uma entidade simples (Tipo, Pais, Uva)
 * Esta função abre/fecha a própria conexão e usa input seguro.
 */
void criarEntidadeSimples(const char* nomeEntidade, const char* nomeTabela, const char* nomeColuna, int maxLen) {
    MYSQL *con = conectar_db();
    
    // VERIFICA A CONEXAO
    if (con == NULL) {
        fprintf(stderr, "\nErro: Nao foi possivel conectar ao banco.\n");
        return;
    }
    
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    char nomeValor[256]; // Buffer seguro
    char query[512];

    if (maxLen > 255) maxLen = 255; 

    printf("\n--- Adicionar Novo %s ---\n", nomeEntidade);
    printf("Nome do %s: ", nomeEntidade);
    
    // USA LEITURA SEGURA
    lerStringSegura(nomeValor, maxLen + 1); // +1 para o '\0'

    if (nomeValor[0] == '\0') {
        printf("O nome nao pode ser vazio. Operacao cancelada.\n");
        mysql_close(con);
        return;
    }

    sprintf(query, "INSERT INTO %s (%s) VALUES (?)", nomeTabela, nomeColuna);

    stmt = mysql_stmt_init(con);
    if (!stmt) {
        fprintf(stderr, " mysql_stmt_init() falhou\n");
        mysql_close(con);
        return;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, " mysql_stmt_prepare() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)nomeValor;
    bind[0].buffer_length = strlen(nomeValor);

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        if (mysql_errno(con) == 1062) {
            fprintf(stderr, "\nErro: O %s '%s' ja esta cadastrado.\n", nomeEntidade, nomeValor);
        } else {
            fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        }
    } else {
        printf("\n%s '%s' criado com sucesso!\n", nomeEntidade, nomeValor);
        
        char log_msg[512];
        sprintf(log_msg, "CRIOU %s: %s", nomeEntidade, nomeValor);
        registrarLog(log_msg);
    }

    mysql_stmt_close(stmt);
    mysql_close(con);
}

//Atualiza o nome de uma entidade simples (Tipo, Pais, Uva).
void atualizarEntidadeSimples(const char* nomeEntidade, const char* nomeTabela, const char* nomeColunaId, const char* nomeColunaNome, int maxLen) {
    MYSQL *con = conectar_db();
    if (con == NULL) return;

    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];
    char query[512];
    char novoNome[256];
    int id_busca = 0;

    if (maxLen > 255) maxLen = 255;

    limparTela();
    printf("\n--- Atualizar %s ---\n", nomeEntidade);
    // ALTERAÇÃO: Chamada sem o parâmetro 'status'
    listarEntidadeSimples(con, nomeEntidade, nomeTabela, nomeColunaId, nomeColunaNome);

    printf("\nDigite o ID do %s a atualizar (ou 0 para cancelar): ", nomeEntidade);
    id_busca = lerInteiroSeguro();

    if (id_busca <= 0) {
        printf("Operacao cancelada.\n");
        mysql_close(con);
        return;
    }

    printf("Novo Nome: ");
    lerStringSegura(novoNome, maxLen + 1);
    if (novoNome[0] == '\0') {
        printf("Nome invalido.\n"); mysql_close(con); return;
    }

    sprintf(query, "UPDATE %s SET %s = ? WHERE %s = ?", nomeTabela, nomeColunaNome, nomeColunaId);

    stmt = mysql_stmt_init(con);
    if (!stmt) { mysql_close(con); return; }
    if (mysql_stmt_prepare(stmt, query, strlen(query))) { mysql_stmt_close(stmt); mysql_close(con); return; }

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING; bind[0].buffer = (char *)novoNome; bind[0].buffer_length = strlen(novoNome);
    bind[1].buffer_type = MYSQL_TYPE_LONG; bind[1].buffer = (char *)&id_busca;

    if (mysql_stmt_bind_param(stmt, bind)) { mysql_stmt_close(stmt); mysql_close(con); return; }
    
    if (mysql_stmt_execute(stmt)) {
        if (mysql_errno(con) == 1062) printf("\nErro: Nome ja existe.\n");
        else printf("\nErro ao atualizar.\n");
    } else {
        if (mysql_stmt_affected_rows(stmt) > 0) {
            printf("\n%s atualizado com sucesso!\n", nomeEntidade);
            char log_msg[512];
            sprintf(log_msg, "ATUALIZOU %s: ID %d para '%s'", nomeEntidade, id_busca, novoNome);
            registrarLog(log_msg);
        } else {
            printf("\nID nao encontrado.\n");
        }
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}

void menuGerenciarTipos() {
    int opcao;
    do {
        limparTela();
        printf("\n--- GERENCIAR TIPOS ---\n");
        printf("1. Listar Tipos\n");
        printf("2. Adicionar Tipo\n");
        printf("3. Atualizar Tipo\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        opcao = lerInteiroSeguro();
        switch (opcao) {
            case 1: {
                MYSQL *con = conectar_db();
                if (con) {
                    // ALTERAÇÃO: Sem parâmetro '1'
                    listarEntidadeSimples(con, "Tipo", "tipo", "id_tipo", "nome_tipo");
                    mysql_close(con);
                }
                pausarTela();
            } break;
            case 2: criarTipo(); pausarTela(); break;
            case 3: atualizarTipo(); pausarTela(); break;
            case 0: printf("Voltando...\n"); break;
            default: printf("Invalido.\n"); pausarTela(); break;
        }
    } while (opcao != 0);
}

void menuGerenciarPaises() {
    int opcao;
    do {
        limparTela();
        printf("\n--- GERENCIAR PAISES ---\n");
        printf("1. Listar Paises\n");
        printf("2. Adicionar Pais\n");
        printf("3. Atualizar Pais\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        opcao = lerInteiroSeguro();
        switch (opcao) {
            case 1: {
                MYSQL *con = conectar_db();
                if (con) {
                    listarEntidadeSimples(con, "Pais", "paisorigem", "id_paisorigem", "nome_pais");
                    mysql_close(con);
                }
                pausarTela();
            } break;
            case 2: criarPais(); pausarTela(); break;
            case 3: atualizarPais(); pausarTela(); break;
            case 0: printf("Voltando...\n"); break;
            default: printf("Invalido.\n"); pausarTela(); break;
        }
    } while (opcao != 0);
}

void menuGerenciarUva() {
    int opcao;
    do {
        limparTela();
        printf("\n--- GERENCIAR UVAS ---\n");
        printf("1. Listar Uvas\n");
        printf("2. Adicionar Uva\n");
        printf("3. Atualizar Uva\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        opcao = lerInteiroSeguro();
        switch (opcao) {
            case 1: {
                MYSQL *con = conectar_db();
                if (con) {
                    listarEntidadeSimples(con, "Uva", "uva", "id_uva", "nome_uva");
                    mysql_close(con);
                }
                pausarTela();
            } break;
            case 2: criarUva(); pausarTela(); break;
            case 3: atualizarUva(); pausarTela(); break;
            case 0: printf("Voltando...\n"); break;
            default: printf("Invalido.\n"); pausarTela(); break;
        }
    } while (opcao != 0);
}

// --- LÓGICA N-para-N (UVAS) ---
// Limpa as uvas associadas a um vinho (para a função UPDATE)
void limparUvasDoVinho(MYSQL *con, int id_vinho) {
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    char query[] = "DELETE FROM composto_por WHERE fk_vinho_id_vinho = ?";

    stmt = mysql_stmt_init(con);
    if (!stmt) { 
        fprintf(stderr, " (limparUvas) mysql_stmt_init() falhou\n");
        return;
    }
    
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, " (limparUvas) mysql_stmt_prepare() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }
    
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&id_vinho;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " (limparUvas) mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " (limparUvas) mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
    }
    
    printf("\n... Uvas anteriores do Vinho ID %d removidas (para atualizacao).\n", id_vinho);
    //registro de LOG
    char log_msg[256];
    sprintf(log_msg, "REMOVEU UVAS (UPDATE): Todas as uvas do Vinho ID %d", id_vinho);
    registrarLog(log_msg);

    mysql_stmt_close(stmt);
}

/*
 * Função auxiliar para gerenciar a adição de uvas (N-para-N)
 * a um vinho, inserindo na tabela 'composto_por'.
 * Esta função recebe uma conexão JÁ ABERTA.
 */
void gerenciarUvasDoVinho(MYSQL *con, int id_vinho) {
    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;
    int id_uva_encontrada = -1;
    char resposta_buffer[5]; 
    char resposta = 's';     
    
    while (resposta == 's' || resposta == 'S') {
        limparTela();
        printf("\n--- Adicionar Uvas ao Vinho (ID: %d) ---\n", id_vinho);
        
        // 1. Lista as uvas disponíveis (ATUALIZADO)
        // ALTERAÇÃO: Removido parâmetro '1'
        listarEntidadeSimples(con, "Uva", "uva", "id_uva", "nome_uva");
        
        printf("Digite o NOME ou ID da uva (ou '0' para parar): ");
        lerStringSegura(input_buffer, 101);

        if (strcmp(input_buffer, "0") == 0) break;
        
        id_digitado = strtol(input_buffer, &end_ptr, 10);
        id_uva_encontrada = -1; 

        if (end_ptr == input_buffer) {
            id_uva_encontrada = obter_id_pelo_nome(con, "uva", "id_uva", "nome_uva", input_buffer);
        } else if (*end_ptr == '\0') {
            // ALTERAÇÃO: verificar_id_existe
            if (verificar_id_existe(con, "uva", "id_uva", (int)id_digitado)) {
                id_uva_encontrada = (int)id_digitado;
            } else {
                printf("\nErro: ID de uva '%ld' nao existe.\n", id_digitado);
                pausarTela();
            }
        } else {
            printf("\nErro: Entrada '%s' invalida.\n", input_buffer);
            pausarTela();
        }

        if (id_uva_encontrada == -1 && end_ptr == input_buffer && input_buffer[0] != '\0') {
             printf("Uva '%s' nao encontrada. Deseja cadastra-la? (s/n): ", input_buffer);
             lerStringSegura(resposta_buffer, 5);
             resposta = resposta_buffer[0];

             if (resposta == 's' || resposta == 'S') {
                 id_uva_encontrada = (int)criar_item_retornando_id(con, "uva", "nome_uva", input_buffer);
                 if (id_uva_encontrada == -1) {
                     fprintf(stderr, "Erro ao cadastrar nova uva.\n");
                     pausarTela();
                     continue; 
                 }
             }
        }

        if (id_uva_encontrada != -1) {
            MYSQL_STMT *stmt_junction;
            MYSQL_BIND bind_junction[2];
            char query_junction[] = "INSERT INTO composto_por (fk_vinho_id_vinho, fk_uva_id_uva) VALUES (?, ?)";

            stmt_junction = mysql_stmt_init(con);
            if (!stmt_junction) continue; 
            if (mysql_stmt_prepare(stmt_junction, query_junction, strlen(query_junction))) {
                mysql_stmt_close(stmt_junction); continue;
            }
            
            memset(bind_junction, 0, sizeof(bind_junction));
            bind_junction[0].buffer_type = MYSQL_TYPE_LONG; bind_junction[0].buffer = (char *)&id_vinho;
            bind_junction[1].buffer_type = MYSQL_TYPE_LONG; bind_junction[1].buffer = (char *)&id_uva_encontrada;

            if (mysql_stmt_bind_param(stmt_junction, bind_junction)) { mysql_stmt_close(stmt_junction); continue; }
            
            if (mysql_stmt_execute(stmt_junction)) {
                if (mysql_errno(con) == 1062) printf("\n... Esta uva ja foi adicionada.\n");
                else fprintf(stderr, "\nErro ao adicionar uva.\n");
            } else {
                printf("\n... Uva (ID: %d) adicionada!\n", id_uva_encontrada);
                char log_msg[256];
                sprintf(log_msg, "ASSOCIACAO VINHO-UVA: Vinho ID %d <- Uva ID %d", id_vinho, id_uva_encontrada);
                registrarLog(log_msg);
            }
            mysql_stmt_close(stmt_junction);
        }
        
        printf("\nDeseja adicionar outra uva a este vinho? (s/n): ");
        lerStringSegura(resposta_buffer, 5);
        resposta = resposta_buffer[0];
    }
    printf("\nGerenciamento de uvas concluido.\n");
}

/*
 * Menu para Movimentações de Estoque
 */
void menuMovimentacoes() {
    int opcao;
    do {
        limparTela();
        printf("\n--- MOVIMENTAR ESTOQUE ---\n");
        printf("1. Registrar Saida (Venda)\n");
        printf("2. Registrar Entrada (Compra)\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        
        opcao = lerInteiroSeguro();

        switch (opcao) {
            case 1: 
                registrarSaida(); 
                pausarTela(); 
                break;
            case 2: 
                // --- ALTERAÇÃO AQUI ---
                registrarEntrada(); 
                pausarTela(); 
                break;
            case 0: 
                printf("Voltando...\n"); 
                break;
            default: 
                printf("Opcao invalida!\n");
                pausarTela();
                break;
        }
    } while (opcao != 0);
}

void registrarSaida() {
    int id_vinho = 0;
    int quantidade_venda = 0;
    char nome_cliente[101];

    MYSQL *con = conectar_db();
    if (con == NULL) return;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[3];
    long id_digitado = 0;

    limparTela();
    printf("\n--- REGISTRAR SAIDA (VENDA) ---\n");
    listarVinhosSimples(con); // Lista apenas os ativos

    printf("\nDigite o ID do Vinho (ou 0 para cancelar): ");
    id_digitado = lerInteiroSeguro();
    if (id_digitado <= 0) { mysql_close(con); return; }
    id_vinho = (int)id_digitado;

    printf("Digite a quantidade: ");
    quantidade_venda = lerInteiroSeguro();
    if (quantidade_venda <= 0) { mysql_close(con); return; }
    
    printf("Cliente: ");
    lerStringSegura(nome_cliente, 101);

    char nome_vinho[151];
    double preco_vinho = 0;
    int qtd_estoque = 0;
    
    // 1. Verificar Estoque
    char query_select[] = "SELECT nome_vinho, preco, quantidade FROM vinho WHERE id_vinho = ? AND ativo = 1";
    stmt = mysql_stmt_init(con);
    if (!stmt) { mysql_close(con); return; } // Proteção extra

    mysql_stmt_prepare(stmt, query_select, strlen(query_select));
    
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG; bind[0].buffer = (char *)&id_vinho;
    mysql_stmt_bind_param(stmt, bind);
    mysql_stmt_execute(stmt);
    mysql_stmt_store_result(stmt);
    
    if (mysql_stmt_num_rows(stmt) == 0) {
        printf("\nVinho nao encontrado ou sem estoque.\n");
        mysql_stmt_close(stmt); mysql_close(con); return;
    }

    MYSQL_BIND bind_out[3];
    memset(bind_out, 0, sizeof(bind_out));
    bind_out[0].buffer_type = MYSQL_TYPE_STRING; bind_out[0].buffer = nome_vinho; bind_out[0].buffer_length = 151;
    bind_out[1].buffer_type = MYSQL_TYPE_DOUBLE; bind_out[1].buffer = (char *)&preco_vinho;
    bind_out[2].buffer_type = MYSQL_TYPE_LONG; bind_out[2].buffer = (char *)&qtd_estoque;
    mysql_stmt_bind_result(stmt, bind_out);
    mysql_stmt_fetch(stmt);
    mysql_stmt_close(stmt);

    if (quantidade_venda > qtd_estoque) {
        printf("\nErro: Estoque insuficiente (Disp: %d).\n", qtd_estoque);
        mysql_close(con); return;
    }

    // 2. CALCULAR NOVO STATUS E ESTOQUE
    int novo_estoque = qtd_estoque - quantidade_venda;
    int novo_status = (novo_estoque > 0) ? 1 : 0; // Se sobrar 0, desativa (0)

    // 3. UPDATE INTELIGENTE
    // Atualiza quantidade E o status automaticamente
    char query_update[] = "UPDATE vinho SET quantidade = ?, ativo = ? WHERE id_vinho = ?";
    stmt = mysql_stmt_init(con);
    mysql_stmt_prepare(stmt, query_update, strlen(query_update));

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG; bind[0].buffer = (char *)&novo_estoque;
    bind[1].buffer_type = MYSQL_TYPE_LONG; bind[1].buffer = (char *)&novo_status;
    bind[2].buffer_type = MYSQL_TYPE_LONG; bind[2].buffer = (char *)&id_vinho;
    
    mysql_stmt_bind_param(stmt, bind);
    mysql_stmt_execute(stmt);
    mysql_stmt_close(stmt);

    // 4. INSERT Movimentação
    char query_insert[] = "INSERT INTO movimentacao (tipo_movimentacao, quantidade_movimentada, fk_vinho_id_vinho, fk_usuario_id_usuario) VALUES ('SAIDA', ?, ?, ?)";
    stmt = mysql_stmt_init(con);
    mysql_stmt_prepare(stmt, query_insert, strlen(query_insert));

    MYSQL_BIND bind_ins[3];
    memset(bind_ins, 0, sizeof(bind_ins));
    bind_ins[0].buffer_type = MYSQL_TYPE_LONG; bind_ins[0].buffer = (char *)&quantidade_venda;
    bind_ins[1].buffer_type = MYSQL_TYPE_LONG; bind_ins[1].buffer = (char *)&id_vinho;
    bind_ins[2].buffer_type = MYSQL_TYPE_LONG; bind_ins[2].buffer = (char *)&g_id_usuario_logado; 

    mysql_stmt_bind_param(stmt, bind_ins);
    mysql_stmt_execute(stmt);
    mysql_stmt_close(stmt);

    // LOG CORRIGIDO: Inclui o nome do cliente
    char log_msg[256];
    sprintf(log_msg, "VENDA: %d un. Vinho ID %d para Cliente: %s", quantidade_venda, id_vinho, nome_cliente);
    registrarLog(log_msg);

    printf("\nVenda Concluida.\nNovo Estoque: %d\n", novo_estoque);
    if (novo_status == 0) {
        printf(">>> ALERTA: O vinho foi automaticamente DESATIVADO pois o estoque zerou. <<<\n");
    }

    mysql_close(con);
}

void registrarEntrada() {
    int id_vinho = 0;
    int quantidade_entrada = 0;
    char fornecedor[101];

    MYSQL *con = conectar_db();
    if (con == NULL) return;
    
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];
    long id_digitado = 0;

    limparTela();
    printf("\n--- REGISTRAR ENTRADA (COMPRA) ---\n");
    // Chama a função auxiliar acima
    listarTodosVinhosSimples(con); 

    printf("\nDigite o ID do Vinho recebido (ou 0 para cancelar): ");
    id_digitado = lerInteiroSeguro();
    if (id_digitado <= 0) { mysql_close(con); return; }
    id_vinho = (int)id_digitado;

    printf("Digite a quantidade recebida: ");
    quantidade_entrada = lerInteiroSeguro();
    if (quantidade_entrada <= 0) {
        printf("Quantidade invalida.\n"); mysql_close(con); return;
    }
    
    printf("Digite o nome do Fornecedor: ");
    lerStringSegura(fornecedor, 101);

    // 1. Verificar estoque atual e existência
    char nome_vinho[151];
    int qtd_estoque_atual = 0;
    
    char query_select[] = "SELECT nome_vinho, quantidade FROM vinho WHERE id_vinho = ?";
    stmt = mysql_stmt_init(con);
    if (!stmt) { mysql_close(con); return; }
    
    if (mysql_stmt_prepare(stmt, query_select, strlen(query_select))) {
        mysql_stmt_close(stmt); mysql_close(con); return;
    }
    
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG; bind[0].buffer = (char *)&id_vinho;
    mysql_stmt_bind_param(stmt, bind);
    mysql_stmt_execute(stmt);
    mysql_stmt_store_result(stmt);
    
    if (mysql_stmt_num_rows(stmt) == 0) {
        printf("\nVinho ID %d nao encontrado.\n", id_vinho);
        mysql_stmt_close(stmt); mysql_close(con); return;
    }

    MYSQL_BIND bind_out[2];
    memset(bind_out, 0, sizeof(bind_out));
    bind_out[0].buffer_type = MYSQL_TYPE_STRING; bind_out[0].buffer = nome_vinho; bind_out[0].buffer_length = 151;
    bind_out[1].buffer_type = MYSQL_TYPE_LONG; bind_out[1].buffer = (char *)&qtd_estoque_atual;
    mysql_stmt_bind_result(stmt, bind_out);
    mysql_stmt_fetch(stmt);
    mysql_stmt_close(stmt);

    // 2. UPDATE: Soma estoque E força ATIVO = 1 (Reativação Automática)
    char query_update[] = "UPDATE vinho SET quantidade = (quantidade + ?), ativo = 1 WHERE id_vinho = ?";
    
    stmt = mysql_stmt_init(con);
    mysql_stmt_prepare(stmt, query_update, strlen(query_update));

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG; bind[0].buffer = (char *)&quantidade_entrada;
    bind[1].buffer_type = MYSQL_TYPE_LONG; bind[1].buffer = (char *)&id_vinho;
    
    mysql_stmt_bind_param(stmt, bind);
    
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Erro no Update: %s\n", mysql_stmt_error(stmt));
    } else {
        printf("\nSucesso! Estoque atualizado e Vinho ativado.\n");
    }
    mysql_stmt_close(stmt);

    // 3. INSERT Movimentação
    char query_insert[] = "INSERT INTO movimentacao (tipo_movimentacao, quantidade_movimentada, fk_vinho_id_vinho, fk_usuario_id_usuario) VALUES ('ENTRADA', ?, ?, ?)";
    stmt = mysql_stmt_init(con);
    mysql_stmt_prepare(stmt, query_insert, strlen(query_insert));

    MYSQL_BIND bind_ins[3];
    memset(bind_ins, 0, sizeof(bind_ins));
    bind_ins[0].buffer_type = MYSQL_TYPE_LONG; bind_ins[0].buffer = (char *)&quantidade_entrada;
    bind_ins[1].buffer_type = MYSQL_TYPE_LONG; bind_ins[1].buffer = (char *)&id_vinho;
    bind_ins[2].buffer_type = MYSQL_TYPE_LONG; bind_ins[2].buffer = (char *)&g_id_usuario_logado; 

    mysql_stmt_bind_param(stmt, bind_ins);
    mysql_stmt_execute(stmt);
    mysql_stmt_close(stmt);

    // 4. Log (Corrigido para incluir o Fornecedor)
    char log_msg[256];
    // CORREÇÃO AQUI: Adicionado "de %s" e a variável fornecedor
    sprintf(log_msg, "COMPRA: %d un. Vinho ID %d de %s (Total: %d)", 
            quantidade_entrada, id_vinho, fornecedor, qtd_estoque_atual + quantidade_entrada);
    registrarLog(log_msg);

    // 5. Resumo Final
    printf("Novo Estoque: %d (Status: ATIVO)\n", qtd_estoque_atual + quantidade_entrada);
    
    mysql_close(con);
}

/*
 * ==========================================================
 * FUNÇÕES DE LOG (Completas e Corrigidas)
 * ==========================================================
 */

/*
 * (MOTOR DE LOG)
 * Registra uma ação na tabela de log.
 * Esta função abre e fecha sua própria conexão para ser rápida e independente.
 * Ela usa "falha silenciosa" intencionalmente para não travar o programa
 * principal se o log falhar.
 */
void registrarLog(const char *acao) {
    // Se nenhum usuário estiver logado, não faz nada
    if (g_id_usuario_logado == 0) return;

    MYSQL *con_log = conectar_db();
    
    // --- VERIFICACAO CRITICA ---
    // Se a conexao falhar (ex: banco offline), apenas
    // aborte o log, mas não feche o programa.
    if (con_log == NULL) {
        fprintf(stderr, "[AVISO LOG]: Nao foi possivel conectar ao banco para registrar o log.\n");
        return;
    }
    // --- FIM DA VERIFICACAO ---

    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];
    char query[] = "INSERT INTO log (acao, fk_usuario_id_usuario) VALUES (?, ?)";

    stmt = mysql_stmt_init(con_log);
    if (!stmt) { 
        // Falha silenciosa
        mysql_close(con_log); 
        return; 
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        // Falha silenciosa
        mysql_stmt_close(stmt);
        mysql_close(con_log);
        return;
    }

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)acao;
    bind[0].buffer_length = strlen(acao);
    
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *)&g_id_usuario_logado;

    if (mysql_stmt_bind_param(stmt, bind)) { 
        // Falha silenciosa
        mysql_stmt_close(stmt);
        mysql_close(con_log);
        return;
    }

    if (mysql_stmt_execute(stmt)) { 
        // Falha silenciosa
    }

    mysql_stmt_close(stmt);
    mysql_close(con_log);
}

/*
 * (VISUALIZADOR DE LOG)
 * Mostra o histórico de logs do sistema.
 */
void listarLogs() {
    registrarLog("VISUALIZOU LOGS (Ultimos 50)");
    MYSQL *con = conectar_db();
    if (con == NULL) {
        fprintf(stderr, "\nErro: Nao foi possivel conectar ao banco. Tente novamente mais tarde.\n");
        return; 
    }
    
    // ALTERAÇÃO: ORDER BY l.id_log DESC
    char query[] = "SELECT l.id_log, l.data_hora, l.acao, u.nome_usuario "
                   "FROM log AS l "
                   "JOIN usuario AS u ON l.fk_usuario_id_usuario = u.id_usuario "
                   "ORDER BY l.id_log DESC " // IDs maiores (mais novos) primeiro
                   "LIMIT 50"; 

    if (mysql_query(con, query)) {
        fprintf(stderr, "Erro ao listar logs: %s\n", mysql_error(con));
        mysql_close(con);
        return;
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) { 
        fprintf(stderr, "Erro ao obter resultados (logs): %s\n", mysql_error(con));
        mysql_close(con); 
        return; 
    }
    MYSQL_ROW row;
    int num_rows = mysql_num_rows(result);
    limparTela();
    printf("\n--- HISTORICO DE LOGS (Ultimos 50) ---\n");
    if (num_rows == 0) {
        printf("Nenhum registro de log encontrado.\n");
    } else {
        printf("%-5s | %-20s | %-15s | %-50s\n", "ID", "Data/Hora", "Usuario", "Acao");
        printf("--------------------------------------------------------------------------------------------------\n");
        while ((row = mysql_fetch_row(result))) {
            printf("%-5s | %-20s | %-15s | %-50s\n", row[0], row[1], row[3], row[2]); 
        }
        printf("--------------------------------------------------------------------------------------------------\n");
    }
    mysql_free_result(result);
    mysql_close(con);
}

void listarTodosLogs() {
    registrarLog("VISUALIZOU LOGS (TODOS)");
    MYSQL *con = conectar_db();
    if (con == NULL) {
        fprintf(stderr, "\nErro: Nao foi possivel conectar ao banco. Tente novamente mais tarde.\n");
        return; 
    }
    
    // ALTERAÇÃO: ORDER BY l.id_log DESC
    char query[] = "SELECT l.id_log, l.data_hora, l.acao, u.nome_usuario "
                   "FROM log AS l "
                   "JOIN usuario AS u ON l.fk_usuario_id_usuario = u.id_usuario "
                   "ORDER BY l.id_log DESC"; // IDs maiores primeiro

    if (mysql_query(con, query)) {
        fprintf(stderr, "Erro ao listar todos os logs: %s\n", mysql_error(con));
        mysql_close(con);
        return;
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) { 
        fprintf(stderr, "Erro ao obter resultados (todos os logs): %s\n", mysql_error(con));
        mysql_close(con); 
        return; 
    }
    MYSQL_ROW row;
    int num_rows = mysql_num_rows(result);
    limparTela();
    printf("\n--- HISTORICO DE LOGS (COMPLETO) ---\n");
    if (num_rows == 0) {
        printf("Nenhum registro de log encontrado.\n");
    } else {
        printf("%-5s | %-20s | %-15s | %-50s\n", "ID", "Data/Hora", "Usuario", "Acao");
        printf("--------------------------------------------------------------------------------------------------\n");
        while ((row = mysql_fetch_row(result))) {
            printf("%-5s | %-20s | %-15s | %-50s\n", row[0], row[1], row[3], row[2]); 
        }
        printf("--------------------------------------------------------------------------------------------------\n");
    }
    mysql_free_result(result);
    mysql_close(con);
}

/*
 * Lê uma linha inteira do console de forma segura (prevenindo overflow)
 * e limpa o buffer de entrada (stdin).
 */
void lerStringSegura(char *buffer, int tamanho) {
    fgets(buffer, tamanho, stdin);
    
    // Encontra e remove o '\n' que o fgets deixa
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    } else {
        // Se não encontrou '\n', o input foi maior que o buffer.
        // Precisamos limpar o resto da linha do stdin.
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
    }
}

/*
 * Lê um número inteiro de forma segura.
 * Retorna INT_MIN se a entrada for inválida (letras, vazio, etc.)
 */
int lerInteiroSeguro() {
    char buffer[32]; // Buffer suficiente para um int
    lerStringSegura(buffer, 32);
    
    char *end_ptr;
    
    // Se a string está vazia, é inválido
    if (buffer[0] == '\0') {
        return INT_MIN; // Código de erro
    }
    
    long valor = strtol(buffer, &end_ptr, 10);
    
    // Se strtol falhou (end_ptr == buffer) ou
    // se há lixo no final da string (*end_ptr != '\0')
    if (end_ptr == buffer || *end_ptr != '\0') {
        return INT_MIN; // Código de erro
    }
    
    // Checa se o valor está dentro dos limites de um 'int'
    if (valor > INT_MAX || valor < INT_MIN) {
        return INT_MIN; // Código de erro
    }
    
    return (int)valor;
}

/*
 * (MENU DE LOG)
 * Menu para acessar o visualizador de logs.
 */
void menuLogs() {
    int opcao;
    do {
        limparTela();
        printf("\n--- LOGS ---\n");
        printf("1. Listar Logs (Ultimos 50)\n");
        printf("2. Listar Logs (Todos)\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        
        // --- CORREÇÃO APLICADA ---
        opcao = lerInteiroSeguro();

        switch (opcao) {
            case 1: 
                listarLogs(); // A função que lista 50
                pausarTela(); 
                break;
            case 2:
                listarTodosLogs(); // A função que lista tudo
                pausarTela();
                break;
            case 0: 
                printf("Voltando...\n"); 
                break;
            default: 
                printf("Opcao invalida!\n");
                pausarTela();
                break;
        }
    } while (opcao != 0);
}

void listarVinhosSimples(MYSQL *con) {
    // ALTERAÇÃO: ORDER BY v.id_vinho
    char query[] = 
        "SELECT v.id_vinho, v.nome_vinho, v.safra, v.preco, IFNULL(t.nome_tipo, 'N/A') "
        "FROM vinho AS v "
        "LEFT JOIN tipo AS t ON v.fk_tipo_id_tipo = t.id_tipo "
        "WHERE v.ativo = 1 "
        "ORDER BY v.id_vinho"; 

    if (mysql_query(con, query)) {
        fprintf(stderr, "Erro ao listar vinhos (simples): %s\n", mysql_error(con));
        return;
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        fprintf(stderr, "Erro ao obter resultados (simples): %s\n", mysql_error(con));
        return;
    }

    MYSQL_ROW row;
    int num_rows = mysql_num_rows(result);

    printf("\n>>> Lista de Vinhos em Estoque <<<\n");
    
    if (num_rows == 0) {
        printf("Nenhum vinho com estoque disponivel no momento.\n");
    } else {
        printf("%-3s | %-40s | %-5s | %-10s | %-15s\n", 
               "ID", "Nome", "Safra", "Preco", "Tipo");
        printf("--------------------------------------------------------------------------\n");

        while ((row = mysql_fetch_row(result))) {
            printf("%-3s | %-40s | %-5s | %-10s | %-15s\n",
                   row[0] ? row[0] : "N/A",
                   row[1] ? row[1] : "N/A",
                   row[2] ? row[2] : "N/A",
                   row[3] ? row[3] : "N/A",
                   row[4] ? row[4] : "N/A");
        }
        printf("--------------------------------------------------------------------------\n");
    }
    mysql_free_result(result);
}

/*
 * (NOVA FUNÇÃO AUXILIAR 2)
 * Mostra TODOS os detalhes de um vinho específico.
 * Esta função RECEBE uma conexão e usa Prepared Statements.
 */
void listarDetalhesVinho(MYSQL *con, int id_vinho) {
    MYSQL_STMT *stmt;
    MYSQL_BIND bind_in[1];
    MYSQL_BIND bind_out[8]; // 8 colunas que vamos buscar
    char query[] = 
        "SELECT "
        "   v.id_vinho, v.nome_vinho, v.safra, v.preco, v.quantidade, "
        "   IFNULL(t.nome_tipo, 'N/A'), "
        "   IFNULL(p.nome_pais, 'N/A'), "
        "   IFNULL(GROUP_CONCAT(u.nome_uva SEPARATOR ', '), 'N/A') AS uvas "
        "FROM vinho AS v "
        "LEFT JOIN tipo AS t ON v.fk_tipo_id_tipo = t.id_tipo "
        "LEFT JOIN paisorigem AS p ON v.fk_paisorigem_id_paisorigem = p.id_paisorigem "
        "LEFT JOIN composto_por AS cp ON v.id_vinho = cp.fk_vinho_id_vinho "
        "LEFT JOIN uva AS u ON cp.fk_uva_id_uva = u.id_uva "
        "WHERE v.ativo = 1 AND v.id_vinho = ? " // Filtra pelo ID
        "GROUP BY v.id_vinho";

    // Buffers para armazenar os resultados
    char res_id[10], res_nome[151], res_safra[6], res_preco[20], res_qtd[10];
    char res_tipo[51], res_pais[101], res_uvas[1024];
    unsigned long length[8]; // Para strings

    stmt = mysql_stmt_init(con);
    // Erro no INIT
    if (!stmt) {
        fprintf(stderr, " (detalhes) mysql_stmt_init() falhou\n");
        return; // Não podemos fechar o stmt, pois ele é NULO
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, " (detalhes) prepare falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    // Bind do parâmetro de ENTRADA (o ID)
    memset(bind_in, 0, sizeof(bind_in));
    bind_in[0].buffer_type = MYSQL_TYPE_LONG;
    bind_in[0].buffer = (char *)&id_vinho;
    
    // Erro no BIND PARAM
    if (mysql_stmt_bind_param(stmt, bind_in)) {
        fprintf(stderr, " (detalhes) mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }
    
    // Erro no EXECUTE
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " (detalhes) mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    // Bind dos parâmetros de SAÍDA (os resultados)
    memset(bind_out, 0, sizeof(bind_out));
    bind_out[0].buffer_type = MYSQL_TYPE_STRING; bind_out[0].buffer = res_id; bind_out[0].buffer_length = 10; bind_out[0].length = &length[0];
    bind_out[1].buffer_type = MYSQL_TYPE_STRING; bind_out[1].buffer = res_nome; bind_out[1].buffer_length = 151; bind_out[1].length = &length[1];
    bind_out[2].buffer_type = MYSQL_TYPE_STRING; bind_out[2].buffer = res_safra; bind_out[2].buffer_length = 6; bind_out[2].length = &length[2];
    bind_out[3].buffer_type = MYSQL_TYPE_STRING; bind_out[3].buffer = res_preco; bind_out[3].buffer_length = 20; bind_out[3].length = &length[3];
    bind_out[4].buffer_type = MYSQL_TYPE_STRING; bind_out[4].buffer = res_qtd; bind_out[4].buffer_length = 10; bind_out[4].length = &length[4];
    bind_out[5].buffer_type = MYSQL_TYPE_STRING; bind_out[5].buffer = res_tipo; bind_out[5].buffer_length = 51; bind_out[5].length = &length[5];
    bind_out[6].buffer_type = MYSQL_TYPE_STRING; bind_out[6].buffer = res_pais; bind_out[6].buffer_length = 101; bind_out[6].length = &length[6];
    bind_out[7].buffer_type = MYSQL_TYPE_STRING; bind_out[7].buffer = res_uvas; bind_out[7].buffer_length = 1024; bind_out[7].length = &length[7];
    
    // Erro no BIND RESULT
    if (mysql_stmt_bind_result(stmt, bind_out)) {
        fprintf(stderr, " (detalhes) mysql_stmt_bind_result() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }
    
    // Erro no STORE RESULT
    if (mysql_stmt_store_result(stmt)) {
        fprintf(stderr, " (detalhes) mysql_stmt_store_result() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    // Tenta buscar o resultado
    if (mysql_stmt_fetch(stmt) == 0) {
        // Sucesso, imprime os detalhes
        limparTela();
        printf("\n--- DETALHES DO VINHO ID: %s ---\n", res_id);
        printf("Nome: \t\t%s\n", res_nome);
        printf("Safra: \t\t%s\n", res_safra);
        printf("Preço: \t\tR$ %s\n", res_preco);
        printf("Estoque: \t%s unidades\n", res_qtd);
        printf("Tipo: \t\t%s\n", res_tipo);
        printf("País: \t\t%s\n", res_pais);
        printf("Uvas: \t\t%s\n", res_uvas);
        printf("----------------------------------------\n");
    } else {
        // Se o fetch falhar (não encontrou linhas)
        printf("\nErro: Vinho com ID %d nao foi encontrado ou sem estoque.\n", id_vinho);
    }
    
    mysql_stmt_close(stmt);
}

/*
 * Verifica se já existe um vinho com este nome.
 * id_ignorar: Passar 0 para CRIAÇÃO. Passar o ID do vinho para ATUALIZAÇÃO.
 * Retorna 1 se já existe (ERRO), 0 se está livre (OK).
 */
int verificar_duplicidade_nome_vinho(MYSQL *con, const char *nome, int id_ignorar) {
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];
    // A query verifica se existe alguem com esse nome, MAS que não seja o proprio ID que estamos editando
    char query[] = "SELECT id_vinho FROM vinho WHERE nome_vinho = ? AND id_vinho != ?";
    
    stmt = mysql_stmt_init(con);
    if (!stmt) return 0; // Se falhar init, deixamos passar (fail-open) ou tratamos erro
    
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        mysql_stmt_close(stmt); return 0;
    }
    
    memset(bind, 0, sizeof(bind));
    // Param 1: Nome
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)nome;
    bind[0].buffer_length = strlen(nome);
    
    // Param 2: ID a ignorar (na criacao é 0, entao nao ignora ninguem existente)
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *)&id_ignorar;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        mysql_stmt_close(stmt); return 0;
    }
    
    if (mysql_stmt_execute(stmt)) {
        mysql_stmt_close(stmt); return 0;
    }
    
    if (mysql_stmt_store_result(stmt)) {
        mysql_stmt_close(stmt); return 0;
    }
    
    // Se num_rows > 0, significa que JÁ EXISTE um vinho com esse nome
    int existe = (mysql_stmt_num_rows(stmt) > 0);
    
    mysql_stmt_close(stmt);
    return existe;
}

//funcao para limpeza do terminal
void limparTela() {
    #ifdef _WIN32
        system("cls");
    #else
        // Assume POSIX (Linux, macOS)
        system("clear");
    #endif
}

//funcao para pausar o terminal
void pausarTela() {
    printf("\nPressione Enter para continuar...");
    getchar();
}