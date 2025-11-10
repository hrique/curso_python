#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h> //conexao com o MYSQL
#include <conio.h>

// --- Informações de Conexão ---
// Modifique com os dados do seu banco de dados
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
void atualizarVinho();
void desativarVinho();
// Protótipos para Gerenciar Tipos
void menuGerenciarTipos();
void listarTipos(MYSQL *con);
void criarTipo();
void atualizarTipo();
void desativarTipo();
// Protótipos para Gerenciar Países
void menuGerenciarPaises();
void listarPaises(MYSQL *con);
void criarPais();
void atualizarPais();
void desativarPais();
//prototipos para gerenciar uvas
void menuGerenciarUva();
void listarUvas(MYSQL *con);
void criarUva();
void atualizarUva();
void desativarUva();
void gerenciarUvasDoVinho(MYSQL *con, int id_vinho);
void limparUvasDoVinho(MYSQL *con, int id_vinho);
// Prototipos para Lista de Inativos (Reativacao)
void menuListaInativos();
void listarVinhosDesativados(MYSQL *con);
void reativarVinho();
void listarTiposDesativados(MYSQL *con);
void reativarTipo();
void listarPaisesDesativados(MYSQL *con);
void reativarPais();
void listarUvasDesativadas(MYSQL *con);
void reativarUva();
// Protótipo funcao de retorno de id
long long criar_item_retornando_id(MYSQL *con, const char *tabela, const char *coluna_nome, const char *nome_valor);
// Protótipo funcao de busca de id
int obter_id_pelo_nome(MYSQL *con, const char *tabela, const char *coluna_id, const char *coluna_nome, const char *nome_busca);
// prototipo funcao de busca de id ativo
int verificar_id_ativo(MYSQL *con, const char *tabela, const char *coluna_id, int id_busca);
// Prototipo da limpeza de terminal
void limparTela();
// Prototipo para pause do terminal
void pausarTela();
//Prototipo login
int realizarLogin();
//Prototipo das movimentacoes
void menuMovimentacoes();
//Prototipo Venda
void registrarSaida();
//Prototipos de LOGs
void menuLogs();
void listarLogs();
void listarTodosLogs();
void registrarLog(const char *acao);

