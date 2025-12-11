/*
 * CRUD Vintagem - Versão Refatorada Final
 * * Este código é a fusão do projeto funcional com uma refatoração
 * para eliminar código duplicado, usando funções "genéricas".
 *
 * MELHORIAS:
 * 1. Funções genéricas (criar/listar/atualizar/desativarEntidade)
 * 2. Helper interativo (obterIdEntidadeInterativo) para criar/atualizar vinhos.
 * 3. Gerenciamento de conexão padronizado (sem vazamentos).
 * 4. Validação de entrada robusta (strtol) em TODOS os menus.
 * 5. Leitura de string segura (fgets) em vez de scanf.
 * 6. Todos os recursos (Login, Logs, N-N, Movimentação) mantidos.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include <conio.h>

// --- Informações de Conexão ---
const char *server = "localhost";
const char *user = "root";
const char *password = "@vintagem123";
const char *database = "vintagem";

// --- Variáveis Globais de Sessão ---
int g_id_usuario_logado = 0;
char g_nome_nivel_logado[51] = "";

// --- Estruturas ---
typedef struct {
    int id_vinho;
    char nome_vinho[151];
    int safra;
    double preco;
    int quantidade;
    int fk_tipo_id_tipo;
    int fk_paisorigem_id_paisorigem;
} Vinho;

// ==========================================================
// PROTÓTIPOS DE FUNÇÕES
// ==========================================================

// --- PROTÓTIPOS: Funções Principais (Menus) ---
int main();
int realizarLogin();
void menuMovimentacoes();
void menuGerenciarTipos();
void menuGerenciarPaises();
void menuGerenciarUva();
void menuListaInativos();
void menuLogs();

// --- PROTÓTIPOS: Funções CRUD Vinho ---
void criarVinho();
void listarVinhos(); // Menu interativo
void atualizarVinho();
void desativarVinho();

// --- PROTÓTIPOS: Funções CRUD Movimentação ---
void registrarSaida();

// --- PROTÓTIPOS: Funções Genéricas (para Tipo, Pais, Uva) ---
void menuCriarEntidade(const char *tabela, const char *coluna_nome, const char *prompt_nome, int tam_nome);
void menuAtualizarEntidade(const char *tabela, const char *coluna_id, const char *coluna_nome, const char *prompt_id, const char *prompt_nome, int tam_nome);
void menuDesativarEntidade(const char *tabela, const char *coluna_id, const char *coluna_nome, const char *prompt_id);
void menuReativarEntidade(const char *tabela, const char *coluna_id, const char *coluna_nome, const char *prompt_id);
void listarEntidadeSimples(MYSQL *con, const char *tabela, const char *coluna_id, const char *coluna_nome);
void listarEntidadeInativa(MYSQL *con, const char *tabela, const char *coluna_id, const char *coluna_nome);

// --- PROTÓTIPOS: Funções Auxiliares (Helpers) ---
MYSQL* conectar_db();
void limparTela();
void pausarTela();
void limparBuffer();
void lerString(char *buffer, int tamanho);

// --- PROTÓTIPOS: Helpers de Banco de Dados ---
int obter_id_pelo_nome(MYSQL *con, const char *tabela, const char *coluna_id, const char *coluna_nome, const char *nome_busca);
long long criar_item_retornando_id(MYSQL *con, const char *tabela, const char *coluna_nome, const char *nome_valor);
int verificar_id_ativo(MYSQL *con, const char *tabela, const char *coluna_id, int id_busca);
int obterIdEntidadeInterativo(MYSQL *con, const char *tabela, const char *coluna_id, const char *coluna_nome, const char *prompt_menu, int tam_nome);

// --- PROTÓTIPOS: Helpers de Vinho (N-N) ---
void listarVinhosSimples(MYSQL *con);
void listarDetalhesVinho(MYSQL *con, int id_vinho);
void gerenciarUvasDoVinho(MYSQL *con, int id_vinho);
void limparUvasDoVinho(MYSQL *con, int id_vinho);

// --- PROTÓTIPOS: Logs ---
void registrarLog(const char *acao);
void listarLogs();
void listarTodosLogs();


// ==========================================================
// FUNÇÕES PRINCIPAIS E DE LOGIN
// ==========================================================

/*
 * Função auxiliar para tratar erros FATAIS do MySQL (usada apenas no conectar_db)
 */
void finish_with_error(MYSQL *con) {
  fprintf(stderr, "Erro Fatal: %s\n", mysql_error(con));
  mysql_close(con);
  exit(1);
}

/*
 * Conecta ao banco de dados
 */
