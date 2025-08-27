# #olá mundo em python
# print('Olá Mundo!')

# #algoritmo que imprime meu nome na tela
# print('Olá, meu nome é Henrique Santana Frameschi')

# #algoritmo que solicita o nome do usuário
# nome = input('Digite o seu nome completo: ')
# print(f'Seja bem vindo(a) {nome}!')

# #algoritmo que soma os valores a, b, c
# a = 5
# b = 7
# c = 10
# print(f'A soma de {a}, {b} e {c} é: {a+b+c}')

# #algoritmo média
# #tenta realizar o algoritmo#
# try:
#     #solicita a nota 1, nota 2 e converte em float#
#     nota1 = float(input('Digite a primeira nota do aluno: '))
#     nota2 = float(input('Digite a segunda nota do aluno: '))
#     #realiza o calculo da média
#     media = (nota1+nota2)/2
#     #verifica se a media esta na condição de aprovado ou reprovado#
#     if media >= 6:
#         print(f'Sua média final foi: {media:.1f}, você está APROVADO!')
#     else:
#         print(f'Sua média final foi: {media:.1f}, infelizmente você está REPROVADO!')
# #se ocorrer um erro na conversao em float para o código e informa para digitar apenas numeros#
# except ValueError:
#     print('Digite apenas numeros!')




# #algoritmo calculos
# num1 = input('Digite um número inteiro: ')
# num2 = input('Digite outro número inteiro: ')

# if num1.isdigit() and num2.isdigit():
#     num1_int = int(num1)
#     num2_int = int(num2)
#     print(f'A soma de {num1_int} + {num2_int} é igual a: {num1_int + num2_int}')
#     print(f'A subtração de {num1_int} - {num2_int} é igual a: {num1_int - num2_int}')
#     print(f'A multiplicação de {num1_int} x {num2_int} é igual a: {num1_int * num2_int}')
#     print(f'A divisão de {num1_int} : {num2_int} é igual a : {num1_int / num2_int:.2f}')
#     print(f'O módulo de {num1_int} é {num1_int % 2} e o módulo de {num2_int} é {num2_int % 2}')
# else:
#     print('Digite apenas numeros inteiros!')

# #algoritmo par ou impar
# try:
#     numero = float(input('Digite um número: '))
#     resto = numero % 2
#     if resto == 0:
#         print(f'O número {numero} é par!')
#     else:
#         print(f'O número {numero} é impar!')
# except ValueError:
#     print('Digite apenas numeros!')

# #algoritmo dados do usuario
# nome = input('Digite o seu nome: ')
# idade = int(input('Digite qual a sua idade: '))
# altura = float(input('Digite a sua altura em metros: '))

# print(f'Olá {nome}, você tem {idade} anos e {altura} metros de altura')

# #algoritmo calculo idade
# ano_nascimento = int(input('Digite o ano do seu nascimento: '))
# idade = 2025 - ano_nascimento

# print(f'Você tem {idade} anos! Acertei?')


# #algoritmo area triangulo
# base = float(input('Digite o valor da base do triângulo: '))
# altura = float(input('Digite o valor da altura do triângulo: '))

# area = (base * altura)/2

# print(f'O valor da área do triângulo é: {area}')