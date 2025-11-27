#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h> //conexao com o MYSQL

// --- Informações de Conexão ---
// Modifique com os dados do seu banco de dados
const char *server = "localhost";
const char *user = "root";
const char *password = "@vintagem123";
const char *database = "vintagem";

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

// Funcao auxiliar para tratar erros do MySQL
void finish_with_error(MYSQL *con) {
  fprintf(stderr, "Erro Fatal: %s\n", mysql_error(con));
  mysql_close(con);
  exit(1); // Sai do programa em caso de erro fatal de BD
}

int main() {
    int opcao;
    mysql_library_init(0, NULL, NULL);

    do {
        limparTela();
        printf("\n--- ADEGA (CRUD DE VINHOS) ---\n");
        printf("--- Vinhos ---\n");
        printf("1. Adicionar Vinho\n");
        printf("2. Listar Vinhos (Ativos)\n");
        printf("3. Atualizar Vinho\n");
        printf("4. Desativar Vinho\n");
        printf("\n--- Gerenciamento ---\n");
        printf("5. Gerenciar Tipos\n");
        printf("6. Gerenciar Paises\n");
        printf("7. Gerenciar Uvas\n");
        printf("\n0. Sair\n");
        printf("Escolha uma das opcoes acima: ");
        
        scanf("%d", &opcao);
        while(getchar() != '\n');

        switch (opcao) {
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
    int num_rows = mysql_num_rows(result); // Pega o número de linhas

    printf("\n>>> Lista de Vinhos (Apenas Ativos) <<<\n");
    if (num_rows == 0) {
        printf("Nenhum vinho ativo cadastrado.\n");
    } else {
        printf("%-5s %-30s %-5s %-10s %-5s\n", "ID", "Nome", "Safra", "Preco", "Qtd.");
        printf("-------------------------------------------------------------------\n");

        while ((row = mysql_fetch_row(result))) {
            printf("%-5s %-30s %-5s %-10s %-5s\n",
                   row[0] ? row[0] : "NULL",
                   row[1] ? row[1] : "NULL",
                   row[2] ? row[2] : "NULL",
                   row[3] ? row[3] : "NULL",
                   row[4] ? row[4] : "NULL");
        }
        printf("-------------------------------------------------------------------\n");
    }

    mysql_free_result(result);
    mysql_close(con);
}

//UPDATE
void atualizarVinho() {
    int id_busca;
    Vinho v; // Struct para armazenar os novos dados
    MYSQL *con = conectar_db();
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[7]; // 6 campos SET + 1 ID para o WHERE
    char query[] = "UPDATE vinho SET nome_vinho = ?, safra = ?, preco = ?, quantidade = ?, fk_tipo_id_tipo = ?, fk_paisorigem_id_paisorigem = ? WHERE id_vinho = ?";

    // Buffers para os nomes
    char input_buffer[101];
    long id_digitado = 0;
    char *end_ptr;
    int id_tipo_encontrado;
    int id_pais_encontrado;
    char resposta;

    // 1. Coletar ID do Vinho
    printf("Digite o ID do Vinho que deseja atualizar: ");
    scanf("%d", &id_busca);
    while(getchar() != '\n'); 

    printf("\n>>> Insira os NOVOS dados para o Vinho ID %d <<<\n", id_busca);
    
    // 2. Coletar dados básicos
    printf("Novo Nome: ");
    scanf("%150[^\n]", v.nome_vinho);
    while(getchar() != '\n');

    printf("Nova Safra (Ano): ");
    scanf("%d", &v.safra);

    printf("Novo Preco: ");
    scanf("%lf", &v.preco);

    printf("Nova Quantidade: ");
    scanf("%d", &v.quantidade);
    while(getchar() != '\n'); // Limpa buffer

    // --- 3. Coletar e Validar NOVO Tipo ---
    printf("\n--- Selecao do NOVO Tipo ---\n");
    listarTipos(con); // Mostra a lista de tipos existentes
    printf("Digite o NOME do novo Tipo: ");
    scanf("%100[^\n]", input_buffer);
    while(getchar() != '\n');
    
    id_digitado = strtol(input_buffer, &end_ptr, 10);
    
    if (end_ptr == input_buffer) {
        id_tipo_encontrado = obter_id_pelo_nome(con, "tipo", "id_tipo", "nome_tipo", input_buffer);
    } else if (*end_ptr == '\0') {
        if (verificar_id_ativo(con, "tipo", "id_tipo", (int)id_digitado)) {
            id_tipo_encontrado = (int)id_digitado;
        } else {
            id_tipo_encontrado = -1;
        }
    } else {
        id_tipo_encontrado = -1;
    }

    if (id_tipo_encontrado == -1) {
        if (end_ptr == input_buffer) { 
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
        } else {
            printf("ID ou entrada invalida. Atualizacao abortada.\n");
            mysql_close(con);
            return;
        }
    }
    v.fk_tipo_id_tipo = id_tipo_encontrado; // Armazena o ID encontrado

    // --- 4. Coletar e Validar NOVO País ---
    printf("\n--- Selecao do NOVO Pais ---\n");
    listarPaises(con); // Mostra a lista de países existentes
    printf("Digite o NOME do novo Pais de Origem: ");
    scanf("%100[^\n]", input_buffer);
    while(getchar() != '\n');
    
    id_digitado = strtol(input_buffer, &end_ptr, 10);

    if (end_ptr == input_buffer) {
        id_pais_encontrado = obter_id_pelo_nome(con, "paisorigem", "id_paisorigem", "nome_pais", input_buffer);
    } else if (*end_ptr == '\0') {
        if (verificar_id_ativo(con, "paisorigem", "id_paisorigem", (int)id_digitado)) {
            id_pais_encontrado = (int)id_digitado;
        } else {
            id_pais_encontrado = -1;
        }
    } else {
        id_pais_encontrado = -1;
    }

    if (id_pais_encontrado == -1) {
        if (end_ptr == input_buffer) { 
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
        } else {
            printf("ID ou entrada invalida. Atualizacao abortada.\n");
            mysql_close(con);
            return;
        }
    }
    v.fk_paisorigem_id_paisorigem = id_pais_encontrado; // Armazena o ID encontrado


    // --- 5. Preparar e Executar o UPDATE ---
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

    // 6. Associar (Bind) os 7 parâmetros
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
    // Param 4: fk_tipo (O ID que buscamos)
    bind[4].buffer_type = MYSQL_TYPE_LONG;
    bind[4].buffer = (char *)&v.fk_tipo_id_tipo;
    // Param 5: fk_pais (O ID que buscamos)
    bind[5].buffer_type = MYSQL_TYPE_LONG;
    bind[5].buffer = (char *)&v.fk_paisorigem_id_paisorigem;
    // Param 6: id_vinho (O ID do WHERE)
    bind[6].buffer_type = MYSQL_TYPE_LONG;
    bind[6].buffer = (char *)&id_busca;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return; 
    }

    // 7. Executar
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    // 8. Verificar se funcionou
    if (mysql_stmt_affected_rows(stmt) > 0) {
        printf("\n>>> Vinho ID %d atualizado com sucesso! <<<\n", id_busca);
    } else {
        printf("\n>>> Vinho com ID %d nao foi encontrado (nenhuma alteracao feita). <<<\n", id_busca);
    }

    mysql_stmt_close(stmt); // Fecha o statement do UPDATE

    // --- 9. NOVO: Gerenciar Uvas ---
    printf("\n--- Gerenciamento de Uvas para o Vinho ID %d ---\n", id_busca);
    limparUvasDoVinho(con, id_busca); // Limpa as uvas antigas
    gerenciarUvasDoVinho(con, id_busca); // Adiciona as novas

    mysql_close(con);
}

//DELETE(DESATIVAR)
void desativarVinho() {
    int id_busca;
    MYSQL *con = conectar_db();
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    
    // A query mudou de DELETE para UPDATE
    char query[] = "UPDATE vinho SET ativo = 0 WHERE id_vinho = ?";

    // 1. Coletar ID
    printf("Digite o ID do Vinho que deseja DESATIVAR: "); // Mensagem atualizada
    scanf("%d", &id_busca);
    while(getchar() != '\n');

    // 2. Preparar
    stmt = mysql_stmt_init(con);
    // Tratamento de erro do init
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

    // 3. Associar (Bind)
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&id_busca;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, " mysql_stmt_bind_param() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    // 4. Executar
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(con);
        return;
    }

    // 5. Verificar
    if (mysql_stmt_affected_rows(stmt) > 0) {
        printf("Vinho desativado com sucesso!\n"); // Mensagem atualizada
    } else {
        printf("Vinho com ID %d nao foi encontrado.\n", id_busca);
    }

    // 6. Limpar
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
    }

    mysql_stmt_close(stmt);
    mysql_close(con);
}