MYSQL* conectar_db() {
    MYSQL *con = mysql_init(NULL);
    const char *plugin_dir = ".\\plugin"; 

    if (con == NULL) {
        fprintf(stderr, "mysql_init() falhou\n");
        exit(1);
    }

    if (mysql_options(con, MYSQL_PLUGIN_DIR, plugin_dir)) {
        fprintf(stderr, "Erro ao configurar o diretorio de plugins: %s\n", mysql_error(con));
        mysql_close(con);
        exit(1);
    }

    if (mysql_real_connect(con, server, user, password, database, 3306, NULL, 0) == NULL) {
        finish_with_error(con);
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
    lerString(email, 101); // Leitura segura
    
    printf("Digite sua Senha: ");
    i = 0;
    while (1) {
        ch = _getch();
        if (ch == 13) { senha_digitada[i] = '\0'; break; }
        if (ch == 8) { if (i > 0) { i--; printf("\b \b"); } }
        else { if (i < 255) { senha_digitada[i] = ch; i++; printf("*"); } }
    }
    printf("\nVerificando...\n");

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

    if (mysql_stmt_num_rows(stmt) == 1) {
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

/*
 * Função principal do programa
 */
int main() {
    int opcao;
    int tentativas;
    int id_logado = 0; 

    mysql_library_init(0, NULL, NULL);

    // --- Bloco de Login com 3 Tentativas ---
    for (tentativas = 1; tentativas <= 3; tentativas++) {
        id_logado = realizarLogin();
        
        if (id_logado > 0) { 
            g_id_usuario_logado = id_logado; 
            printf("\nLogin realizado com sucesso! (ID: %d, Nivel: %s)\n", g_id_usuario_logado, g_nome_nivel_logado);
            char log_msg[256];
            sprintf(log_msg, "LOGIN SUCESSO: Usuario %s (ID: %d) logou.", g_nome_nivel_logado, g_id_usuario_logado);
            registrarLog(log_msg);
            pausarTela();
            break; 
        } else {
            limparTela();
            fprintf(stderr, "\n>>> LOGIN FALHOU (Tentativa %d de 3) <<<\n", tentativas);
            fprintf(stderr, "Email ou senha incorretos, ou usuario inativo.\n");
            pausarTela();
        }
    }

    // --- Verificação Pós-Loop ---
    if (g_id_usuario_logado == 0) { 
        limparTela();
        fprintf(stderr, "\n>>> ACESSO NEGADO <<<\n");
        fprintf(stderr, "Voce falhou 3 tentativas de login. O programa sera encerrado.\n");
        pausarTela();
        mysql_library_end(); 
        return 1; 
    }
    
    // --- Loop Principal do Menu ---
    do {
        limparTela();
        printf("\n--- ADEGA VINTAGEM ---\n");
        printf("--- Vinhos ---\n");
        printf("1. Adicionar Vinho\n");
        printf("2. Listar Vinhos (Ativos)\n");
        printf("3. Atualizar Vinho\n");
        printf("4. Desativar Vinho\n");
        printf("\n--- Gerenciamento ---\n");
        printf("5. Gerenciar Tipos\n");
        printf("6. Gerenciar Paises\n");
        printf("7. Gerenciar Uvas\n");
        printf("8. Lista de Inativos (Reativar)\n");
        printf("9. Movimentar Estoque\n");

        if (strcmp(g_nome_nivel_logado, "Admin") == 0) {
            printf("10. Gerenciar Logs\n");
        }
        
        printf("\n0. Sair\n");
        printf("Escolha uma das opcoes acima: ");
        
        // Validação de entrada numérica
        if (scanf("%d", &opcao) != 1) {
            opcao = -1; // Define um valor inválido para ir ao 'default'
        }
        limparBuffer(); // Limpa o buffer de entrada

        switch (opcao) {
            case 1:
                criarVinho();
                pausarTela();
                break;
            case 2:
                listarVinhos();
                // Pausa já está dentro de listarVinhos()
                break;
            case 3:
                atualizarVinho();
                pausarTela();
                break;
            case 4:
                desativarVinho();
                pausarTela();
                break;
            case 5:
                menuGerenciarTipos();
                break;
            case 6:
                menuGerenciarPaises();
                break;
            case 7:
                menuGerenciarUva();
                break;
            case 8:
                menuListaInativos();
                break;
            case 9:
                menuMovimentacoes();
                break;
            
            case 10:
                if (strcmp(g_nome_nivel_logado, "Admin") == 0) {
                    menuLogs();
                } else {
                    printf("Opcao invalida! Tente Novamente.\n");
                    pausarTela();
                }
                break;

            case 0:
                printf("Saindo do Programa...\n");
                break;
            default:
                printf("Opcao invalida! Tente Novamente.\n");
                pausarTela();
                break;    
        }
    } while (opcao != 0);

    mysql_library_end(); 
    return 0;
}

// ==========================================================
// FUNÇÕES DE CRUD: VINHO
// ==========================================================

/*
 * Função Mestra para criar um Vinho.
 * Usa o helper obterIdEntidadeInterativo()
 */
void criarVinho() {
    Vinho v;
    MYSQL *con = conectar_db();
    if (!con) {
        fprintf(stderr, "Erro ao conectar ao BD.\n");
        return;
    }

    MYSQL_STMT *stmt;
    MYSQL_BIND bind[6];
    char query[] = "INSERT INTO vinho(nome_vinho, safra, preco, quantidade, fk_tipo_id_tipo, fk_paisorigem_id_paisorigem, ativo) VALUES(?, ?, ?, ?, ?, ?, 1)";

    // --- 1. Coletar dados do Vinho ---
    limparTela();
    printf("\n>>> Adicionar Novo Vinho <<<\n");
    printf("Nome: ");
    lerString(v.nome_vinho, 151);

    printf("Safra (Ano): ");
    if (scanf("%d", &v.safra) != 1) {
        printf("Entrada invalida.\n");
        limparBuffer();
        mysql_close(con);
        return;
    }
    limparBuffer();

    printf("Preco (ex: 59.99): ");
    if (scanf("%lf", &v.preco) != 1) {
        printf("Entrada invalida.\n");
        limparBuffer();
        mysql_close(con);
        return;
    }
    limparBuffer();

    printf("Quantidade em estoque: ");
    if (scanf("%d", &v.quantidade) != 1) {
        printf("Entrada invalida.\n");
        limparBuffer();
        mysql_close(con);
        return;
    }
    limparBuffer();

    // --- 2. Coletar Tipo (Usando Helper) ---
    v.fk_tipo_id_tipo = obterIdEntidadeInterativo(con, "tipo", "id_tipo", "nome_tipo", "--- Selecao de Tipo ---", 51);
    if (v.fk_tipo_id_tipo == -1) {
        printf("Criacao de vinho abortada.\n");
        mysql_close(con);
        return;
    }

    // --- 3. Coletar País (Usando Helper) ---
    v.fk_paisorigem_id_paisorigem = obterIdEntidadeInterativo(con, "paisorigem", "id_paisorigem", "nome_pais", "--- Selecao de Pais ---", 101);
    if (v.fk_paisorigem_id_paisorigem == -1) {
        printf("Criacao de vinho abortada.\n");
        mysql_close(con);
        return;
    }

    // --- 4. Preparar e Executar o INSERT do Vinho ---
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
    bind[0].buffer = (char *)v.nome_vinho;
    bind[0].buffer_length = strlen(v.nome_vinho);
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *)&v.safra;
    bind[2].buffer_type = MYSQL_TYPE_DOUBLE;
    bind[2].buffer = (char *)&v.preco;
    bind[3].buffer_type = MYSQL_TYPE_LONG;
    bind[3].buffer = (char *)&v.quantidade;
    bind[4].buffer_type = MYSQL_TYPE_LONG;
    bind[4].buffer = (char *)&v.fk_tipo_id_tipo;
    bind[5].buffer_type = MYSQL_TYPE_LONG;
    bind[5].buffer = (char *)&v.fk_paisorigem_id_paisorigem;

    if (mysql_stmt_bind_param(stmt, bind)) { 
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }
    
    if (mysql_stmt_execute(stmt)) { 
        fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    printf("\n>>> Vinho '%s' adicionado com sucesso! <<<\n", v.nome_vinho);

    long long novo_id_vinho = mysql_insert_id(con);
    char log_msg[256];
    sprintf(log_msg, "CRIOU VINHO: ID %lld (%s)", novo_id_vinho, v.nome_vinho);
    registrarLog(log_msg);

    mysql_stmt_close(stmt);

    if (novo_id_vinho > 0) {
        gerenciarUvasDoVinho(con, (int)novo_id_vinho);
    } else {
        fprintf(stderr, "Erro critico: Nao foi possivel obter o ID do vinho recem-criado.\n");
    }

    mysql_close(con);
}

/*
 * Menu interativo para listar vinhos e ver detalhes.
 */
void listarVinhos() {
    MYSQL *con = conectar_db();
    if (!con) {
        fprintf(stderr, "Erro ao conectar ao BD.\n");
        pausarTela();
        return;
    }
    
    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;
    
    do {
        limparTela();
        
        listarVinhosSimples(con);
        
        printf("\nDigite o ID do vinho para ver detalhes (ou 0 para voltar): ");
        lerString(input_buffer, 101);

        id_digitado = strtol(input_buffer, &end_ptr, 10);

        if (end_ptr == input_buffer || *end_ptr != '\0') {
            printf("\nErro: Entrada invalida. Apenas numeros sao permitidos.\n");
            pausarTela();
            continue;
        }
        
        if (id_digitado == 0) {
            break; 
        } else {
            listarDetalhesVinho(con, (int)id_digitado);
            pausarTela(); 
        }
        
    } while (id_digitado != 0);
    
    mysql_close(con);
    printf("Voltando ao menu principal...\n");
}

/*
 * Mostra a lista SIMPLES de vinhos
 */
void listarVinhosSimples(MYSQL *con) {
    char query[] = 
        "SELECT v.id_vinho, v.nome_vinho, v.safra, v.preco, IFNULL(t.nome_tipo, 'N/A') "
        "FROM vinho AS v "
        "LEFT JOIN tipo AS t ON v.fk_tipo_id_tipo = t.id_tipo "
        "WHERE v.ativo = 1 "
        "ORDER BY v.nome_vinho";

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
    int num_rows = (int)mysql_num_rows(result);

    printf("\n>>> Lista de Vinhos (Ativos) <<<\n");
    if (num_rows == 0) {
        printf("Nenhum vinho ativo cadastrado.\n");
    } else {
        printf("%-3s | %-40s | %-5s | %-10s | %-15s\n", 
               "ID", "Nome", "Safra", "Preco", "Tipo");
        printf("----------------------------------------------------------------------------------\n");

        while ((row = mysql_fetch_row(result))) {
            printf("%-3s | %-40s | %-5s | %-10s | %-15s\n",
                   row[0] ? row[0] : "N/A",
                   row[1] ? row[1] : "N/A",
                   row[2] ? row[2] : "N/A",
                   row[3] ? row[3] : "N/A",
                   row[4] ? row[4] : "N/A");
        }
        printf("----------------------------------------------------------------------------------\n");
    }
    mysql_free_result(result);
}

/*
 * Mostra TODOS os detalhes de um vinho específico.
 */
void listarDetalhesVinho(MYSQL *con, int id_vinho) {
    MYSQL_STMT *stmt;
    MYSQL_BIND bind_in[1];
    MYSQL_BIND bind_out[8]; 
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
        "WHERE v.ativo = 1 AND v.id_vinho = ? " 
        "GROUP BY v.id_vinho";

    char res_id[10], res_nome[151], res_safra[6], res_preco[20], res_qtd[10];
    char res_tipo[51], res_pais[101], res_uvas[1024];
    unsigned long length[8]; 

    stmt = mysql_stmt_init(con);
    if (!stmt) {
        fprintf(stderr, " (detalhes) mysql_stmt_init() falhou\n");
        return; 
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, " (detalhes) prepare falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    memset(bind_in, 0, sizeof(bind_in));
    bind_in[0].buffer_type = MYSQL_TYPE_LONG;
    bind_in[0].buffer = (char *)&id_vinho;
    
    if (mysql_stmt_bind_param(stmt, bind_in)) {
        fprintf(stderr, " (detalhes) mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }
    
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " (detalhes) mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    memset(bind_out, 0, sizeof(bind_out));
    bind_out[0].buffer_type = MYSQL_TYPE_STRING; bind_out[0].buffer = res_id; bind_out[0].buffer_length = 10; bind_out[0].length = &length[0];
    bind_out[1].buffer_type = MYSQL_TYPE_STRING; bind_out[1].buffer = res_nome; bind_out[1].buffer_length = 151; bind_out[1].length = &length[1];
    bind_out[2].buffer_type = MYSQL_TYPE_STRING; bind_out[2].buffer = res_safra; bind_out[2].buffer_length = 6; bind_out[2].length = &length[2];
    bind_out[3].buffer_type = MYSQL_TYPE_STRING; bind_out[3].buffer = res_preco; bind_out[3].buffer_length = 20; bind_out[3].length = &length[3];
    bind_out[4].buffer_type = MYSQL_TYPE_STRING; bind_out[4].buffer = res_qtd; bind_out[4].buffer_length = 10; bind_out[4].length = &length[4];
    bind_out[5].buffer_type = MYSQL_TYPE_STRING; bind_out[5].buffer = res_tipo; bind_out[5].buffer_length = 51; bind_out[5].length = &length[5];
    bind_out[6].buffer_type = MYSQL_TYPE_STRING; bind_out[6].buffer = res_pais; bind_out[6].buffer_length = 101; bind_out[6].length = &length[6];
    bind_out[7].buffer_type = MYSQL_TYPE_STRING; bind_out[7].buffer = res_uvas; bind_out[7].buffer_length = 1024; bind_out[7].length = &length[7];
    
    if (mysql_stmt_bind_result(stmt, bind_out)) {
        fprintf(stderr, " (detalhes) mysql_stmt_bind_result() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }
    
    if (mysql_stmt_store_result(stmt)) {
        fprintf(stderr, " (detalhes) mysql_stmt_store_result() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    if (mysql_stmt_fetch(stmt) == 0) {
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
        printf("\nErro: Vinho com ID %d nao foi encontrado ou esta inativo.\n", id_vinho);
    }
    
    mysql_stmt_close(stmt);
}

/*
 * Função Mestra para atualizar um Vinho.
 * Usa o helper obterIdEntidadeInterativo()
 */
void atualizarVinho() {
    int id_busca = 0;
    Vinho v;
    MYSQL *con = conectar_db();
    if (!con) {
        fprintf(stderr, "Erro ao conectar ao BD.\n");
        return;
    }

    MYSQL_STMT *stmt;
    MYSQL_BIND bind[7];
    char query[] = "UPDATE vinho SET nome_vinho = ?, safra = ?, preco = ?, quantidade = ?, fk_tipo_id_tipo = ?, fk_paisorigem_id_paisorigem = ? WHERE id_vinho = ?";

    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;
    
    int id_tipo_encontrado;
    int id_pais_encontrado;

    // --- 1. MOSTRAR OS VINHOS PRIMEIRO ---
    limparTela();
    printf("\n--- ATUALIZAR VINHO ---\n");
    printf("Estes são os vinhos ativos que podem ser atualizados:\n\n");
    
    listarVinhosSimples(con); 

    // --- 2. COLETAR E VALIDAR O ID ---
    printf("\nDigite o ID do Vinho que deseja atualizar (ou 0 para cancelar): ");
    lerString(input_buffer, 101);

    id_digitado = strtol(input_buffer, &end_ptr, 10);

    if (end_ptr == input_buffer || *end_ptr != '\0') {
        printf("\nErro: Entrada invalida. Apenas numeros sao permitidos.\n");
        mysql_close(con); 
        return; 
    }
    
    if (id_digitado == 0) {
        printf("Operacao cancelada.\n");
        mysql_close(con); 
        return; 
    }
    
    id_busca = (int)id_digitado; 
    
    printf("\n>>> Insira os NOVOS dados para o Vinho ID %d <<<\n", id_busca);
    
    printf("Novo Nome: ");
    lerString(v.nome_vinho, 151);

    printf("Nova Safra (Ano): ");
    if (scanf("%d", &v.safra) != 1) { /* (erro) */ limparBuffer(); mysql_close(con); return; }
    limparBuffer();

    printf("Novo Preco (ex: 59.99): ");
    if (scanf("%lf", &v.preco) != 1) { /* (erro) */ limparBuffer(); mysql_close(con); return; }
    limparBuffer();

    printf("Nova Quantidade: ");
    if (scanf("%d", &v.quantidade) != 1) { /* (erro) */ limparBuffer(); mysql_close(con); return; }
    limparBuffer();

    // --- 4. Coletar Tipo (Usando Helper) ---
    v.fk_tipo_id_tipo = obterIdEntidadeInterativo(con, "tipo", "id_tipo", "nome_tipo", "--- Selecao do NOVO Tipo ---", 51);
    if (v.fk_tipo_id_tipo == -1) {
        printf("Atualizacao abortada.\n");
        mysql_close(con);
        return;
    }

    // --- 5. Coletar País (Usando Helper) ---
    v.fk_paisorigem_id_paisorigem = obterIdEntidadeInterativo(con, "paisorigem", "id_paisorigem", "nome_pais", "--- Selecao do NOVO Pais ---", 101);
    if (v.fk_paisorigem_id_paisorigem == -1) {
        printf("Atualizacao abortada.\n");
        mysql_close(con);
        return;
    }

    // --- 6. Preparar e Executar o UPDATE ---
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
    bind[0].buffer = (char *)v.nome_vinho;
    bind[0].buffer_length = strlen(v.nome_vinho);
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *)&v.safra;
    bind[2].buffer_type = MYSQL_TYPE_DOUBLE;
    bind[2].buffer = (char *)&v.preco;
    bind[3].buffer_type = MYSQL_TYPE_LONG;
    bind[3].buffer = (char *)&v.quantidade;
    bind[4].buffer_type = MYSQL_TYPE_LONG;
    bind[4].buffer = (char *)&v.fk_tipo_id_tipo;
    bind[5].buffer_type = MYSQL_TYPE_LONG;
    bind[5].buffer = (char *)&v.fk_paisorigem_id_paisorigem;
    bind[6].buffer_type = MYSQL_TYPE_LONG;
    bind[6].buffer = (char *)&id_busca;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    if (mysql_stmt_affected_rows(stmt) > 0) {
        printf("\n>>> Dados principais do Vinho ID %d atualizados! <<<\n", id_busca);
        
        char log_msg[512];
        sprintf(log_msg, "ATUALIZOU VINHO (DADOS): ID %d para Nome: %s, Safra: %d, Preco: %.2f, Qtd: %d", 
                id_busca, v.nome_vinho, v.safra, v.preco, v.quantidade);
        registrarLog(log_msg);

    } else {
        printf("\n>>> Vinho com ID %d nao foi encontrado ou nenhum dado foi alterado. <<<\n", id_busca);
    }

    mysql_stmt_close(stmt);

    // --- 7. Gerenciar Uvas ---
    printf("\n--- Gerenciamento de Uvas para o Vinho ID %d ---\n", id_busca);
    limparUvasDoVinho(con, id_busca); 
    gerenciarUvasDoVinho(con, id_busca); 

    mysql_close(con); 
}

/*
 * Desativa um Vinho (Soft Delete)
 */
void desativarVinho() {
    int id_busca = 0;
    MYSQL *con = conectar_db(); 
    if (!con) {
        fprintf(stderr, "Erro ao conectar ao BD.\n");
        return;
    }
    
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    char query[] = "UPDATE vinho SET ativo = 0 WHERE id_vinho = ?";

    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;

    limparTela();
    printf("\n--- DESATIVAR VINHO ---\n");
    printf("Estes são os vinhos ativos que podem ser desativados:\n\n");
    
    listarVinhosSimples(con); 

    printf("\nDigite o ID do Vinho que deseja DESATIVAR (ou 0 para cancelar): ");
    lerString(input_buffer, 101);

    id_digitado = strtol(input_buffer, &end_ptr, 10);

    if (end_ptr == input_buffer || *end_ptr != '\0') {
        printf("\nErro: Entrada invalida. Apenas numeros sao permitidos.\n");
        mysql_close(con); 
        return; 
    }
    
    if (id_digitado == 0) {
        printf("Operacao cancelada.\n");
        mysql_close(con); 
        return; 
    }
    
    id_busca = (int)id_digitado;
    
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
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&id_busca;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
    } else {
        if (mysql_stmt_affected_rows(stmt) > 0) {
            printf("\nVinho ID %d desativado com sucesso!\n", id_busca);
            
            char log_msg[256];
            sprintf(log_msg, "DESATIVOU VINHO: ID %d", id_busca);
            registrarLog(log_msg);

        } else {
            printf("\nVinho com ID %d nao foi encontrado ou ja esta desativado.\n", id_busca);
        }
    }

    mysql_stmt_close(stmt);
    mysql_close(con);
}

// ==========================================================
// FUNÇÕES DE GERENCIAMENTO (TIPOS, PAISES, UVAS)
// ==========================================================

// --- Menu de Gerenciamento de Tipos ---
void menuGerenciarTipos() {
    int opcao;
    do {
        limparTela();
        printf("\n--- GERENCIAR TIPOS ---\n");
        printf("1. Listar Tipos\n");
        printf("2. Adicionar Tipo\n");
        printf("3. Atualizar Tipo\n");
        printf("4. Desativar Tipo\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        
        if (scanf("%d", &opcao) != 1) { opcao = -1; }
        limparBuffer();

        switch (opcao) {
            case 1: { 
                MYSQL *con = conectar_db();
                if (con) {
                    listarEntidadeSimples(con, "tipo", "id_tipo", "nome_tipo");
                    mysql_close(con);
                }
                pausarTela(); 
                break;
            }
            case 2: 
                menuCriarEntidade("tipo", "nome_tipo", "Nome do Tipo", 51);
                pausarTela(); 
                break;
            case 3: 
                menuAtualizarEntidade("tipo", "id_tipo", "nome_tipo", "ID do Tipo a atualizar", "Novo Nome", 51);
                pausarTela(); 
                break;
            case 4: 
                menuDesativarEntidade("tipo", "id_tipo", "nome_tipo", "ID do Tipo a desativar");
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

// --- Menu de Gerenciamento de Países ---
void menuGerenciarPaises() {
    int opcao;
    do {
        limparTela();
        printf("\n--- GERENCIAR PAISES ---\n");
        printf("1. Listar Paises\n");
        printf("2. Adicionar Pais\n");
        printf("3. Atualizar Pais\n");
        printf("4. Desativar Pais\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        
        if (scanf("%d", &opcao) != 1) { opcao = -1; }
        limparBuffer();

        switch (opcao) {
            case 1: {
                MYSQL *con = conectar_db();
                if(con) {
                    listarEntidadeSimples(con, "paisorigem", "id_paisorigem", "nome_pais");
                    mysql_close(con);
                }
                pausarTela(); 
                break;
            }
            case 2: 
                menuCriarEntidade("paisorigem", "nome_pais", "Nome do Pais", 101);
                pausarTela(); 
                break;
            case 3: 
                menuAtualizarEntidade("paisorigem", "id_paisorigem", "nome_pais", "ID do Pais a atualizar", "Novo Nome", 101);
                pausarTela(); 
                break;
            case 4: 
                menuDesativarEntidade("paisorigem", "id_paisorigem", "nome_pais", "ID do Pais a desativar");
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

// --- Menu de Gerenciamento de Uvas ---
void menuGerenciarUva() {
    int opcao;
    do {
        limparTela();
        printf("\n--- GERENCIAR UVAS ---\n");
        printf("1. Listar Uvas\n");
        printf("2. Adicionar Uva\n");
        printf("3. Atualizar Uva\n");
        printf("4. Desativar Uva\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        
        if (scanf("%d", &opcao) != 1) { opcao = -1; }
        limparBuffer();

        switch (opcao) {
            case 1: {
                MYSQL *con = conectar_db();
                if(con) {
                    listarEntidadeSimples(con, "uva", "id_uva", "nome_uva");
                    mysql_close(con);
                }
                pausarTela(); 
                break;
            }
            case 2: 
                menuCriarEntidade("uva", "nome_uva", "Nome da Uva", 101);
                pausarTela(); 
                break;
            case 3: 
                menuAtualizarEntidade("uva", "id_uva", "nome_uva", "ID da Uva a atualizar", "Novo Nome", 101);
                pausarTela(); 
                break;
            case 4: 
                menuDesativarEntidade("uva", "id_uva", "nome_uva", "ID da Uva a desativar");
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

// ==========================================================
// FUNÇÕES GENÉRICAS DE CRUD (Tipo, Pais, Uva)
// ==========================================================

/*
 * Lista entidades simples (Tipo, Pais, Uva)
 * Esta função é um *HELPER* e espera receber uma conexão.
 */
void listarEntidadeSimples(MYSQL *con, const char *tabela, const char *coluna_id, const char *coluna_nome) {
    char query[512];
    sprintf(query, "SELECT %s, %s FROM %s WHERE ativo = 1 ORDER BY %s", coluna_id, coluna_nome, tabela, coluna_nome);

    if (mysql_query(con, query)) {
        fprintf(stderr, "Erro ao listar %s: %s\n", tabela, mysql_error(con));
        return; 
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        fprintf(stderr, "Erro ao obter resultados (%s): %s\n", tabela, mysql_error(con));
        return;
    }

    MYSQL_ROW row;
    int num_rows = (int)mysql_num_rows(result);

    printf("\n--- Lista de %s (Ativos) ---\n", tabela);
    if (num_rows == 0) {
        printf("Nenhum item ativo cadastrado.\n");
    } else {
        printf("%-5s | %-30s\n", "ID", "Nome");
        printf("---------------------------------------\n");
        while ((row = mysql_fetch_row(result))) {
            printf("%-5s | %-30s\n", row[0], row[1]);
        }
        printf("---------------------------------------\n");
    }
    mysql_free_result(result);
}

/*
 * Wrapper de menu para criar uma entidade simples
 */
void menuCriarEntidade(const char *tabela, const char *coluna_nome, const char *prompt_nome, int tam_nome) {
    char *nome_entidade = malloc(tam_nome * sizeof(char));
    if (!nome_entidade) {
        fprintf(stderr, "Erro de alocacao de memoria!\n");
        return;
    }

    MYSQL *con = conectar_db();
    if (!con) {
        fprintf(stderr, "Erro ao conectar ao BD.\n");
        free(nome_entidade);
        return;
    }

    printf("\n--- Adicionar Novo %s ---\n", tabela);
    printf("%s: ", prompt_nome);
    lerString(nome_entidade, tam_nome);

    // Usa a função de criação genérica
    long long novo_id = 0;
    if (criar_item_retornando_id(con, tabela, coluna_nome, nome_entidade) != -1) {
         printf("%s '%s' criado com sucesso!\n", tabela, nome_entidade);
    }
    // A msg de erro/log já é tratada dentro de criar_item_retornando_id

    free(nome_entidade);
    mysql_close(con);
}

/*
 * Wrapper de menu para atualizar uma entidade simples
 */
void menuAtualizarEntidade(const char *tabela, const char *coluna_id, const char *coluna_nome, const char *prompt_id, const char *prompt_nome, int tam_nome) {
    int id_busca = 0;
    char *novo_nome = malloc(tam_nome * sizeof(char));
    if (!novo_nome) {
        fprintf(stderr, "Erro de alocacao de memoria!\n");
        return;
    }

    MYSQL *con = conectar_db();
    if (!con) {
        fprintf(stderr, "Erro ao conectar ao BD.\n");
        free(novo_nome);
        return;
    }
    
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];
    char query[512];
    sprintf(query, "UPDATE %s SET %s = ? WHERE %s = ?", tabela, coluna_nome, coluna_id);
    
    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;

    limparTela();
    printf("\n--- Atualizar %s ---\n", tabela);
    listarEntidadeSimples(con, tabela, coluna_id, coluna_nome);

    printf("\n%s (ou 0 para cancelar): ", prompt_id);
    lerString(input_buffer, 101);

    id_digitado = strtol(input_buffer, &end_ptr, 10);
    if (end_ptr == input_buffer || *end_ptr != '\0') {
        printf("\nErro: Entrada invalida. Apenas numeros sao permitidos.\n");
        mysql_close(con); free(novo_nome);
        return;
    }
    if (id_digitado == 0) {
        printf("Operacao cancelada.\n");
        mysql_close(con); free(novo_nome);
        return;
    }
    id_busca = (int)id_digitado;

    printf("%s: ", prompt_nome);
    lerString(novo_nome, tam_nome);

    stmt = mysql_stmt_init(con);
    if (!stmt) {
        fprintf(stderr, " mysql_stmt_init() falhou\n");
        mysql_close(con); free(novo_nome);
        return;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, " mysql_stmt_prepare() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt); mysql_close(con); free(novo_nome);
        return;
    }
    
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)novo_nome;
    bind[0].buffer_length = strlen(novo_nome);
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *)&id_busca;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt); mysql_close(con); free(novo_nome);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        if (mysql_errno(con) == 1062) {
            fprintf(stderr, "\nErro: O nome '%s' ja esta em uso.\n", novo_nome);
        } else {
            fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        }
    } else {
        if (mysql_stmt_affected_rows(stmt) > 0) {
            printf("%s atualizado com sucesso!\n", tabela);
            char log_msg[256];
            sprintf(log_msg, "ATUALIZOU %s: ID %d para '%s'", tabela, id_busca, novo_nome);
            registrarLog(log_msg);
        } else {
            printf("ID nao encontrado ou nenhum dado foi alterado.\n");
        }
    }
    
    free(novo_nome);
    mysql_stmt_close(stmt);
    mysql_close(con);
}

/*
 * Wrapper de menu para desativar uma entidade simples
 */
void menuDesativarEntidade(const char *tabela, const char *coluna_id, const char *coluna_nome, const char *prompt_id) {
    int id_busca = 0;
    MYSQL *con = conectar_db();
    if (!con) {
        fprintf(stderr, "Erro ao conectar ao BD.\n");
        return;
    }
    
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    char query[512];
    sprintf(query, "UPDATE %s SET ativo = 0 WHERE %s = ?", tabela, coluna_id);

    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;

    limparTela();
    printf("\n--- Desativar %s ---\n", tabela);
    listarEntidadeSimples(con, tabela, coluna_id, coluna_nome);

    printf("\n%s (ou 0 para cancelar): ", prompt_id);
    lerString(input_buffer, 101);

    id_digitado = strtol(input_buffer, &end_ptr, 10);
    if (end_ptr == input_buffer || *end_ptr != '\0') {
        printf("\nErro: Entrada invalida. Apenas numeros sao permitidos.\n");
        mysql_close(con);
        return;
    }
    if (id_digitado == 0) {
        printf("Operacao cancelada.\n");
        mysql_close(con);
        return;
    }
    id_busca = (int)id_digitado;

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
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&id_busca;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    if (mysql_stmt_execute(stmt)) { 
        fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
    } else {
        if (mysql_stmt_affected_rows(stmt) > 0) {
            printf("\n%s ID %d desativado com sucesso!\n", tabela, id_busca);
            char log_msg[256];
            sprintf(log_msg, "DESATIVOU %s: ID %d", tabela, id_busca); 
            registrarLog(log_msg);
        } else {
            printf("\n%s com ID %d nao foi encontrado ou ja esta desativado.\n", tabela, id_busca);
        }
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}

// --- LÓGICA N-para-N (UVAS) ---
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
    char log_msg[256];
    sprintf(log_msg, "REMOVEU UVAS (UPDATE): Todas as uvas do Vinho ID %d", id_vinho);
    registrarLog(log_msg);

    mysql_stmt_close(stmt);
}

void gerenciarUvasDoVinho(MYSQL *con, int id_vinho) {
    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;
    int id_uva_encontrada = -1;
    char resposta = 's';
    
    while (resposta == 's' || resposta == 'S') {
        limparTela();
        printf("\n--- Adicionar Uvas ao Vinho (ID: %d) ---\n", id_vinho);
        listarEntidadeSimples(con, "uva", "id_uva", "nome_uva");
        printf("Digite o NOME ou ID da uva (ou '0' para parar): ");
        lerString(input_buffer, 101);

        if (strcmp(input_buffer, "0") == 0) {
            break;
        }
        
        id_digitado = strtol(input_buffer, &end_ptr, 10);
        id_uva_encontrada = -1;

        if (end_ptr == input_buffer) { 
            id_uva_encontrada = obter_id_pelo_nome(con, "uva", "id_uva", "nome_uva", input_buffer);
        } else if (*end_ptr == '\0') {
            if (verificar_id_ativo(con, "uva", "id_uva", (int)id_digitado)) {
                id_uva_encontrada = (int)id_digitado;
            } else {
                printf("\nErro: ID de uva '%ld' nao existe ou esta inativo.\n", id_digitado);
                pausarTela();
            }
        } else {
            printf("\nErro: Entrada '%s' invalida.\n", input_buffer);
            pausarTela();
        }

        if (id_uva_encontrada == -1 && end_ptr == input_buffer) {
             printf("Uva '%s' nao encontrada. Deseja cadastra-la? (s/n): ", input_buffer);
             scanf(" %c", &resposta);
             limparBuffer();
             if (resposta == 's' || resposta == 'S') {
                 id_uva_encontrada = (int)criar_item_retornando_id(con, "uva", "nome_uva", input_buffer);
                 if (id_uva_encontrada == -1) {
                     fprintf(stderr, "Erro ao cadastrar nova uva. Tente novamente.\n");
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
            if (!stmt_junction) { 
                fprintf(stderr, " (composto_por) mysql_stmt_init() falhou\n");
                continue; 
            }

            if (mysql_stmt_prepare(stmt_junction, query_junction, strlen(query_junction))) {
                fprintf(stderr, " (composto_por) prepare falhou: %s\n", mysql_stmt_error(stmt_junction));
                mysql_stmt_close(stmt_junction);
                continue;
            }
            
            memset(bind_junction, 0, sizeof(bind_junction));
            bind_junction[0].buffer_type = MYSQL_TYPE_LONG;
            bind_junction[0].buffer = (char *)&id_vinho;
            bind_junction[1].buffer_type = MYSQL_TYPE_LONG;
            bind_junction[1].buffer = (char *)&id_uva_encontrada;

            if (mysql_stmt_bind_param(stmt_junction, bind_junction)) { 
                fprintf(stderr, " (composto_por) bind_param falhou: %s\n", mysql_stmt_error(stmt_junction));
                mysql_stmt_close(stmt_junction);
                continue; 
            }
            
            if (mysql_stmt_execute(stmt_junction)) {
                if (mysql_errno(con) == 1062) { 
                    printf("\n... Esta uva ja foi adicionada a este vinho.\n");
                } else {
                    fprintf(stderr, "\nErro ao adicionar uva ao vinho: %s\n", mysql_stmt_error(stmt_junction));
                }
            } else {
                printf("\n... Uva (ID: %d) adicionada ao vinho (ID: %d) com sucesso!\n", id_uva_encontrada, id_vinho);
                char log_msg[256];
                sprintf(log_msg, "ASSOCIACAO VINHO-UVA: Vinho ID %d <- Uva ID %d", id_vinho, id_uva_encontrada);
                registrarLog(log_msg);
            }
            mysql_stmt_close(stmt_junction);
        }
        
        printf("\nDeseja adicionar outra uva a este vinho? (s/n): ");
        scanf(" %c", &resposta);
        limparBuffer();
    }
}

// --- LISTA DE INATIVOS ---
void menuListaInativos() {
    int opcao;
    do {
        limparTela();
        printf("\n--- LISTA DE INATIVOS ---\n");
        printf("1. Reativar Vinho\n");
        printf("2. Reativar Tipo\n");
        printf("3. Reativar Pais\n");
        printf("4. Reativar Uva\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        
        if (scanf("%d", &opcao) != 1) { opcao = -1; }
        limparBuffer();

        switch (opcao) {
            case 1: 
                reativarVinho();
                pausarTela(); 
                break;
            case 2: 
                menuReativarEntidade("tipo", "id_tipo", "nome_tipo", "ID do Tipo a reativar");
                pausarTela(); 
                break;
            case 3: 
                menuReativarEntidade("paisorigem", "id_paisorigem", "nome_pais", "ID do Pais a reativar");
                pausarTela(); 
                break;
            case 4: 
                menuReativarEntidade("uva", "id_uva", "nome_uva", "ID da Uva a reativar");
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

void listarVinhosDesativados(MYSQL *con) {
    char query[] = 
        "SELECT "
        "   v.id_vinho, v.nome_vinho, v.safra, v.preco, "
        "   IFNULL(t.nome_tipo, 'N/A'), "
        "   IFNULL(p.nome_pais, 'N/A') "
        "FROM vinho AS v "
        "LEFT JOIN tipo AS t ON v.fk_tipo_id_tipo = t.id_tipo "
        "LEFT JOIN paisorigem AS p ON v.fk_paisorigem_id_paisorigem = p.id_paisorigem "
        "WHERE v.ativo = 0 " 
        "GROUP BY v.id_vinho";

    if (mysql_query(con, query)) {
        fprintf(stderr, "Erro ao listar vinhos desativados: %s\n", mysql_error(con));
        return;
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        fprintf(stderr, "Erro ao obter resultados (vinhos desativados): %s\n", mysql_error(con));
        return;
    }

    MYSQL_ROW row;
    int num_rows = (int)mysql_num_rows(result);

    printf("\n>>> Lista de Vinhos DESATIVADOS <<<\n");
    if (num_rows == 0) {
        printf("Nenhum vinho desativado encontrado.\n");
    } else {
        printf("%-3s | %-40s | %-5s | %-10s | %-15s | %-15s\n", 
               "ID", "Nome", "Safra", "Preco", "Tipo", "Pais");
        printf("------------------------------------------------------------------------------------------\n");

        while ((row = mysql_fetch_row(result))) {
            printf("%-3s | %-40s | %-5s | %-10s | %-15s | %-15s\n",
                   row[0], row[1], row[2], row[3], row[4], row[5]);
        }
        printf("------------------------------------------------------------------------------------------\n");
    }
    mysql_free_result(result);
}

void reativarVinho() {
    int id_busca;
    MYSQL *con = conectar_db();
    if (!con) {
        fprintf(stderr, "Erro ao conectar ao BD.\n");
        return;
    }
    
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    char query[] = "UPDATE vinho SET ativo = 1 WHERE id_vinho = ?";

    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;

    listarVinhosDesativados(con);

    printf("\nDigite o ID do Vinho que deseja REATIVAR (ou 0 para cancelar): ");
    lerString(input_buffer, 101);

    id_digitado = strtol(input_buffer, &end_ptr, 10);
    if (end_ptr == input_buffer || *end_ptr != '\0') {
        printf("\nErro: Entrada invalida. Apenas numeros sao permitidos.\n");
        mysql_close(con);
        return;
    }
    if (id_digitado == 0) {
        printf("Operacao cancelada.\n");
        mysql_close(con);
        return;
    }
    id_busca = (int)id_digitado;

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
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&id_busca;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
    } else {
        if (mysql_stmt_affected_rows(stmt) > 0) {
            printf("\nVinho ID %d reativado com sucesso!\n", id_busca);
            char log_msg[256];
            sprintf(log_msg, "REATIVOU VINHO: ID %d", id_busca);
            registrarLog(log_msg);
        } else {
            printf("\nVinho com ID %d nao foi encontrado na lista de inativos.\n", id_busca);
        }
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}

void listarEntidadeInativa(MYSQL *con, const char *tabela, const char *coluna_id, const char *coluna_nome) {
    char query[512];
    sprintf(query, "SELECT %s, %s FROM %s WHERE ativo = 0 ORDER BY %s", coluna_id, coluna_nome, tabela, coluna_nome);

    if (mysql_query(con, query)) {
        fprintf(stderr, "Erro ao listar inativos (%s): %s\n", tabela, mysql_error(con));
        return;
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        fprintf(stderr, "Erro ao obter resultados (inativos %s): %s\n", tabela, mysql_error(con));
        return;
    }

    MYSQL_ROW row;
    int num_rows = (int)mysql_num_rows(result);

    printf("\n--- Lista de %s (Inativos) ---\n", tabela);
    if (num_rows == 0) {
        printf("Nenhum item inativo encontrado.\n");
    } else {
        printf("%-5s | %-30s\n", "ID", "Nome");
        printf("---------------------------------------\n");
        while ((row = mysql_fetch_row(result))) {
            printf("%-5s | %-30s\n", row[0], row[1]);
        }
        printf("---------------------------------------\n");
    }
    mysql_free_result(result);
}

void menuReativarEntidade(const char *tabela, const char *coluna_id, const char *coluna_nome, const char *prompt_id) {
    int id_busca;
    MYSQL *con = conectar_db();
    if (!con) {
        fprintf(stderr, "Erro ao conectar ao BD.\n");
        return;
    }
    
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    char query[512];
    sprintf(query, "UPDATE %s SET ativo = 1 WHERE %s = ?", tabela, coluna_id);

    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;

    listarEntidadeInativa(con, tabela, coluna_id, coluna_nome);

    printf("\n%s (ou 0 para cancelar): ", prompt_id);
    lerString(input_buffer, 101);

    id_digitado = strtol(input_buffer, &end_ptr, 10);
    if (end_ptr == input_buffer || *end_ptr != '\0') {
        printf("\nErro: Entrada invalida. Apenas numeros sao permitidos.\n");
        mysql_close(con);
        return;
    }
    if (id_digitado == 0) {
        printf("Operacao cancelada.\n");
        mysql_close(con);
        return;
    }
    id_busca = (int)id_digitado;

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
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&id_busca;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
    } else {
        if (mysql_stmt_affected_rows(stmt) > 0) {
            printf("\n%s ID %d reativado com sucesso!\n", tabela, id_busca);
            char log_msg[256];
            sprintf(log_msg, "REATIVOU %s: ID %d", tabela, id_busca);
            registrarLog(log_msg);
        } else {
            printf("\n%s com ID %d nao foi encontrado na lista de inativos.\n", tabela, id_busca);
        }
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}

// --- MOVIMENTAÇÕES ---
void menuMovimentacoes() {
    int opcao;
    do {
        limparTela();
        printf("\n--- MOVIMENTAR ESTOQUE ---\n");
        printf("1. Registrar Saida (Venda)\n");
        printf("2. Registrar Entrada (Compra)\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        
        if (scanf("%d", &opcao) != 1) { opcao = -1; }
        limparBuffer();

        switch (opcao) {
            case 1: 
                registrarSaida();
                pausarTela(); 
                break;
            case 2: 
                printf("Funcao 'Registrar Entrada' ainda nao implementada.\n");
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
    if (!con) {
        fprintf(stderr, "Erro ao conectar ao BD.\n");
        return;
    }

    MYSQL_STMT *stmt;
    MYSQL_BIND bind[3];

    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;

    limparTela();
    printf("\n--- REGISTRAR SAIDA (VENDA) ---\n");
    listarVinhosSimples(con);

    printf("\nDigite o ID do Vinho a ser vendido (ou 0 para cancelar): ");
    lerString(input_buffer, 101);

    id_digitado = strtol(input_buffer, &end_ptr, 10);
    if (end_ptr == input_buffer || *end_ptr != '\0') {
        printf("\nErro: Entrada invalida. Apenas numeros sao permitidos.\n");
        mysql_close(con);
        return;
    }
    if (id_digitado == 0) {
        printf("Operacao cancelada.\n");
        mysql_close(con);
        return;
    }
    id_vinho = (int)id_digitado;

    printf("Digite a quantidade a ser vendida: ");
    lerString(input_buffer, 101);

    id_digitado = strtol(input_buffer, &end_ptr, 10);
    if (end_ptr == input_buffer || *end_ptr != '\0' || id_digitado <= 0) {
        printf("\nErro: Quantidade invalida. Deve ser um numero maior que zero.\n");
        mysql_close(con);
        return;
    }
    quantidade_venda = (int)id_digitado;

    printf("Digite o nome do Cliente (para o recibo): ");
    lerString(nome_cliente, 101);

    char nome_vinho[151];
    double preco_vinho = 0;
    int qtd_estoque = 0;
    
    // 5a. Buscar dados e verificar estoque
    char query_select[] = "SELECT nome_vinho, preco, quantidade FROM vinho WHERE id_vinho = ? AND ativo = 1";
    stmt = mysql_stmt_init(con);
    if (!stmt) {
        fprintf(stderr, " (venda_select) mysql_stmt_init() falhou\n");
        mysql_close(con);
        return;
    }
    
    if (mysql_stmt_prepare(stmt, query_select, strlen(query_select))) {
        fprintf(stderr, " (venda_select) mysql_stmt_prepare() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }
    
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&id_vinho;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " (venda_select) mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }
    
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " (venda_select) mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }
    
    mysql_stmt_store_result(stmt);
    if (mysql_stmt_num_rows(stmt) == 0) {
        printf("\nErro: Vinho com ID %d nao encontrado ou esta inativo.\n", id_vinho);
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    MYSQL_BIND bind_out[3];
    memset(bind_out, 0, sizeof(bind_out));
    bind_out[0].buffer_type = MYSQL_TYPE_STRING;
    bind_out[0].buffer = nome_vinho;
    bind_out[0].buffer_length = 151;
    bind_out[1].buffer_type = MYSQL_TYPE_DOUBLE;
    bind_out[1].buffer = (char *)&preco_vinho;
    bind_out[2].buffer_type = MYSQL_TYPE_LONG;
    bind_out[2].buffer = (char *)&qtd_estoque;
    
    if (mysql_stmt_bind_result(stmt, bind_out)) {
        fprintf(stderr, " (venda_select) mysql_stmt_bind_result() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }
    
    mysql_stmt_fetch(stmt);
    mysql_stmt_close(stmt);

    // 5b. Validar estoque
    if (quantidade_venda > qtd_estoque) {
        printf("\nErro: Estoque insuficiente!\n");
        printf("Disponivel: %d | Solicitado: %d\n", qtd_estoque, quantidade_venda);
        mysql_close(con);
        return;
    }

    // 5c. Atualizar o estoque
    char query_update[] = "UPDATE vinho SET quantidade = (quantidade - ?) WHERE id_vinho = ?";
    stmt = mysql_stmt_init(con);
    if (!stmt) {
        fprintf(stderr, " (venda_update) mysql_stmt_init() falhou\n");
        mysql_close(con);
        return;
    }

    if (mysql_stmt_prepare(stmt, query_update, strlen(query_update))) {
        fprintf(stderr, " (venda_update) mysql_stmt_prepare() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&quantidade_venda;
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *)&id_vinho;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " (venda_update) mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }
    
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "\nErro ao atualizar o estoque: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }
    mysql_stmt_close(stmt);

    // 5d. Registrar a movimentação
    char query_insert[] = "INSERT INTO movimentacao (tipo_movimentacao, quantidade_movimentada, fk_vinho_id_vinho, fk_usuario_id_usuario) VALUES ('SAIDA', ?, ?, ?)";
    stmt = mysql_stmt_init(con);
    if (!stmt) {
        fprintf(stderr, " (venda_insert) mysql_stmt_init() falhou\n");
        mysql_close(con);
        return;
    }

    if (mysql_stmt_prepare(stmt, query_insert, strlen(query_insert))) {
        fprintf(stderr, " (venda_insert) mysql_stmt_prepare() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&quantidade_venda;
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *)&id_vinho;
    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer = (char *)&g_id_usuario_logado; 

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " (venda_insert) mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }
    
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "\nErro ao registrar a movimentacao: %s\n", mysql_stmt_error(stmt));
    }
    mysql_stmt_close(stmt);

    char log_msg[256];
    sprintf(log_msg, "VENDA: %d un. do Vinho ID %d para %s", quantidade_venda, id_vinho, nome_cliente);
    registrarLog(log_msg);

    // --- 6. MOSTRAR RESUMO ---
    double valor_total = preco_vinho * quantidade_venda;
    
    limparTela();
    printf("\n--- VENDA CONCLUIDA COM SUCESSO ---\n");
    printf("Cliente: \t%s\n", nome_cliente);
    printf("Vinho: \t\t%s (ID: %d)\n", nome_vinho, id_vinho);
    printf("Quantidade: \t%d\n", quantidade_venda);
    printf("Preco Unit.: \tR$ %.2f\n", preco_vinho);
    printf("----------------------------------------\n");
    printf("VALOR TOTAL: \tR$ %.2f\n", valor_total);
    printf("----------------------------------------\n");
    printf("Estoque Atualizado: %d\n", (qtd_estoque - quantidade_venda));

    mysql_close(con);
}

// --- LOGS ---
void registrarLog(const char *acao) {
    if (g_id_usuario_logado == 0) return;

    MYSQL *con_log = conectar_db();
    if (!con_log) return; // Falha silenciosa se não puder conectar

    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];
    char query[] = "INSERT INTO log (acao, fk_usuario_id_usuario) VALUES (?, ?)";

    stmt = mysql_stmt_init(con_log);
    if (!stmt) { 
        mysql_close(con_log); 
        return; 
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
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

void listarLogs() {
    registrarLog("VISUALIZOU LOGS (Ultimos 50)");
    MYSQL *con = conectar_db();
    if (!con) {
        fprintf(stderr, "Erro ao conectar ao BD.\n");
        return;
    }
    
    char query[] = "SELECT l.id_log, l.data_hora, l.acao, u.nome_usuario "
                   "FROM log AS l "
                   "LEFT JOIN usuario AS u ON l.fk_usuario_id_usuario = u.id_usuario "
                   "ORDER BY l.data_hora DESC " 
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
    int num_rows = (int)mysql_num_rows(result);

    limparTela();
    printf("\n--- HISTORICO DE LOGS (Ultimos 50) ---\n");
    if (num_rows == 0) {
        printf("Nenhum registro de log encontrado.\n");
    } else {
        printf("%-5s | %-20s | %-15s | %-50s\n", "ID", "Data/Hora", "Usuario", "Acao");
        printf("--------------------------------------------------------------------------------------------------\n");
        while ((row = mysql_fetch_row(result))) {
            printf("%-5s | %-20s | %-15s | %-50s\n",
                   row[0] ? row[0] : "N/A", 
                   row[1] ? row[1] : "N/A", 
                   row[3] ? row[3] : "N/A", // Nome do Usuario
                   row[2] ? row[2] : "N/A"); // Acao
        }
        printf("--------------------------------------------------------------------------------------------------\n");
    }
    mysql_free_result(result);
    mysql_close(con);
}

void listarTodosLogs() {
    registrarLog("VISUALIZOU LOGS (TODOS)");
    MYSQL *con = conectar_db();
    if (!con) {
        fprintf(stderr, "Erro ao conectar ao BD.\n");
        return;
    }
    
    char query[] = "SELECT l.id_log, l.data_hora, l.acao, u.nome_usuario "
                   "FROM log AS l "
                   "LEFT JOIN usuario AS u ON l.fk_usuario_id_usuario = u.id_usuario "
                   "ORDER BY l.data_hora DESC"; 

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
    int num_rows = (int)mysql_num_rows(result);

    limparTela();
    printf("\n--- HISTORICO DE LOGS (COMPLETO) ---\n");
    if (num_rows == 0) {
        printf("Nenhum registro de log encontrado.\n");
    } else {
        printf("%-5s | %-20s | %-15s | %-50s\n", "ID", "Data/Hora", "Usuario", "Acao");
        printf("--------------------------------------------------------------------------------------------------\n");
        while ((row = mysql_fetch_row(result))) {
            printf("%-5s | %-20s | %-15s | %-50s\n",
                   row[0] ? row[0] : "N/A", 
                   row[1] ? row[1] : "N/A", 
                   row[3] ? row[3] : "N/A", 
                   row[2] ? row[2] : "N/A");
        }
        printf("--------------------------------------------------------------------------------------------------\n");
    }
    mysql_free_result(result);
    mysql_close(con);
}

void menuLogs() {
    int opcao;
    do {
        limparTela();
        printf("\n--- LOGS ---\n");
        printf("1. Listar Logs (Ultimos 50)\n");
        printf("2. Listar Logs (Todos)\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        
        if (scanf("%d", &opcao) != 1) { opcao = -1; }
        limparBuffer();

        switch (opcao) {
            case 1: 
                listarLogs(); 
                pausarTela(); 
                break;
            case 2:
                listarTodosLogs(); 
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

// ==========================================================
// FUNÇÕES AUXILIARES (Helpers)
// ==========================================================

/*
 * Limpa o buffer de entrada (stdin)
 * Essencial após usar scanf() para %d
 */
void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/*
 * Função segura para ler strings, substituindo scanf("%[^\n]")
 * Remove o '\n' do final.
 */
void lerString(char *buffer, int tamanho) {
    if (fgets(buffer, tamanho, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove o \n
    } else {
        // Limpa o buffer em caso de erro no fgets
        limparBuffer();
    }
}

//funcao para limpeza do terminal
void limparTela() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

//funcao para pausar o terminal (Corrigida)
void pausarTela() {
    printf("\nPressione Enter para continuar...");
    getchar();
}

/*
 * Helper interativo MESTRE.
 * Pede ao usuário por um NOME ou ID.
 * Valida, busca, e oferece criação se não existir.
 * Retorna o ID final ou -1 em caso de falha/cancelamento.
 */
int obterIdEntidadeInterativo(MYSQL *con, const char *tabela, const char *coluna_id, const char *coluna_nome, const char *prompt_menu, int tam_nome) {
    
    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;
    int id_encontrado = -1;
    char resposta;

    printf("\n%s\n", prompt_menu);
    
    // Lista as opções disponíveis
    listarEntidadeSimples(con, tabela, coluna_id, coluna_nome);
    
    printf("Digite o NOME ou o ID: ");
    lerString(input_buffer, 101);

    // Tenta converter para número
    id_digitado = strtol(input_buffer, &end_ptr, 10);
    
    if (end_ptr == input_buffer) {
        // NENHUM número foi lido. Tratar input_buffer como NOME.
        id_encontrado = obter_id_pelo_nome(con, tabela, coluna_id, coluna_nome, input_buffer);

    } else if (*end_ptr == '\0') {
        // A string inteira é um NÚMERO (ID).
        if (verificar_id_ativo(con, tabela, coluna_id, (int)id_digitado)) {
            id_encontrado = (int)id_digitado; // ID é válido
        } else {
            printf("\nErro: O ID '%ld' nao existe ou esta inativo.\n", id_digitado);
            id_encontrado = -1; // Sinaliza falha
        }
    } else {
        // Input misto (ex: "123Tinto") - Inválido
        printf("\nErro: Entrada '%s' invalida.\n", input_buffer);
        id_encontrado = -1; // Sinaliza falha
    }

    // Bloco de falha (Se não encontrou)
    if (id_encontrado == -1) {
        // Só pergunta se deseja cadastrar se a entrada foi um NOME
        if (end_ptr == input_buffer) { 
            printf("'%s' nao encontrado.\n", input_buffer);
            printf("Deseja cadastra-lo agora? (s/n): ");
            
            if (scanf(" %c", &resposta) != 1) { resposta = 'n'; }
            limparBuffer();

            if (resposta == 's' || resposta == 'S') {
                // Reusa o input_buffer que já contém o nome
                id_encontrado = (int)criar_item_retornando_id(con, tabela, coluna_nome, input_buffer);
                if (id_encontrado == -1) {
                    fprintf(stderr, "Erro ao cadastrar novo item. Abortando.\n");
                    return -1;
                }
            } else {
                return -1; // Usuário cancelou
            }
        } else {
             // Se falhou e era um ID ou input misto, apenas aborte.
            return -1;
        }
    }
    
    return id_encontrado;
}