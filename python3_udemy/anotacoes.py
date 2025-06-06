# usado para fazer um comentario de uma linha

"""
DocString = usado para comentarios multilinha, o Python lê a DocString,
mas não faz nada com as informações lidas.
"""

"""
# str = string = textos que estão dentro de aspas Ex: "Henrique" ou 'Henrique'
# Tipos int e Float
# int > Numero inteiro > representa qualquer numero positivo ou negativo, 
# int sem sinal é considerado positivo.
print('tipos int = numeros inteiros')
print(11) # int
print (-11) # int
print (0) #int
"""

"""
# float > Numero com ponto flutuante (casa decimal)
# O tipo float representa qualquer numero positivo ou negativo,
# sem sinal é considerado positivo, usar ponto (.) para separar casa decimal
print('tipos float = ponto flutuante (numeros com casa decimal)')
print(1.1, 10.11)
print(0.0, -1.5)
"""

"""
# A função type mostra o tipo que o Python inferiu ao valor.
print('a função type mostra o tipo que o Python inferiu ao valor')
print(type("Henrique"))
print(type(0))
print(type(1.1), type(-1.1), type(0.0))
"""

"""
# CTRL+; comenta a linha selecionada
"""

"""
# Tipo de dado bool (boolean)
# usada ao questionar algo em um programa
# Só existem 2 respostas sim (True) ou não (False)
# Existem varios operadores para "questionar"
# Dentre eles o ==, que é um operador lógico que questiona se um valor é igual a outro
print('tipo booleano, True ou False')
print(10 == 10) # Sim > True 
print(10 == 11) #Não > False 
"""

"""
# conversão de tipo, coerção, type convertion, typecasting, coercion
# é o ato de converter um tipo em outro
# tipos imutáveis e primitivos:
# str, int, float, bool
print('conversões de tipos')
print(int('1'), type(int('1')))
print(type(float('1')+1))
print(bool(''), (str('ou')), bool(' '))
"""

"""
#operadores aritméticos
adicao = 10 + 10

subtracao = 10 - 5

multiplicacao = 10 * 10

#divisao com numeros quebrados ou inteiros, sempre retorna em float
divisao = 10 / 2.2

#divisao inteira (sempre trunca/arredonda o resultado)
divisao_inteira = 10 // 2.2

#potenciacao
exponenciacao = 2 ** 10

#retorna o resto da divisao
modulo = 55 % 2 

#concatenacao, juntar 2 strings
concatenacao = "A" + "B"

#repeticao de strings
a_dez_vezes = 'A' * 10
tres_vezes_henrique = 3 * 'Henrique'
"""

"""
#ordem de execução aritmética
#1. (n + n) os parenteses sempre são executados de dentro para fora ex: (4(3(2(1))))
#2. **
#3. * / // % em operações de mesmo "nivel" é executado da esquerda para a direita
#4. + -
conta_1 = 1 + 1 ** 5 + 5 = 7
conta_1_ordenada = (1 + 1) ** (5 + 5) = 1024
"""

"""
#solicitar dados do usuario, o programa para na linha até o usuario interagir, 
# saida sempre é no tipo string
# input('Qual o seu nome? ')
# numero_1 = input ('Digite um numero: ')
# int_numero_1 = int(numero_1)
# int_numero_2 = int(numero_2)
# print(f'A soma dos números é: {numero_1 + numero_2})'
"""

"""
#blocos de código condicionais
#if  / elif      / else
#se  /se não se  / se não
#Ex:

#entrada = input('Você quer "entrar" ou "sair" do sistema? ')

#if entrada == 'entrar':    (se atender a esta condição executa o codigo abaixo)
#   print('Você entrou no sistema')
#   print('xdxdxdxdxdxdxd')
#elif entrada == 'sair':     (se nao atender a primeira condição e atender a 
# segunda executa o codigo abaixo, podemos ter varios elif no mesmo bloco)
#   print('Você saiu do sistema')
#else:          (se nao atender nenhuma das condições executa o codigo abaixo)
#   print('Você não digitou nem entrar e nem sair.')

# extras: podemos ter varios elif para ter mais condições dentro do mesmo if
# podemos digitar pass para o código seguir e ignorar aquela condição
# a primeira condição que atender executa o codigo relacionado e pula todas as
# outras condições do mesmo if
""" 