void atualizarTipo() {
    MYSQL *con = conectar_db();
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];
    int id_tipo;
    char nome_tipo[51];
    char query[] = "UPDATE tipo SET nome_tipo = ? WHERE id_tipo = ?";

    printf("\n--- Atualizar Tipo ---\n");
    printf("ID do Tipo a atualizar: ");
    scanf("%d", &id_tipo);
    while(getchar() != '\n');

    printf("Novo Nome: ");
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
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *)&id_tipo;

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
            fprintf(stderr, "\nErro: O nome de tipo '%s' ja esta em uso.\n", nome_tipo);
        } else {
            // Outro erro qualquer
            fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        }
    } else {
        if (mysql_stmt_affected_rows(stmt) > 0) {
            printf("Tipo atualizado com sucesso!\n");
        } else {
            printf("ID do Tipo nao encontrado ou nenhum dado foi alterado.\n");
        }
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}

void desativarTipo() {
    MYSQL *con = conectar_db();
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    int id_tipo;
    char query[] = "UPDATE tipo SET ativo = 0 WHERE id_tipo = ?";

    printf("\n--- Desativar Tipo ---\n");
    printf("ID do Tipo a desativar: ");
    scanf("%d", &id_tipo);
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
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&id_tipo;

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
    } else {
        // Se a execução foi SUCESSO, verifica as linhas
        if (mysql_stmt_affected_rows(stmt) > 0) {
            printf("Tipo desativado com sucesso!\n");
        } else {
            printf("ID do Tipo nao encontrado ou ja esta desativado.\n");
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
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}

void atualizarPais() {
    MYSQL *con = conectar_db();
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];
    int id_pais;
    char nome_pais[101];
    char query[] = "UPDATE paisorigem SET nome_pais = ? WHERE id_paisorigem = ?";

    printf("\n--- Atualizar Pais ---\n");
    printf("ID do Pais a atualizar: ");
    scanf("%d", &id_pais);
    while(getchar() != '\n');

    printf("Novo Nome: ");
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
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *)&id_pais;

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
            fprintf(stderr, "\nErro: O nome de pais '%s' ja esta em uso.\n", nome_pais);
        } else {
            // Outro erro qualquer
            fprintf(stderr, " mysql_stmt_execute() falhou: %s\n", mysql_stmt_error(stmt));
        }
    } else {
        if (mysql_stmt_affected_rows(stmt) > 0) {
            printf("Pais atualizado com sucesso!\n");
        } else {
            printf("ID do Pais nao encontrado ou nenhum dado foi alterado.\n");
        }
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}

