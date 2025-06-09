# solicita o cpf do usuário
cpf_digitado = input('Digite o seu CPF (Exemplo 000.000.000-00): ')

# Digito 1 
# contador para a contagem regressiva
contador_1 = 10

# lista que vai guardar os numeros
numeros_cpf = []
# for que vai pegar apenas os numeros
for numero in cpf_digitado:
    if numero .isdigit():
        numeros_cpf.append(int(numero))

#nove primeiros digitos do cpf
digitos = numeros_cpf[:9]

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
digitos.append(int(digito_1))

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
digitos.append(int(digito_2))

# verifica se o cpf é válido
if digitos == numeros_cpf:
    print('O CPF digitado é válido!')
else:
    print('CPF inválido!')

