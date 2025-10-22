//1 #include <stdio.h>

// int main(void)
// {
//     int vetNum[5], soma = 0, i;

//     //solicitando os numeros
//     for (i = 0; i<5; i++)
//     {
//         printf("Digite um numero inteiro: \n");
//         scanf("%d", &vetNum[i]);

//         soma += vetNum[i];
//     }

//     printf("A soma dos numeros digitados é: %d \n", soma);

//     return 0;
// }

//2 #include <stdio.h>

// int main(void)
// {
//     float vetNum[5], soma = 0;
//     int i;

//     //solicitando os numeros
//     for (i = 0; i<5; i++)
//     {
//         printf("Digite um numero real: \n");
//         scanf("%f", &vetNum[i]);

//         soma += vetNum[i];
//     }

//     printf("A soma dos numeros digitados é: %.2f \n", soma);

//     return 0;
// }

//3 #include <stdio.h>

// int main(void)
// {
//     float vetNotas[4], soma = 0;
//     int i;

//     //solicitando os numeros
//     for (i = 0; i<4; i++)
//     {
//         printf("Digite a nota do aluno: \n");
//         scanf("%f", &vetNotas[i]);

//         soma += vetNotas[i];
//     }

//     printf("A media das notas digitadas é: %.2f \n", soma/4);

//     return 0;
// }

// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>

// int main()
// {
//     char palavra[3][15];
//     int i;

//     printf("Digitar 3 palavras!\n");

//     for (i = 0; i < 3; i++)
//     {
//         printf("Digite uma palavra: \n");
//         fflush(stdin);
//         gets(palavra[i]);
//     }

//     printf("As palavras digitadas foram: \n");

//     for (i = 2; i >= 0; i--)
//     {
//         printf("%s\n", palavra[i]);
//     }
//     return 0;    
// }

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    char palavra[2][15], comparar[9]="papagaio";
    int i;

    printf("Digite 2 palavras!\n");

    //loop para pegar as 2 palavras
    for (i = 0; i < 2; i++)
    {
        printf("Digite uma palavra: \n");
        fflush(stdin);
        gets(palavra[i]);
    }

    printf("As palavras digitadas foram: \n");
    //loop para exibir as 2 palavras
    for (i = 0; i < 2; i++)
    {
        printf("%s\n", palavra[i]);
    }
    
    //comparar as palavras
    if (strcmp(palavra[0], palavra[1]) == 0)
    {
        printf("As palavras são iguais!\n");
    }
    else
    {
        printf("As palavras são diferentes! \n");
    }

    //comparar as palavras com a palavra papagaio
    if (strcmp(palavra[0], comparar) == 0)
    {
        printf("A primeira palavra é igual a %s!\n", comparar);
    }
    else if (strcmp(palavra[1], comparar) == 0)
    {
        printf("A segunda palavra é igual a %s!\n", comparar);
    }
    else
    {
        printf("Nenhuma das 2 palavras é igual a %s! \n", comparar);
    }

    return 0;
}