void desativarPais() {
    MYSQL *con = conectar_db();
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    int id_pais;
    char query[] = "UPDATE paisorigem SET ativo = 0 WHERE id_paisorigem = ?";

    printf("\n--- Desativar Pais ---\n");
    printf("ID do Pais a desativar: ");
    scanf("%d", &id_pais);
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
        return; // Sai da função
    }
    
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&id_pais;

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
    } else {
        // Se a execução foi SUCESSO, verifica as linhas
        if (mysql_stmt_affected_rows(stmt) > 0) {
            printf("Pais desativado com sucesso!\n");
        } else {
            printf("ID do Pais nao encontrado ou ja esta desativado.\n");
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
        } else {
            printf("ID da Uva nao encontrado ou nenhum dado foi alterado.\n");
        }
    }
    mysql_stmt_close(stmt);
    mysql_close(con);
}

void desativarUva() {
    MYSQL *con = conectar_db();
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    int id_uva;
    char query[] = "UPDATE uva SET ativo = 0 WHERE id_uva = ?";

    printf("\n--- Desativar Uva ---\n");
    printf("ID da Uva a desativar: ");
    scanf("%d", &id_uva);
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
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&id_uva;

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
            printf("Uva desativada com sucesso!\n");
        } else {
            printf("ID da Uva nao encontrado ou ja esta desativada.\n");
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
            }
            mysql_stmt_close(stmt_junction);
        }
        
        // Pergunta se quer adicionar MAIS uvas
        printf("\nDeseja adicionar outra uva a este vinho? (s/n): ");
        scanf(" %c", &resposta);
        while(getchar() != '\n');
    }
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