// Funcao auxiliar para tratar erros do MySQL
void finish_with_error(MYSQL *con) {
  fprintf(stderr, "Erro Fatal: %s\n", mysql_error(con));
  mysql_close(con);
  exit(1); // Sai do programa em caso de erro fatal de BD
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
        printf("2. Listar Vinhos (Ativos)\n");
        printf("3. Atualizar Vinho\n");
        printf("4. Desativar Vinho\n");
        printf("\n--- Gerenciamento ---\n");
        printf("5. Gerenciar Tipos\n");
        printf("6. Gerenciar Paises\n");
        printf("7. Gerenciar Uvas\n");
        printf("8. Lista de Inativos (Reativar)\n");
        printf("9. Movimentar Estoque\n");

        // --- Menu Condicional de Admin ---
        if (strcmp(g_nome_nivel_logado, "Admin") == 0) {
            printf("10. Gerenciar Logs\n");
        }
        
        printf("\n0. Sair\n");
        printf("Escolha uma das opcoes acima: ");
        
        scanf("%d", &opcao);
        while(getchar() != '\n');

        switch (opcao) {
            // --- Casos 1-9 (Preenchidos) ---
            case 1:
                criarVinho();
                pausarTela();
                break;
            case 2:
                listarVinhos();
                pausarTela();
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
                // A pausa já está dentro do menuMovimentacoes
                break;
            
            // --- Case Condicional de Admin ---
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

    mysql_library_end(); // Limpa a biblioteca ao sair normalmente
    return 0; // Termina o programa com sucesso
}

// Conecta ao banco de dados
MYSQL* conectar_db() {
    MYSQL *con = mysql_init(NULL);
    // const caminho dos plugins
    const char *plugin_dir = ".\\plugin";

    if (con == NULL) {
        fprintf(stderr, "mysql_init() falhou\n");
        exit(1);
    }

    // Diz explicitamente à biblioteca onde encontrar os plugins (ex: caching_sha2_password.dll)
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
    
    // O valor de sucesso agora é o ID do usuário
    int id_usuario_out = 0;
    char nome_nivel_out[51] = "";
    
    // Variáveis para o Banco
    MYSQL *con = conectar_db();
    MYSQL_STMT *stmt;
    MYSQL_BIND bind_in[2]; 
    MYSQL_BIND bind_out[2];
    
    // A query agora usa JOIN para buscar o nome do nível
    char query[] = "SELECT u.id_usuario, n.nome_nivel "
                   "FROM usuario AS u "
                   "JOIN nivel AS n ON u.fk_nivel_id_nivel = n.id_nivel "
                   "WHERE u.email = ? AND u.senha = SHA2(?, 256) AND u.ativo = 1";

    limparTela();
    printf("\n--- LOGIN VINTAGEM ---\n");
    printf("Digite seu Email: ");
    scanf("%100[^\n]", email);
    while(getchar() != '\n');
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
    // CORREÇÃO: Erro no init
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

    // CORREÇÃO: Erro no bind
    if (mysql_stmt_bind_param(stmt, bind_in)) { 
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return 0;
    }
    
    // CORREÇÃO: Erro no execute
    if (mysql_stmt_execute(stmt)) { 
        fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return 0;
    }
    
    // CORREÇÃO: Erro no store_result
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
    MYSQL_BIND bind_in[1];  // Parâmetros de entrada (WHERE)
    MYSQL_BIND bind_out[1]; // Parâmetros de saída (O ID que queremos)
    int id_encontrado = -1; // Valor padrão (não encontrado)
    char query[512];

    // Monta a query dinamicamente. Ex: "SELECT id_tipo FROM tipo WHERE nome_tipo = ?"
    sprintf(query, "SELECT %s FROM %s WHERE %s = ? AND ativo = 1", coluna_id, tabela, coluna_nome);

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

    // --- Bind do parâmetro de ENTRADA (o nome que buscamos) ---
    memset(bind_in, 0, sizeof(bind_in));
    bind_in[0].buffer_type = MYSQL_TYPE_STRING;
    bind_in[0].buffer = (char *)nome_busca;
    bind_in[0].buffer_length = strlen(nome_busca);

    if (mysql_stmt_bind_param(stmt, bind_in)) {
        fprintf(stderr, " (obter_id) mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    // Executa a busca
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " (obter_id) mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    // --- Bind do parâmetro de SAÍDA (o ID que queremos) ---
    memset(bind_out, 0, sizeof(bind_out));
    bind_out[0].buffer_type = MYSQL_TYPE_LONG; // Esperamos um ID (int)
    bind_out[0].buffer = (char *)&id_encontrado;

    if (mysql_stmt_bind_result(stmt, bind_out)) {
        fprintf(stderr, " (obter_id) mysql_stmt_bind_result() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    // Pega o resultado
    if (mysql_stmt_fetch(stmt) == 0) {
        // Sucesso! 'id_encontrado' agora tem o valor do ID.
    } else {
        // Nenhum resultado encontrado (ou erro), reseta para -1
        id_encontrado = -1;
    }

    mysql_stmt_close(stmt);
    return id_encontrado;
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

    // Ex: "INSERT INTO tipo (nome_tipo) VALUES (?)"
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

    long long new_id = mysql_insert_id(con); // Pega o último ID inserido
    mysql_stmt_close(stmt);
    
    if (new_id == 0) {
        fprintf(stderr, " (criar_item) Nao foi possivel obter o ID inserido.\n");
        return -1;
    }
    
    printf("... Item '%s' cadastrado com ID %lld.\n", nome_valor, new_id);
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

    // Tenta buscar o resultado
    if (mysql_stmt_fetch(stmt) == 0) {
        // Sucesso! Encontrou um registro.
        sucesso = 1;
    }

    mysql_stmt_close(stmt);
    return sucesso;
}

// ==========================================================
// FUNÇÃO CRIAR ATUALIZADA (Aceita NOME ou ID)
// ==========================================================
void criarVinho() {
    Vinho v;
    MYSQL *con = conectar_db();
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[6];
    char query[] = "INSERT INTO vinho(nome_vinho, safra, preco, quantidade, fk_tipo_id_tipo, fk_paisorigem_id_paisorigem, ativo) VALUES(?, ?, ?, ?, ?, ?, 1)";

    // Buffers para entrada
    char input_buffer[101]; // Buffer genérico para ler ID ou Nome
    long id_digitado = 0;
    char *end_ptr; // Ponteiro para strtol
    int id_tipo_encontrado = -1;
    int id_pais_encontrado = -1;
    char resposta;

    // 1. Coletar dados do Vinho
    limparTela();
    printf("\n>>> Adicionar Novo Vinho <<<\n");
    printf("Nome: ");
    scanf("%150[^\n]", v.nome_vinho);
    while(getchar() != '\n'); 

    printf("Safra (Ano): ");
    scanf("%d", &v.safra);

    printf("Preco (ex: 59.99): ");
    scanf("%lf", &v.preco);

    printf("Quantidade em estoque: ");
    scanf("%d", &v.quantidade);
    while(getchar() != '\n'); 

    // --- 2. Coletar e Validar Tipo (NOME ou ID) ---
    printf("\n--- Selecao de Tipo ---\n");
    listarTipos(con); // Mostra a lista de tipos existentes
    printf("Digite o NOME ou o ID do Tipo: ");
    scanf("%100[^\n]", input_buffer); // Lê como string
    while(getchar() != '\n');
    
    // Tenta converter para número
    id_digitado = strtol(input_buffer, &end_ptr, 10);
    
    if (end_ptr == input_buffer) {
        // NENHUM número foi lido. Tratar input_buffer como NOME.
        id_tipo_encontrado = obter_id_pelo_nome(con, "tipo", "id_tipo", "nome_tipo", input_buffer);

    } else if (*end_ptr == '\0') {
        // A string inteira é um NÚMERO (ID).
        if (verificar_id_ativo(con, "tipo", "id_tipo", (int)id_digitado)) {
            id_tipo_encontrado = (int)id_digitado; // ID é válido
        } else {
            printf("\nErro: O ID de tipo '%ld' nao existe ou esta inativo.\n", id_digitado);
            id_tipo_encontrado = -1; // Sinaliza falha
        }
    } else {
        // Input misto (ex: "123Tinto") - Inválido
        printf("\nErro: Entrada '%s' invalida.\n", input_buffer);
        id_tipo_encontrado = -1; // Sinaliza falha
    }

    // Bloco de falha (igual a antes, mas agora mais inteligente)
    if (id_tipo_encontrado == -1) {
        // Só pergunta se deseja cadastrar se a entrada foi um NOME
        if (end_ptr == input_buffer) { 
            printf("Tipo '%s' nao encontrado.\n", input_buffer);
            printf("Deseja cadastra-lo agora? (s/n): ");
            scanf(" %c", &resposta);
            while(getchar() != '\n');

            if (resposta == 's' || resposta == 'S') {
                id_tipo_encontrado = (int)criar_item_retornando_id(con, "tipo", "nome_tipo", input_buffer);
                if (id_tipo_encontrado == -1) {
                    fprintf(stderr, "Erro ao cadastrar novo tipo. Abortando.\n");
                    mysql_close(con);
                    return;
                }
            } else {
                printf("Criacao de vinho abortada.\n");
                mysql_close(con);
                return;
            }
        } else {
            // Se falhou e era um ID ou input misto, apenas aborte.
            printf("Criacao de vinho abortada.\n");
            mysql_close(con);
            return;
        }
    }
    v.fk_tipo_id_tipo = id_tipo_encontrado;


    // --- 3. Coletar e Validar País (NOME ou ID) ---
    printf("\n--- Selecao de Pais ---\n");
    listarPaises(con); // Mostra a lista de países existentes
    printf("Digite o NOME ou o ID do Pais de Origem: ");
    scanf("%100[^\n]", input_buffer); // Reusa o buffer
    while(getchar() != '\n');
    
    // Tenta converter para número
    id_digitado = strtol(input_buffer, &end_ptr, 10);
    
    if (end_ptr == input_buffer) {
        // NENHUM número foi lido. Tratar input_buffer como NOME.
        id_pais_encontrado = obter_id_pelo_nome(con, "paisorigem", "id_paisorigem", "nome_pais", input_buffer);

    } else if (*end_ptr == '\0') {
        // A string inteira é um NÚMERO (ID).
        if (verificar_id_ativo(con, "paisorigem", "id_paisorigem", (int)id_digitado)) {
            id_pais_encontrado = (int)id_digitado; // ID é válido
        } else {
            printf("\nErro: O ID de pais '%ld' nao existe ou esta inativo.\n", id_digitado);
            id_pais_encontrado = -1; // Sinaliza falha
        }
    } else {
        // Input misto (ex: "123Chile") - Inválido
        printf("\nErro: Entrada '%s' invalida.\n", input_buffer);
        id_pais_encontrado = -1; // Sinaliza falha
    }
    
    // Bloco de falha (igual a antes, mas agora mais inteligente)
    if (id_pais_encontrado == -1) {
        // Só pergunta se deseja cadastrar se a entrada foi um NOME
        if (end_ptr == input_buffer) { 
            printf("Pais '%s' nao encontrado.\n", input_buffer);
            printf("Deseja cadastra-lo agora? (s/n): ");
            scanf(" %c", &resposta);
            while(getchar() != '\n');

            if (resposta == 's' || resposta == 'S') {
                id_pais_encontrado = (int)criar_item_retornando_id(con, "paisorigem", "nome_pais", input_buffer);
                if (id_pais_encontrado == -1) {
                    fprintf(stderr, "Erro ao cadastrar novo pais. Abortando.\n");
                    mysql_close(con);
                    return;
                }
            } else {
                printf("Criacao de vinho abortada.\n");
                mysql_close(con);
                return;
            }
        } else {
            // Se falhou e era um ID ou input misto, apenas aborte.
            printf("Criacao de vinho abortada.\n");
            mysql_close(con);
            return;
        }
    }
    v.fk_paisorigem_id_paisorigem = id_pais_encontrado;

    // --- 4. Preparar e Executar o INSERT do Vinho (COM TRATAMENTO DE ERRO) ---
    stmt = mysql_stmt_init(con);
    // Erro no INIT
    if (!stmt) { 
        fprintf(stderr, " mysql_stmt_init() falhou\n");
        mysql_close(con);
        return;
    }

    // Erro no PREPARE
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, " mysql_stmt_prepare() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    // Lógica de Bind dos 6 parâmetros
    memset(bind, 0, sizeof(bind));
  	// Param 0: nome_vinho
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)v.nome_vinho;
    bind[0].buffer_length = strlen(v.nome_vinho);
  	// Param 1: safra
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *)&v.safra;
  	// Param 2: preco
    bind[2].buffer_type = MYSQL_TYPE_DOUBLE;
    bind[2].buffer = (char *)&v.preco;
  	// Param 3: quantidade
    bind[3].buffer_type = MYSQL_TYPE_LONG;
    bind[3].buffer = (char *)&v.quantidade;
  	// Param 4: fk_tipo_id_tipo
    bind[4].buffer_type = MYSQL_TYPE_LONG;
    bind[4].buffer = (char *)&v.fk_tipo_id_tipo;
  	// Param 5: fk_paisorigem_id_paisorigem
    bind[5].buffer_type = MYSQL_TYPE_LONG;
    bind[5].buffer = (char *)&v.fk_paisorigem_id_paisorigem;

    // Erro no BIND
    if (mysql_stmt_bind_param(stmt, bind)) { 
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }
    
    // Erro no EXECUTE
    if (mysql_stmt_execute(stmt)) { 
        fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    printf("\n>>> Vinho '%s' adicionado com sucesso! <<<\n", v.nome_vinho);

    // Obter o ID do vinho que acabamos de criar
    long long novo_id_vinho = mysql_insert_id(con);
    //registro de LOG
    char log_msg[256];
    sprintf(log_msg, "CRIOU VINHO: ID %lld (%s)", novo_id_vinho, v.nome_vinho);
    registrarLog(log_msg);

    // Fechar o statement do vinho
    mysql_stmt_close(stmt);

    if (novo_id_vinho > 0) {
        // Chamar a função de gerenciamento de uvas
        gerenciarUvasDoVinho(con, (int)novo_id_vinho);
    } else {
        fprintf(stderr, "Erro critico: Nao foi possivel obter o ID do vinho recem-criado.\n");
    }

    // Fechar a conexão (no final de tudo)
    mysql_close(con);
}

//READ
void listarVinhos() {
    MYSQL *con = conectar_db();
    
    // Query com JOINs e GROUP_CONCAT
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
        "WHERE v.ativo = 1 "
        "GROUP BY v.id_vinho"; // Agrupa para o GROUP_CONCAT funcionar
    
    // Executa a query
    if (mysql_query(con, query)) {
        fprintf(stderr, "Erro ao listar vinhos: %s\n", mysql_error(con));
        mysql_close(con);
        return;
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        fprintf(stderr, "Erro ao obter resultados: %s\n", mysql_error(con));
        mysql_close(con);
        return;
    }

    MYSQL_ROW row;
    int num_rows = mysql_num_rows(result);  // Pega o número de linhas

    printf("\n>>> Lista de Vinhos (Apenas Ativos) <<<\n");
    if (num_rows == 0) {
        printf("Nenhum vinho ativo cadastrado.\n");
    } else {
        // --- INÍCIO DA CORREÇÃO ---
        // Cabeçalho corrigido para 8 colunas
        printf("%-3s | %-25s | %-5s | %-10s | %-3s | %-15s | %-15s | %-20s\n", 
               "ID", "Nome", "Safra", "Preco", "Qtd", "Tipo", "Pais", "Uvas");
        printf("----------------------------------------------------------------------------------------------------------------------\n");

        while ((row = mysql_fetch_row(result))) {
            // Loop corrigido para imprimir row[0] até row[7]
            printf("%-3s | %-25s | %-5s | %-10s | %-3s | %-15s | %-15s | %-20s\n",
                   row[0] ? row[0] : "N/A", // ID
                   row[1] ? row[1] : "N/A", // Nome
                   row[2] ? row[2] : "N/A", // Safra
                   row[3] ? row[3] : "N/A", // Preco
                   row[4] ? row[4] : "N/A", // Qtd
                   row[5] ? row[5] : "N/A", // Tipo
                   row[6] ? row[6] : "N/A", // Pais
                   row[7] ? row[7] : "N/A"); // Uvas
        }
        printf("----------------------------------------------------------------------------------------------------------------------\n");
        // --- FIM DA CORREÇÃO ---
    }

    mysql_free_result(result);
    mysql_close(con);
}

//UPDATE
// ==========================================================
// FUNÇÃO ATUALIZAR VINHO (Corrigida com Validação e Cancelamento)
// ==========================================================
void atualizarVinho() {
    int id_busca = 0; // O ID do vinho que vamos atualizar
    Vinho v;
    MYSQL *con = NULL; // Inicia a conexão como NULA
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[7];
    char query[] = "UPDATE vinho SET nome_vinho = ?, safra = ?, preco = ?, quantidade = ?, fk_tipo_id_tipo = ?, fk_paisorigem_id_paisorigem = ? WHERE id_vinho = ?";

    // Buffers para validação de ID
    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;
    
    // Buffers para nomes (tipo/país)
    char nome_tipo[51];
    char nome_pais[101];
    int id_tipo_encontrado;
    int id_pais_encontrado;
    char resposta;

    // --- 1. MOSTRAR OS VINHOS PRIMEIRO ---
    limparTela();
    printf("\n--- ATUALIZAR VINHO ---\n");
    printf("Estes são os vinhos ativos que podem ser atualizados:\n\n");
    listarVinhos(); // Esta função abre e fecha a própria conexão

    // --- 2. COLETAR E VALIDAR O ID ---
    printf("\nDigite o ID do Vinho que deseja atualizar (ou 0 para cancelar): ");
    scanf("%100[^\n]", input_buffer);
    while(getchar() != '\n'); 

    // Tenta converter a string para um número
    id_digitado = strtol(input_buffer, &end_ptr, 10);

    // Validação:
    if (end_ptr == input_buffer || *end_ptr != '\0') {
        printf("\nErro: Entrada invalida. Apenas numeros sao permitidos.\n");
        return; // Retorna ao menu
    }
    
    if (id_digitado == 0) {
        printf("Operacao cancelada.\n");
        return; // Retorna ao menu
    }
    
    id_busca = (int)id_digitado; // O ID é válido

    // --- 3. SE O ID FOR VÁLIDO, CONECTA AO DB E CONTINUA ---
    con = conectar_db();
    
    printf("\n>>> Insira os NOVOS dados para o Vinho ID %d <<<\n", id_busca);
    
    printf("Novo Nome: ");
    scanf("%150[^\n]", v.nome_vinho);
    while(getchar() != '\n');

    printf("Nova Safra (Ano): ");
    scanf("%d", &v.safra);

    printf("Novo Preco (ex: 59.99): ");
    scanf("%lf", &v.preco);

    printf("Nova Quantidade: ");
    scanf("%d", &v.quantidade);
    while(getchar() != '\n');

    // --- 4. Coletar e Validar NOVO Tipo ---
    printf("\n--- Selecao do NOVO Tipo ---\n");
    listarTipos(con); // Passa a conexão
    printf("Digite o NOME ou o ID do novo Tipo: ");
    scanf("%100[^\n]", input_buffer); // Reusa o buffer
    while(getchar() != '\n');
    
    id_digitado = strtol(input_buffer, &end_ptr, 10);
    
    if (end_ptr == input_buffer) { // É um NOME
        id_tipo_encontrado = obter_id_pelo_nome(con, "tipo", "id_tipo", "nome_tipo", input_buffer);
    } else if (*end_ptr == '\0') { // É um ID
        if (verificar_id_ativo(con, "tipo", "id_tipo", (int)id_digitado)) {
            id_tipo_encontrado = (int)id_digitado;
        } else {
            id_tipo_encontrado = -1;
        }
    } else { // É misto (inválido)
        id_tipo_encontrado = -1;
    }

    if (id_tipo_encontrado == -1) {
        if (end_ptr == input_buffer) { // Se a falha foi por NOME não encontrado
            printf("Tipo '%s' nao encontrado.\n", input_buffer);
            printf("Deseja cadastra-lo agora? (s/n): ");
            scanf(" %c", &resposta);
            while(getchar() != '\n');

            if (resposta == 's' || resposta == 'S') {
                id_tipo_encontrado = (int)criar_item_retornando_id(con, "tipo", "nome_tipo", input_buffer);
                if (id_tipo_encontrado == -1) {
                    fprintf(stderr, "Erro ao cadastrar novo tipo. Abortando atualizacao.\n");
                    mysql_close(con);
                    return;
                }
            } else {
                printf("Atualizacao abortada.\n");
                mysql_close(con);
                return;
            }
        } else { // Se a falha foi por ID inválido ou misto
            printf("ID ou entrada invalida. Atualizacao abortada.\n");
            mysql_close(con);
            return;
        }
    }
    v.fk_tipo_id_tipo = id_tipo_encontrado;

    // --- 5. Coletar e Validar NOVO País ---
    printf("\n--- Selecao do NOVO Pais ---\n");
    listarPaises(con); // Passa a conexão
    printf("Digite o NOME ou o ID do novo Pais de Origem: ");
    scanf("%100[^\n]", input_buffer); // Reusa o buffer
    while(getchar() != '\n');
    
    id_digitado = strtol(input_buffer, &end_ptr, 10);
    
    if (end_ptr == input_buffer) { // É um NOME
        id_pais_encontrado = obter_id_pelo_nome(con, "paisorigem", "id_paisorigem", "nome_pais", input_buffer);
    } else if (*end_ptr == '\0') { // É um ID
        if (verificar_id_ativo(con, "paisorigem", "id_paisorigem", (int)id_digitado)) {
            id_pais_encontrado = (int)id_digitado;
        } else {
            id_pais_encontrado = -1;
        }
    } else { // É misto (inválido)
        id_pais_encontrado = -1;
    }

    if (id_pais_encontrado == -1) {
        if (end_ptr == input_buffer) { // Se a falha foi por NOME não encontrado
            printf("Pais '%s' nao encontrado.\n", input_buffer);
            printf("Deseja cadastra-lo agora? (s/n): ");
            scanf(" %c", &resposta);
            while(getchar() != '\n');

            if (resposta == 's' || resposta == 'S') {
                id_pais_encontrado = (int)criar_item_retornando_id(con, "paisorigem", "nome_pais", input_buffer);
                if (id_pais_encontrado == -1) {
                    fprintf(stderr, "Erro ao cadastrar novo pais. Abortando atualizacao.\n");
                    mysql_close(con);
                    return;
                }
            } else {
                printf("Atualizacao abortada.\n");
                mysql_close(con);
                return;
            }
        } else { // Se a falha foi por ID inválido ou misto
            printf("ID ou entrada invalida. Atualizacao abortada.\n");
            mysql_close(con);
            return;
        }
    }
    v.fk_paisorigem_id_paisorigem = id_pais_encontrado;

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

    // Bind dos 7 parâmetros
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
        //registro de LOG
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
    limparUvasDoVinho(con, id_busca); // Limpa as uvas antigas
    gerenciarUvasDoVinho(con, id_busca); // Adiciona as novas

    mysql_close(con); // <--- FECHA A CONEXÃO ÚNICA
}

//DELETE(DESATIVAR)
// ==========================================================
// FUNÇÃO DESATIVAR VINHO (com Validação e Cancelamento)
// ==========================================================
void desativarVinho() {
    int id_busca = 0;
    MYSQL *con = NULL;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    char query[] = "UPDATE vinho SET ativo = 0 WHERE id_vinho = ?";

    // Buffers para validação
    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;

    // --- 1. MOSTRAR OS VINHOS PRIMEIRO ---
    limparTela();
    printf("\n--- DESATIVAR VINHO ---\n");
    printf("Estes são os vinhos ativos que podem ser desativados:\n\n");
    listarVinhos(); // Esta função (pelo seu código) abre/fecha a própria conexão

    // --- 2. COLETAR E VALIDAR O ID ---
    printf("\nDigite o ID do Vinho que deseja DESATIVAR (ou 0 para cancelar): ");
    scanf("%100[^\n]", input_buffer);
    while(getchar() != '\n'); 

    id_digitado = strtol(input_buffer, &end_ptr, 10);

    if (end_ptr == input_buffer || *end_ptr != '\0') {
        printf("\nErro: Entrada invalida. Apenas numeros sao permitidos.\n");
        return; // Retorna ao menu
    }
    
    if (id_digitado == 0) {
        printf("Operacao cancelada.\n");
        return; // Retorna ao menu
    }
    
    id_busca = (int)id_digitado;

    // --- 3. SE O ID FOR VÁLIDO, CONECTA AO DB E EXECUTA ---
    con = conectar_db();
    
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
            //registro de LOG
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

//TIPOS DE VINHOS
void listarTipos(MYSQL *con) {
    if (mysql_query(con, "SELECT id_tipo, nome_tipo FROM tipo WHERE ativo = 1")) {
        fprintf(stderr, "Erro ao listar tipos: %s\n", mysql_error(con));
        // Não fecha a conexão, o chamador fará isso
        return; 
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        fprintf(stderr, "Erro ao obter resultados (tipos): %s\n", mysql_error(con));
        return;
    }

    MYSQL_ROW row;
    int num_rows = mysql_num_rows(result);

    printf("\n--- Lista de Tipos Cadastrados (Ativos) ---\n");
    if (num_rows == 0) {
        printf("Nenhum tipo ativo cadastrado.\n");
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

void criarTipo() {
    MYSQL *con = conectar_db(); // Esta função abre/fecha sua própria conexão
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    char nome_tipo[51];
    char query[] = "INSERT INTO tipo (nome_tipo) VALUES (?)";

    printf("\n--- Adicionar Novo Tipo ---\n");
    printf("Nome do Tipo: ");
    scanf("%50[^\n]", nome_tipo);
    while(getchar() != '\n');

    stmt = mysql_stmt_init(con);
    if (!stmt) {
        fprintf(stderr, " mysql_stmt_init() falhou\n");
        mysql_close(con);
        return;
    }

    // Erro no PREPARE
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, " mysql_stmt_prepare() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)nome_tipo;
    bind[0].buffer_length = strlen(nome_tipo);

    // Erro no BIND
    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    // Erro no EXECUTE
    if (mysql_stmt_execute(stmt)) {
        // Verifica se o erro foi por entrada duplicada (Erro 1062)
        if (mysql_errno(con) == 1062) {
            fprintf(stderr, "\nErro: O tipo '%s' ja esta cadastrado.\n", nome_tipo);
        } else {
            // Outro erro qualquer
            fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        }
    } else {
        printf("Tipo '%s' criado com sucesso!\n", nome_tipo);
        //registro de LOG
        char log_msg[256];
        sprintf(log_msg, "CRIOU TIPO: %s", nome_tipo);
        registrarLog(log_msg);
    }

    mysql_stmt_close(stmt);
    mysql_close(con);
}

void atualizarTipo() {
    MYSQL *con = conectar_db(); // Abre a conexão
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];
    int id_tipo;
    char nome_tipo[51];
    char query[] = "UPDATE tipo SET nome_tipo = ? WHERE id_tipo = ?";

    // Buffers para validação
    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;

    limparTela();
    printf("\n--- Atualizar Tipo ---\n");
    
    // 1. Listar os tipos
    listarTipos(con); 

    // 2. Coletar e Validar ID
    printf("\nDigite o ID do Tipo a atualizar (ou 0 para cancelar): ");
    scanf("%100[^\n]", input_buffer);
    while(getchar() != '\n');

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
    id_tipo = (int)id_digitado;

    // 3. Coletar Novo Nome
    printf("Novo Nome: ");
    scanf("%50[^\n]", nome_tipo);
    while(getchar() != '\n');

    // 4. Executar o UPDATE
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
    bind[0].buffer = (char *)nome_tipo;
    bind[0].buffer_length = strlen(nome_tipo);
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *)&id_tipo;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        if (mysql_errno(con) == 1062) {
            fprintf(stderr, "\nErro: O nome de tipo '%s' ja esta em uso.\n", nome_tipo);
        } else {
            fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        }
    } else {
        if (mysql_stmt_affected_rows(stmt) > 0) {
            printf("Tipo atualizado com sucesso!\n");
            //registro de LOG
            char log_msg[256];
            sprintf(log_msg, "ATUALIZOU TIPO: ID %d para '%s'", id_tipo, nome_tipo);
            registrarLog(log_msg);
        } else {
            printf("ID do Tipo nao encontrado ou nenhum dado foi alterado.\n");
        }
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}

void desativarTipo() {
    int id_busca = 0;
    MYSQL *con = conectar_db(); // Abre a conexão UMA VEZ
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    char query[] = "UPDATE tipo SET ativo = 0 WHERE id_tipo = ?";

    // Buffers para validação
    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;

    // --- 1. MOSTRAR OS TIPOS PRIMEIRO ---
    limparTela();
    printf("\n--- DESATIVAR TIPO ---\n");
    printf("Estes são os tipos ativos que podem ser desativados:\n\n");
    listarTipos(con); // Passa a conexão

    // --- 2. COLETAR E VALIDAR O ID ---
    printf("\nDigite o ID do Tipo que deseja DESATIVAR (ou 0 para cancelar): ");
    scanf("%100[^\n]", input_buffer);
    while(getchar() != '\n'); 

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

    // --- 3. SE O ID FOR VÁLIDO, EXECUTA O UPDATE ---
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
            printf("\nTipo ID %d desativado com sucesso!\n", id_busca);
            //registro de LOG
            char log_msg[256];
            sprintf(log_msg, "DESATIVOU TIPO: ID %d", id_busca); // id_busca é o ID
            registrarLog(log_msg);
        } else {
            printf("\nTipo com ID %d nao foi encontrado ou ja esta desativado.\n", id_busca);
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
        printf("4. Desativar Tipo\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);
        while(getchar() != '\n');

        switch (opcao) {
            case 1: { // abrir/fechar conexao
                MYSQL *con = conectar_db();
                if (con) {
                    listarTipos(con);
                    mysql_close(con);
                }
                pausarTela(); 
                break;
            }
            case 2: 
                criarTipo();
                pausarTela(); 
                break;
            case 3: 
                atualizarTipo();
                pausarTela(); 
                break;
            case 4: 
                desativarTipo(); 
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

//PAISES
void listarPaises(MYSQL *con) {    
    if (mysql_query(con, "SELECT id_paisorigem, nome_pais FROM paisorigem WHERE ativo = 1")) {
        fprintf(stderr, "Erro ao listar paises: %s\n", mysql_error(con));
        return;
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        fprintf(stderr, "Erro ao obter resultados (paises): %s\n", mysql_error(con));
        return;
    }

    MYSQL_ROW row;
    int num_rows = mysql_num_rows(result);

    printf("\n--- Lista de Paises Cadastrados (Ativos) ---\n");
    if (num_rows == 0) {
        printf("Nenhum pais ativo cadastrado.\n");
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

void criarPais() {
    MYSQL *con = conectar_db();
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    char nome_pais[101];
    char query[] = "INSERT INTO paisorigem (nome_pais) VALUES (?)";

    printf("\n--- Adicionar Novo Pais ---\n");
    printf("Nome do Pais: ");
    scanf("%100[^\n]", nome_pais);
    while(getchar() != '\n');

    stmt = mysql_stmt_init(con);
    if (!stmt) {
        fprintf(stderr, " mysql_stmt_init() falhou\n");
        mysql_close(con);
        return;
    }

    // Erro no PREPARE
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, " mysql_stmt_prepare() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)nome_pais;
    bind[0].buffer_length = strlen(nome_pais);

    // Erro no BIND
    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    // Erro no EXECUTE
    if (mysql_stmt_execute(stmt)) {
        // Verifica se o erro foi por entrada duplicada (Erro 1062)
        if (mysql_errno(con) == 1062) {
            fprintf(stderr, "\nErro: O pais '%s' ja esta cadastrado.\n", nome_pais);
        } else {
            // Outro erro qualquer
            fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        }
    } else {
        printf("Pais '%s' criado com sucesso!\n", nome_pais);
        //registro de LOG
        char log_msg[256];
        sprintf(log_msg, "CRIOU PAIS: %s", nome_pais);
        registrarLog(log_msg);  
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}

void atualizarPais() {
    MYSQL *con = conectar_db(); // Abre a conexão
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];
    int id_pais;
    char nome_pais[101];
    char query[] = "UPDATE paisorigem SET nome_pais = ? WHERE id_paisorigem = ?";

    // Buffers para validação
    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;

    limparTela();
    printf("\n--- Atualizar Pais ---\n");

    // 1. Listar Países
    listarPaises(con);

    // 2. Coletar e Validar ID
    printf("\nDigite o ID do Pais a atualizar (ou 0 para cancelar): ");
    scanf("%100[^\n]", input_buffer);
    while(getchar() != '\n');

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
    id_pais = (int)id_digitado;

    // 3. Coletar Novo Nome
    printf("Novo Nome: ");
    scanf("%100[^\n]", nome_pais);
    while(getchar() != '\n');

    // 4. Executar o UPDATE
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
    bind[0].buffer = (char *)nome_pais;
    bind[0].buffer_length = strlen(nome_pais);
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *)&id_pais;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        if (mysql_errno(con) == 1062) {
            fprintf(stderr, "\nErro: O nome de pais '%s' ja esta em uso.\n", nome_pais);
        } else {
            fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        }
    } else {
        if (mysql_stmt_affected_rows(stmt) > 0) {
            printf("Pais atualizado com sucesso!\n");
            //registro de LOG
            char log_msg[256];
            sprintf(log_msg, "ATUALIZOU PAIS: ID %d para '%s'", id_pais, nome_pais);
            registrarLog(log_msg);
        } else {
            printf("ID do Pais nao encontrado ou nenhum dado foi alterado.\n");
        }
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}

void desativarPais() {
    int id_busca = 0;
    MYSQL *con = conectar_db(); // Abre a conexão UMA VEZ
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    char query[] = "UPDATE paisorigem SET ativo = 0 WHERE id_paisorigem = ?";

    // Buffers para validação
    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;

    // --- 1. MOSTRAR OS PAÍSES PRIMEIRO ---
    limparTela();
    printf("\n--- DESATIVAR PAIS ---\n");
    printf("Estes são os paises ativos que podem ser desativados:\n\n");
    listarPaises(con); // Passa a conexão

    // --- 2. COLETAR E VALIDAR O ID ---
    printf("\nDigite o ID do Pais que deseja DESATIVAR (ou 0 para cancelar): ");
    scanf("%100[^\n]", input_buffer);
    while(getchar() != '\n'); 

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

    // --- 3. SE O ID FOR VÁLIDO, EXECUTA O UPDATE ---
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
            printf("\nPais ID %d desativado com sucesso!\n", id_busca);
            //registro de LOG
            char log_msg[256];
            sprintf(log_msg, "DESATIVOU PAIS: ID %d", id_busca);
            registrarLog(log_msg);
        } else {
            printf("\nPais com ID %d nao foi encontrado ou ja esta desativado.\n", id_busca);
        }
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}


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
        scanf("%d", &opcao);
        while(getchar() != '\n');

        switch (opcao) {
            case 1: { // abrir/fechar conexao
                MYSQL *con = conectar_db();
                if (con) {
                    listarPaises(con);
                    mysql_close(con);
                }
                pausarTela(); 
                break;
            }
            case 2: 
                criarPais();
                pausarTela(); 
                break;
            case 3: 
                atualizarPais();
                pausarTela(); 
                break;
            case 4: 
                desativarPais();
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

//CRUD UVAS
void criarUva() {
    MYSQL *con = conectar_db();
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    char nome_uva[101];
    char query[] = "INSERT INTO uva (nome_uva) VALUES (?)";

    printf("\n--- Adicionar Nova Uva ---\n");
    printf("Nome da Uva: ");
    scanf("%100[^\n]", nome_uva);
    while(getchar() != '\n');

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
    bind[0].buffer = (char *)nome_uva;
    bind[0].buffer_length = strlen(nome_uva);

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        if (mysql_errno(con) == 1062) {
            fprintf(stderr, "\nErro: A uva '%s' ja esta cadastrada.\n", nome_uva);
        } else {
            fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        }
    } else {
        printf("Uva '%s' criada com sucesso!\n", nome_uva);
        //registro de LOG
        char log_msg[256];
        sprintf(log_msg, "CRIOU UVA: %s", nome_uva);
        registrarLog(log_msg);
    }
    
    mysql_stmt_close(stmt);
    mysql_close(con);
}

void listarUvas(MYSQL *con) {
    if (mysql_query(con, "SELECT id_uva, nome_uva FROM uva WHERE ativo = 1")) {
        fprintf(stderr, "Erro ao listar uvas: %s\n", mysql_error(con));
        return;
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) { 
        fprintf(stderr, "Erro ao obter resultados (uvas): %s\n", mysql_error(con));
        return;
    }

    MYSQL_ROW row;
    int num_rows = mysql_num_rows(result);

    printf("\n--- Lista de Uvas Cadastradas (Ativas) ---\n");
    if (num_rows == 0) {
        printf("Nenhuma uva ativa cadastrada.\n");
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

void atualizarUva() {
    MYSQL *con = conectar_db();
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];
    int id_uva;
    char nome_uva[101];
    char query[] = "UPDATE uva SET nome_uva = ? WHERE id_uva = ?";

    printf("\n--- Atualizar Uva ---\n");
    printf("ID da Uva a atualizar: ");
    scanf("%d", &id_uva);
    while(getchar() != '\n');

    printf("Novo Nome: ");
    scanf("%100[^\n]", nome_uva);
    while(getchar() != '\n');

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
    bind[0].buffer = (char *)nome_uva;
    bind[0].buffer_length = strlen(nome_uva);
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *)&id_uva;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        if (mysql_errno(con) == 1062) {
            fprintf(stderr, "\nErro: O nome de uva '%s' ja esta em uso.\n", nome_uva);
        } else {
            fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        }
    } else {
        if (mysql_stmt_affected_rows(stmt) > 0) {
            printf("Uva atualizada com sucesso!\n");
            //registro de LOG
            char log_msg[256];
            sprintf(log_msg, "ATUALIZOU UVA: ID %d para '%s'", id_uva, nome_uva);
            registrarLog(log_msg);
        } else {
            printf("ID da Uva nao encontrado ou nenhum dado foi alterado.\n");
        }
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}

void desativarUva() {
    int id_busca = 0;
    MYSQL *con = conectar_db(); // Abre a conexão UMA VEZ
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    char query[] = "UPDATE uva SET ativo = 0 WHERE id_uva = ?";

    // Buffers para validação
    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;

    // --- 1. MOSTRAR AS UVAS PRIMEIRO ---
    limparTela();
    printf("\n--- DESATIVAR UVA ---\n");
    printf("Estas são as uvas ativas que podem ser desativadas:\n\n");
    listarUvas(con); // Passa a conexão

    // --- 2. COLETAR E VALIDAR O ID ---
    printf("\nDigite o ID da Uva que deseja DESATIVAR (ou 0 para cancelar): ");
    scanf("%100[^\n]", input_buffer);
    while(getchar() != '\n'); 

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

    // --- 3. SE O ID FOR VÁLIDO, EXECUTA O UPDATE ---
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
            printf("\nUva ID %d desativada com sucesso!\n", id_busca);
            //registro de LOG
            char log_msg[256];
            sprintf(log_msg, "DESATIVOU UVA: ID %d", id_busca); // id_busca é o ID
            registrarLog(log_msg);
        } else {
            printf("\nUva com ID %d nao foi encontrada ou ja esta desativada.\n", id_busca);
        }
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}

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
        scanf("%d", &opcao);
        while(getchar() != '\n');

        switch (opcao) {
            case 1: { //abrir/fechar conexao
                MYSQL *con = conectar_db();
                if (con) {
                    listarUvas(con);
                    mysql_close(con);
                }
                pausarTela(); 
                break;
            }
            case 2: 
                criarUva(); 
                pausarTela(); 
                break;
            case 3: 
                atualizarUva(); 
                pausarTela(); 
                break;
            case 4: 
                desativarUva(); 
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
 * a um vinho recém-criado, inserindo na tabela 'composto_por'.
 */
void gerenciarUvasDoVinho(MYSQL *con, int id_vinho) {
    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;
    int id_uva_encontrada = -1;
    char resposta = 's';
    
    // Loop para adicionar uvas
    while (resposta == 's' || resposta == 'S') {
        limparTela();
        printf("\n--- Adicionar Uvas ao Vinho (ID: %d) ---\n", id_vinho);
        listarUvas(con);
        printf("Digite o NOME ou ID da uva (ou '0' para parar): ");
        scanf("%100[^\n]", input_buffer);
        while(getchar() != '\n');

        if (strcmp(input_buffer, "0") == 0) {
            break; // Usuário terminou de adicionar uvas
        }
        
        // Lógica: Tenta ler como ID, se falhar, lê como NOME
        id_digitado = strtol(input_buffer, &end_ptr, 10);
        id_uva_encontrada = -1;

        if (end_ptr == input_buffer) { // É um NOME
            id_uva_encontrada = obter_id_pelo_nome(con, "uva", "id_uva", "nome_uva", input_buffer);
        } else if (*end_ptr == '\0') { // É um ID
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

        // Se não encontrou (e era um nome), pergunta se quer criar
        if (id_uva_encontrada == -1 && end_ptr == input_buffer) {
             printf("Uva '%s' nao encontrada. Deseja cadastra-la? (s/n): ", input_buffer);
             scanf(" %c", &resposta);
             while(getchar() != '\n');
             if (resposta == 's' || resposta == 'S') {
                 id_uva_encontrada = (int)criar_item_retornando_id(con, "uva", "nome_uva", input_buffer);
                 if (id_uva_encontrada == -1) {
                     fprintf(stderr, "Erro ao cadastrar nova uva. Tente novamente.\n");
                     pausarTela();
                     continue; // Reinicia o loop
                 }
             }
        }

        // Se temos um ID válido, insere na tabela 'composto_por'
        if (id_uva_encontrada != -1) {
            MYSQL_STMT *stmt_junction;
            MYSQL_BIND bind_junction[2];
            char query_junction[] = "INSERT INTO composto_por (fk_vinho_id_vinho, fk_uva_id_uva) VALUES (?, ?)";

            stmt_junction = mysql_stmt_init(con);
            if (!stmt_junction) { 
                fprintf(stderr, " (composto_por) mysql_stmt_init() falhou\n");
                continue; // Pula para a próxima iteração do loop
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
                continue; // Pula para a próxima iteração do loop
            }
            
            if (mysql_stmt_execute(stmt_junction)) {
                if (mysql_errno(con) == 1062) { // 1062 = Entrada Duplicada
                    printf("\n... Esta uva ja foi adicionada a este vinho.\n");
                } else {
                    fprintf(stderr, "\nErro ao adicionar uva ao vinho: %s\n", mysql_stmt_error(stmt_junction));
                }
            } else {
                printf("\n... Uva (ID: %d) adicionada ao vinho (ID: %d) com sucesso!\n", id_uva_encontrada, id_vinho);
                //registro de LOG
                char log_msg[256];
                sprintf(log_msg, "ASSOCIACAO VINHO-UVA: Vinho ID %d <- Uva ID %d", id_vinho, id_uva_encontrada);
                registrarLog(log_msg);
            }
            mysql_stmt_close(stmt_junction);
        }
        
        // Pergunta se quer adicionar MAIS uvas
        printf("\nDeseja adicionar outra uva a este vinho? (s/n): ");
        scanf(" %c", &resposta);
        while(getchar() != '\n');
    }
}

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
        scanf("%d", &opcao);
        while(getchar() != '\n');

        switch (opcao) {
            case 1: 
                reativarVinho();
                pausarTela(); 
                break;
            case 2: 
                reativarTipo();
                pausarTela(); 
                break;
            case 3: 
                reativarPais();
                pausarTela(); 
                break;
            case 4: 
                reativarUva(); 
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
// FUNÇÕES DE REATIVAÇÃO: VINHO
// ==========================================================

/*
 * Lista os vinhos desativados (ativo = 0)
 * (Nota: Esta função ACEITA uma conexão)
 */
void listarVinhosDesativados(MYSQL *con) {
    char query[] = 
        "SELECT "
        "   v.id_vinho, v.nome_vinho, v.safra, v.preco, "
        "   IFNULL(t.nome_tipo, 'N/A'), "
        "   IFNULL(p.nome_pais, 'N/A') "
        "FROM vinho AS v "
        "LEFT JOIN tipo AS t ON v.fk_tipo_id_tipo = t.id_tipo "
        "LEFT JOIN paisorigem AS p ON v.fk_paisorigem_id_paisorigem = p.id_paisorigem "
        "WHERE v.ativo = 0 " // Filtro de inativos
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
    int num_rows = mysql_num_rows(result);

    printf("\n>>> Lista de Vinhos DESATIVADOS <<<\n");
    if (num_rows == 0) {
        printf("Nenhum vinho desativado encontrado.\n");
    } else {
        printf("%-3s | %-25s | %-5s | %-10s | %-15s | %-15s\n", 
               "ID", "Nome", "Safra", "Preco", "Tipo", "Pais");
        printf("--------------------------------------------------------------------------------\n");

        while ((row = mysql_fetch_row(result))) {
            printf("%-3s | %-25s | %-5s | %-10s | %-15s | %-15s\n",
                   row[0], row[1], row[2], row[3], row[4], row[5]);
        }
        printf("--------------------------------------------------------------------------------\n");
    }
    mysql_free_result(result);
}

/*
 * Função principal para Reativar um Vinho
 */
void reativarVinho() {
    int id_busca;
    MYSQL *con = conectar_db(); // Abre/fecha a própria conexão
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    
    char query[] = "UPDATE vinho SET ativo = 1 WHERE id_vinho = ?";

    // 1. Listar vinhos desativados PRIMEIRO
    listarVinhosDesativados(con);

    // 2. Coletar ID
    printf("\nDigite o ID do Vinho que deseja REATIVAR (ou 0 para cancelar): ");
    scanf("%d", &id_busca);
    while(getchar() != '\n');

    if (id_busca == 0) {
        printf("Operacao cancelada.\n");
        mysql_close(con);
        return;
    }

    // 3. Preparar
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

    // 4. Bind
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&id_busca;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    // 5. Executar
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
    } else {
        if (mysql_stmt_affected_rows(stmt) > 0) {
            printf("\nVinho ID %d reativado com sucesso!\n", id_busca);
            //registro de LOG
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

// ==========================================================
// FUNÇÕES DE REATIVAÇÃO: TIPO
// ==========================================================

/*
 * Lista os Tipos desativados (ativo = 0)
 */
void listarTiposDesativados(MYSQL *con) {
    if (mysql_query(con, "SELECT id_tipo, nome_tipo FROM tipo WHERE ativo = 0")) {
        fprintf(stderr, "Erro ao listar tipos desativados: %s\n", mysql_error(con));
        return; 
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        fprintf(stderr, "Erro ao obter resultados (tipos desativados): %s\n", mysql_error(con));
        return;
    }

    MYSQL_ROW row;
    int num_rows = mysql_num_rows(result);

    printf("\n--- Lista de Tipos DESATIVADOS ---\n");
    if (num_rows == 0) {
        printf("Nenhum tipo desativado encontrado.\n");
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
 * Função principal para Reativar um Tipo
 */
void reativarTipo() {
    int id_busca;
    MYSQL *con = conectar_db();
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    
    char query[] = "UPDATE tipo SET ativo = 1 WHERE id_tipo = ?";

    listarTiposDesativados(con);

    printf("\nDigite o ID do Tipo que deseja REATIVAR (ou 0 para cancelar): ");
    scanf("%d", &id_busca);
    while(getchar() != '\n');

    if (id_busca == 0) {
        printf("Operacao cancelada.\n");
        mysql_close(con);
        return;
    }

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
            printf("\nTipo ID %d reativado com sucesso!\n", id_busca);
            //registro de LOG
            char log_msg[256];
            sprintf(log_msg, "REATIVOU TIPO: ID %d", id_busca);
            registrarLog(log_msg);
        } else {
            printf("\nTipo com ID %d nao foi encontrado na lista de inativos.\n", id_busca);
        }
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}

// ==========================================================
// FUNÇÕES DE REATIVAÇÃO: PAÍS
// ==========================================================

/*
 * Lista os Países desativados (ativo = 0)
 */
void listarPaisesDesativados(MYSQL *con) {
    if (mysql_query(con, "SELECT id_paisorigem, nome_pais FROM paisorigem WHERE ativo = 0")) {
        fprintf(stderr, "Erro ao listar paises desativados: %s\n", mysql_error(con));
        return; 
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        fprintf(stderr, "Erro ao obter resultados (paises desativados): %s\n", mysql_error(con));
        return;
    }

    MYSQL_ROW row;
    int num_rows = mysql_num_rows(result);

    printf("\n--- Lista de Paises DESATIVADOS ---\n");
    if (num_rows == 0) {
        printf("Nenhum pais desativado encontrado.\n");
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
 * Função principal para Reativar um País
 */
void reativarPais() {
    int id_busca;
    MYSQL *con = conectar_db();
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    
    char query[] = "UPDATE paisorigem SET ativo = 1 WHERE id_paisorigem = ?";

    listarPaisesDesativados(con);

    printf("\nDigite o ID do Pais que deseja REATIVAR (ou 0 para cancelar): ");
    scanf("%d", &id_busca);
    while(getchar() != '\n');

    if (id_busca == 0) {
        printf("Operacao cancelada.\n");
        mysql_close(con);
        return;
    }

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
            printf("\nPais ID %d reativado com sucesso!\n", id_busca);
            //registro de LOG
            char log_msg[256];
            sprintf(log_msg, "REATIVOU PAIS: ID %d", id_busca);
            registrarLog(log_msg);
        } else {
            printf("\nPais com ID %d nao foi encontrado na lista de inativos.\n", id_busca);
        }
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}

// ==========================================================
// FUNÇÕES DE REATIVAÇÃO: UVA
// ==========================================================

/*
 * Lista as Uvas desativadas (ativo = 0)
 */
void listarUvasDesativadas(MYSQL *con) {
    if (mysql_query(con, "SELECT id_uva, nome_uva FROM uva WHERE ativo = 0")) {
        fprintf(stderr, "Erro ao listar uvas desativadas: %s\n", mysql_error(con));
        return; 
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        fprintf(stderr, "Erro ao obter resultados (uvas desativadas): %s\n", mysql_error(con));
        return;
    }

    MYSQL_ROW row;
    int num_rows = mysql_num_rows(result);

    printf("\n--- Lista de Uvas DESATIVADAS ---\n");
    if (num_rows == 0) {
        printf("Nenhuma uva desativada encontrada.\n");
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
 * Função principal para Reativar uma Uva
 */
void reativarUva() {
    int id_busca;
    MYSQL *con = conectar_db();
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    
    char query[] = "UPDATE uva SET ativo = 1 WHERE id_uva = ?";

    listarUvasDesativadas(con);

    printf("\nDigite o ID da Uva que deseja REATIVAR (ou 0 para cancelar): ");
    scanf("%d", &id_busca);
    while(getchar() != '\n');

    if (id_busca == 0) {
        printf("Operacao cancelada.\n");
        mysql_close(con);
        return;
    }

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
            printf("\nUva ID %d reativada com sucesso!\n", id_busca);
            //registro de LOG
            char log_msg[256];
            sprintf(log_msg, "REATIVOU UVA: ID %d", id_busca);
            registrarLog(log_msg);
        } else {
            printf("\nUva com ID %d nao foi encontrada na lista de inativos.\n", id_busca);
        }
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
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
        scanf("%d", &opcao);
        while(getchar() != '\n');

        switch (opcao) {
            case 1: 
                registrarSaida(); // Função de Venda
                pausarTela(); 
                break;
            case 2: 
                // Futuramente, você pode criar a função registrarEntrada()
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

    MYSQL *con = NULL;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[3];

    // Buffers para validação
    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;

    // --- 1. MOSTRAR OS VINHOS ---
    limparTela();
    printf("\n--- REGISTRAR SAIDA (VENDA) ---\n");
    listarVinhos(); // Mostra os vinhos (abre/fecha conexão própria)

    // --- 2. COLETAR E VALIDAR O ID DO VINHO ---
    printf("\nDigite o ID do Vinho a ser vendido (ou 0 para cancelar): ");
    scanf("%100[^\n]", input_buffer);
    while(getchar() != '\n'); 

    id_digitado = strtol(input_buffer, &end_ptr, 10);
    if (end_ptr == input_buffer || *end_ptr != '\0') {
        printf("\nErro: Entrada invalida. Apenas numeros sao permitidos.\n");
        return;
    }
    if (id_digitado == 0) {
        printf("Operacao cancelada.\n");
        return;
    }
    id_vinho = (int)id_digitado;

    // --- 3. COLETAR E VALIDAR QUANTIDADE ---
    printf("Digite a quantidade a ser vendida: ");
    scanf("%100[^\n]", input_buffer);
    while(getchar() != '\n'); 

    id_digitado = strtol(input_buffer, &end_ptr, 10);
    if (end_ptr == input_buffer || *end_ptr != '\0' || id_digitado <= 0) {
        printf("\nErro: Quantidade invalida. Deve ser um numero maior que zero.\n");
        return;
    }
    quantidade_venda = (int)id_digitado;

    // --- 4. COLETAR NOME DO CLIENTE (Apenas para o resumo) ---
    printf("Digite o nome do Cliente (para o recibo): ");
    scanf("%100[^\n]", nome_cliente);
    while(getchar() != '\n'); 

    // --- 5. EXECUTAR TRANSAÇÃO ---
    con = conectar_db();
    
    // Precisamos buscar o preço e o nome do vinho ANTES de atualizar
    char nome_vinho[151];
    double preco_vinho = 0;
    int qtd_estoque = 0;
    
    // 5a. Buscar dados e verificar estoque
    char query_select[] = "SELECT nome_vinho, preco, quantidade FROM vinho WHERE id_vinho = ? AND ativo = 1";
    stmt = mysql_stmt_init(con);
    if (mysql_stmt_prepare(stmt, query_select, strlen(query_select))) { /* ... (erro) ... */ }
    
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&id_vinho;
    mysql_stmt_bind_param(stmt, bind);
    mysql_stmt_execute(stmt);
    
    mysql_stmt_store_result(stmt);
    if (mysql_stmt_num_rows(stmt) == 0) {
        printf("\nErro: Vinho com ID %d nao encontrado ou esta inativo.\n", id_vinho);
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    // Bind dos resultados
    MYSQL_BIND bind_out[3];
    memset(bind_out, 0, sizeof(bind_out));
    bind_out[0].buffer_type = MYSQL_TYPE_STRING;
    bind_out[0].buffer = nome_vinho;
    bind_out[0].buffer_length = 151;
    bind_out[1].buffer_type = MYSQL_TYPE_DOUBLE;
    bind_out[1].buffer = (char *)&preco_vinho;
    bind_out[2].buffer_type = MYSQL_TYPE_LONG;
    bind_out[2].buffer = (char *)&qtd_estoque;
    
    mysql_stmt_bind_result(stmt, bind_out);
    mysql_stmt_fetch(stmt);
    mysql_stmt_close(stmt); // Fecha o statement do SELECT

    // 5b. Validar estoque
    if (quantidade_venda > qtd_estoque) {
        printf("\nErro: Estoque insuficiente!\n");
        printf("Disponivel: %d | Solicitado: %d\n", qtd_estoque, quantidade_venda);
        mysql_close(con);
        return;
    }

    // 5c. Atualizar o estoque (UPDATE na tabela 'vinho')
    char query_update[] = "UPDATE vinho SET quantidade = (quantidade - ?) WHERE id_vinho = ?";
    stmt = mysql_stmt_init(con);
    if (mysql_stmt_prepare(stmt, query_update, strlen(query_update))) { /* ... (erro) ... */ }

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&quantidade_venda;
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *)&id_vinho;
    
    mysql_stmt_bind_param(stmt, bind);
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "\nErro ao atualizar o estoque: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }
    mysql_stmt_close(stmt); // Fecha o statement do UPDATE

    // 5d. Registrar a movimentação (INSERT na tabela 'movimentacao')
    char query_insert[] = "INSERT INTO movimentacao (tipo_movimentacao, quantidade_movimentada, fk_vinho_id_vinho, fk_usuario_id_usuario) VALUES ('SAIDA', ?, ?, ?)";
    stmt = mysql_stmt_init(con);
    if (mysql_stmt_prepare(stmt, query_insert, strlen(query_insert))) { /* ... (erro) ... */ }

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&quantidade_venda;
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *)&id_vinho;
    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer = (char *)&g_id_usuario_logado; // <-- ID do usuário logado

    mysql_stmt_bind_param(stmt, bind);
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "\nErro ao registrar a movimentacao: %s\n", mysql_stmt_error(stmt));
        // (Nota: Em um sistema real, aqui você daria um ROLLBACK)
    }
    mysql_stmt_close(stmt); // Fecha o statement do INSERT

    //registro de LOG
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

    // CORREÇÃO: Adiciona a limpeza em caso de falha no bind
    if (mysql_stmt_bind_param(stmt, bind)) { 
        // Falha silenciosa
        mysql_stmt_close(stmt);
        mysql_close(con_log);
        return;
    }

    // CORREÇÃO: Adiciona a limpeza em caso de falha no execute
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
    
    // Query com JOIN para pegar o nome do usuário
    char query[] = "SELECT l.id_log, l.data_hora, l.acao, u.nome_usuario "
                   "FROM log AS l "
                   "JOIN usuario AS u ON l.fk_usuario_id_usuario = u.id_usuario "
                   "ORDER BY l.data_hora DESC " // Mais recentes primeiro
                   "LIMIT 50"; // Limita aos últimos 50 logs

    if (mysql_query(con, query)) {
        fprintf(stderr, "Erro ao listar logs: %s\n", mysql_error(con));
        mysql_close(con);
        return;
    }

    MYSQL_RES *result = mysql_store_result(con);
    // CORREÇÃO: Adiciona a mensagem de erro
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
            printf("%-5s | %-20s | %-15s | %-50s\n",
                   row[0], row[1], row[3], row[2]); // Ordem: ID, Data, Usuario, Acao
        }
        printf("--------------------------------------------------------------------------------------------------\n");
    }
    mysql_free_result(result);
    mysql_close(con);
}

void listarTodosLogs() {
    registrarLog("VISUALIZOU LOGS (TODOS)");
    MYSQL *con = conectar_db();
    
    // Query com JOIN (SEM LIMITE)
    char query[] = "SELECT l.id_log, l.data_hora, l.acao, u.nome_usuario "
                   "FROM log AS l "
                   "JOIN usuario AS u ON l.fk_usuario_id_usuario = u.id_usuario "
                   "ORDER BY l.data_hora DESC"; // Mais recentes primeiro

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
            printf("%-5s | %-20s | %-15s | %-50s\n",
                   row[0], row[1], row[3], row[2]); // Ordem: ID, Data, Usuario, Acao
        }
        printf("--------------------------------------------------------------------------------------------------\n");
    }
    mysql_free_result(result);
    mysql_close(con);
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
        printf("1. Listar Logs (Ultimos 50)\n"); // <-- Texto atualizado
        printf("2. Listar Logs (Todos)\n");  // <-- NOVA OPÇÃO
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);
        while(getchar() != '\n');

        switch (opcao) {
            case 1: 
                listarLogs(); // A função antiga que lista 50
                pausarTela(); 
                break;
            case 2:
                listarTodosLogs(); // A nova função que lista tudo
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