"""
#Operadores de comparação (relacionais)
#OP         Significado         Exemplo (True)
#>          maior               2 > 1
#>=         maior ou igual      2 >= 2
#<          menor               1 < 2
#<=         menor ou igual      2 <= 2
#==         igual               'a' == 'a'
#!=         diferente           'a' != 'b'
"""

"""
#Operadores lógicos
# and (e) or (ou) not (não)
# and - Todas as condições precisam ser verdadeiras, se qualquer valor for
# considerado falso, a expressão inteira será avaliada naquele valor.
# São considerados falsy:
# 0 0.0 '' False
# Também existe o tipo None que é usado para representar um não valor.

# or - Qualquer condição verdadeira avalia toda a expressão como verdadeira.
# Ex: if (entrada == 'E' or entrada == 'e') and senha_digitada == senha_permitida:
#       print('Entrar')
#     else:
#       print('Sair')

# not - Utilizado para inverter expressões
# not True = False
# not False = True
"""

"""
#Operadores in e not in
#Strings são iteraveis, pode utilizar o in para verificar se um ou mais valores estão
#  dentro de uma variavel, not in faz o contrario do in
# Ex: nome = 'Henrique'
#     print ('H' in nome)  >>>> retorna True, pois o H esta dentro do nome Henrique
"""

"""
# Interpolação basica de strings
# s - string
# d e i - int
# f - float
# x e X - Hexadecimal (ABCDEF0123456789)
# 
"""

"""
# Formatação básica de strings 
# s - string
# d - int
# f - float
# .<numero de digitos>f
# x ou X - Hexadecimal
# (Caractere)(><^)(quantidade)
# > - Esquerda
# < - Direita
# ^ - Centro
# Sinal - + ou -
# Ex.: 0>-100,.1f
# Conversion flags !r !s !a

# variavel = 'ABC'

# print(f'{variavel:$>10}')  - joga o valor da variavel para a direita e 
# adiciona o simbolo $ a esquerda até chegar no numero indicado 

# print(f'{variavel:&<10}')  - joga o valor da variavel para a esquerda e 
# adiciona o simbolo & a direita até chegar no numero indicado 

# print(f'{variavel:#^10}')  - joga o valor da variavel para o centro e 
# adiciona o simbolo # a direita e esquerda até chegar no numero indicado 
"""

"""
#Fatiamento de strings
#Fatiamento [i:f:p] [::]
#len(variavel) >> retorna a quantidade de caracteres da str

#variavel = henrique
#print(variavel[0]) >> retorna a letra h
#print(variavel[-1]) >> retorna a letra e do fim de henrique
#print(variavel[::-1]) >> retorna o nome invertido
#print(len(variavel)) >> retorna a qtd de caracteres no exemplo sao 8
"""

"""
#Introdução ao try/except
#try >> tenta executar o código
#except >> ocorreu algum erro ao tentar executar
#Ex:. numero_str = input ('Vou dobrar o numero que voce digitar: ')
#
#try:  << tenta realizar o try, se ocorrer algum erro pula para o except
#   numero_float = float(numero_str)
#   print('FLOAT:', numero_float)
#   print(f'O dobro de {numero_str} é {numero_float * 2:.2f}')
#except:
#   print('Isso não é um numero')
"""

"""
#CONSTANTE = "Variáveis" que não vão mudar
#muitas condições no mesmo if (ruim)
# < contagem de complexidade (ruim)
#
#velocidade = 61 # velocidade atual do carro
#local_carro = 101 # local em que o carro está na estrada

#RADAR_1 = 60 # velocidade máxima do radar 1
#LOCAL_1 = 100 #loca onde o radar 1 está
#RADAR_RANGE = 1 # A distância onde o radar pega
"""

"""
#Identidade do valor que esta na memoria
#Flag (Bandeira) - Marcar um local
#None = Não Valor
#is e is not = é ou não é (tipo, valor, identidade)
#id = Identidade
"""

