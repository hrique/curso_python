import random
import sys


digitos = []
# for que vai gerar os numeros do cpf
for i in range(9):
    digitos.append(random.randint(0, 9))

contador_1 = 10

# variavel para guardar a soma do cpf
cpf_soma_1 = 0
# for que vai pegar os numeros do cpf e multiplicar pelo contador
for numeros in digitos:
    cpf_soma_1 += (numeros * contador_1)
    contador_1 -= 1

# multiplicação e divisão do resultado da soma do cpf
cpf_mult_1 = cpf_soma_1 * 10 % 11

# condicao para definir o digito do cpf
condicao_digito_1 = cpf_mult_1 <= 9

# define o valor do primeiro digito verificador do cpf
digito_1 = cpf_mult_1 if condicao_digito_1 else 0

# Digito 2
contador_2 = 11

# adiciona o digito 1 na lista de digitos do cpf
digitos.append(digito_1)

# variavel para guardar a soma do cpf
cpf_soma_2 = 0
# for que vai pegar os numeros do cpf e multiplicar pelo contador
for numeros in digitos:
    cpf_soma_2 += (numeros * contador_2)
    contador_2 -= 1

# multiplicação e divisão do resultado da soma do cpf
cpf_mult_2 = cpf_soma_2 * 10 % 11

# condicao para definir o segundo digito do cpf
condicao_digito_2 = cpf_mult_2 <= 9

# define o valor do segundo digito verificador do cpf
digito_2 = cpf_mult_2 if condicao_digito_2 else 0

# adiciona o digito 2 na lista de digitos do cpf
digitos.append(digito_2)

for numero in digitos:
    print(numero, end='')