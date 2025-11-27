#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Definindo a estrutura para um Vinho
typedef struct {
    int id_vinho;
    char nome_vinho[100];
    int volume;
    int ano;
    int teor_alcool;
    char tipo_vinho[50];
    int quantidade;
} Vinho;

//Prototipos das funcoes do CRUD
void criarVinho();
void listarVinhos();
void atualizarVinho();
void desativarVinho();

//nome do arquivo onde os dados serao armazenados
const char* FILENAME = "vinhos.dat";

int main() {
    int opcao;

    do {
        printf("\n--- CRUD DE VINHOS ---\n");
        printf("1. Adicionar Vinho\n");
        printf("2. Listar todos os Vinhos\n");
        printf("3. Atualizar Vinho\n");
        printf("4. Deletar Vinho\n");
        printf("0. Sair\n");
        printf("Escolha uma das opcoes acima: ");
        scanf("%d", &opcao);

        //limpar buffer de entrada para evitar erros no scanf
        while(getchar() != '\n');

        switch (opcao) {
            case 1:
                criarVinho();
                break;
            case 2:
                listarVinhos();
                break;
            case 3:
                atualizarVinho();
                break;
            case 4:
                desativarVinho();
                break;
            case 0:
                printf("Saindo do Programa...\n");
                break;
            default:
                printf("Opcao invalida! Tente Novamente.\n");    
        }
    } while (opcao != 0);

    return 0;
}

//CREATE
void criarVinho() {
    Vinho v;
    FILE *file;

    //"ab" = append binary. Abre para adicionar no final do arquivo \
    em modo binário.
    //Se o arquivo não existir, ele será criado.
    file = fopen(FILENAME, "ab");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        return;
    }

    printf("\n>>> Adicionar Novo Vinho <<<\n");
    printf("ID: ");
    scanf("%d", &v.id_vinho);

    //Limpeza do buffer
    while(getchar() != '\n');

    printf("Nome: ");
    //Le a string ate a quebra de linha, com limite de 99 caracteres
    scanf("%99[^\n]", &v.nome_vinho);

    printf("Volume: ");
    scanf("%d", &v.volume);

    printf("Ano: ");
    scanf("%d", &v.ano);

    printf("Teor Alcoolico: ");
    scanf("%d", &v.teor_alcool);

    //Limpeza do buffer
    while(getchar() != '\n');

    printf("Tipo do Vinho: ");
    scanf("%49[^\n]", &v.tipo_vinho);

    printf("Quantidade: ");
    scanf("%d", &v.quantidade);

    //Escreve a struc no arquivo
    fwrite(&v, sizeof(Vinho), 1, file);

    fclose(file);
    printf("Vinho adicionado com sucesso!\n");
}

//READ
void listarVinhos() {
    Vinho v;
    FILE *file;

    //"rb" = read binary. Abre o arquivo para leitura em modo binario.
    file = fopen(FILENAME, "rb");
    if (file == NULL) {
        printf("Nenhum vinho cadastrado ou erro ao abrir o arquivo.\n");
        return;
    }
    //ID, NOME_VINHO, VOLUME, ANO, TEOR ALCOOL, TIPO_VINHO, QUANTIDADE
    printf("\n>>> Lista de Vinhos <<<\n");
    printf("%-5s %-20s %-5s %-5s %-5s %-10s %-5s\n", "ID", "Nome", "Vol.", \
    "Ano", "Teor Alcool", "Tipo", "Qtd.");
    printf("----------------------------------\
-------------------------------\n");
    
    //Lê o arquivo registro por registro até o final
    while (fread(&v, sizeof(Vinho), 1, file)) {
        printf("%-5d %-20s %-5d %-5d %-11d %-10s %-5d\n", v.id_vinho, \
        v.nome_vinho, v.volume, v.ano, v.teor_alcool, v.tipo_vinho, \
        v.quantidade);
    }
    printf("----------------------------------\
-------------------------------\n");

    fclose(file);


}

//UPDATE
void atualizarVinho() {
    int id_busca;
    int encontrado = 0;
    Vinho v;
    FILE *file;

    printf("Digite o ID do Vinho que deseja atualizar: ");
    scanf("%d", &id_busca);

    //"rb+" = read/write binary. Abre o arquivo para ler e escrever em modo \
    binario.
    file = fopen(FILENAME, "rb+");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        return;
    }

    while (fread(&v, sizeof(Vinho), 1, file)) {
        if (v.id_vinho == id_busca) {
            encontrado = 1;
            printf("Vinho encontrado: %s\n", v.nome_vinho);

            //Limpeza de buffer
            while(getchar() != '\n');

            printf("Nome: ");
            //Le a string ate a quebra de linha, com limite de 99 caracteres
            scanf("%99[^\n]", &v.nome_vinho);

            printf("Volume: ");
            scanf("%d", &v.volume);

            printf("Ano: ");
            scanf("%d", &v.ano);

            printf("Teor Alcoolico: ");
            scanf("%d", &v.teor_alcool);

            //Limpeza do buffer
            while(getchar() != '\n');

            printf("Tipo do Vinho: ");
            scanf("%49[^\n]", &v.tipo_vinho);

            printf("Quantidade: ");
            scanf("%d", &v.quantidade);

            //Volta o ponteiro do arquivo para o inicio do registro atual
            fseek(file, -(long)sizeof(Vinho), SEEK_CUR);
            //Sobrescreve o registro com os novos dados
            fwrite(&v, sizeof(Vinho), 1, file);

            printf("Vinho atualizado com sucesso!\n");
            break; //encerra o laço apos a atualização
        }
    }

    if(!encontrado) {
        printf("Vinho com ID %d nao foi encontrado.\n", id_busca);
    }

    fclose(file);
}

//DELETE
void desativarVinho() {
    int id_busca;
    int encontrado = 0;
    Vinho v;
    FILE *file, *tempFile;

    printf("Digite o ID do Vinho que deseja deletar: ");
    scanf("%d", &id_busca);

    file = fopen(FILENAME, "rb");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        return;
    }

    //Cria um arquivo temporario para escrita
    tempFile = fopen("temp.dat", "wb");
    if (tempFile == NULL) {
        printf("Erro ao criar o arquivo temporario!\n");
        fclose(file);
        return;
    }

    //Copia todos os produtos, exceto o que será deletado
    while (fread(&v, sizeof(Vinho), 1, file)){
        if (v.id_vinho != id_busca) {
            fwrite(&v, sizeof(Vinho), 1, tempFile);
        }
        else {
            encontrado = 1;
        }
    }

    fclose(file);
    fclose(tempFile);

    if (encontrado) {
        //Deleta o arquivo original
        remove(FILENAME);
        //Renomeia o arquivo temporario para o nome do original
        rename("temp.dat", FILENAME);
        printf("Vinho deletado com sucesso!\n");
    }
    else {
        //Se nao encontrou, apenas deleta o arquivo temporario
        remove("temp.dat");
        printf("Vinho com ID %d nao foi encontrado.\n", id_busca);
    }
} 