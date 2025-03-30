# usado para fazer um comentario de uma linha

"""
DocString = usado para comentarios multilinha, o Python lê a DocString,
mas não faz nada com as informações lidas.
"""

# str = string = textos que estão dentro de aspas Ex: "Henrique" ou 'Henrique'

# Tipos int e Float
# int > Numero inteiro > representa qualquer numero positivo ou negativo, 
# int sem sinal é considerado positivo.
print('tipos int = numeros inteiros')
print(11) # int
print (-11) # int
print (0) #int

#separador
print('\n')
#

# float > Numero com ponto flutuante (casa decimal)
# O tipo float representa qualquer numero positivo ou negativo,
# sem sinal é considerado positivo, usar ponto (.) para separar casa decimal
print('tipos float = ponto flutuante (numeros com casa decimal)')
print(1.1, 10.11)
print(0.0, -1.5)

#separador
print('\n')
#

# A função type mostra o tipo que o Python inferiu ao valor.
print('a função type mostra o tipo que o Python inferiu ao valor')
print(type("Henrique"))
print(type(0))
print(type(1.1), type(-1.1), type(0.0))

#separador
print('\n')
#

# CTRL+; comenta a linha selecionada

# Tipo de dado bool (boolean)
# usada ao questionar algo em um programa
# Só existem 2 respostas sim (True) ou não (False)
# Existem varios operadores para "questionar"
# Dentre eles o ==, que é um operador lógico que questiona se um valor é igual a outro
print('tipo booleano, True ou False')
print(10 == 10) # Sim > True 
print(10 == 11) #Não > False 

#separador
print('\n')
#

# conversão de tipo, coerção, type convertion, typecasting, coercion
# é o ato de converter um tipo em outro
# tipos imutáveis e primitivos:
# str, int, float, bool
print('conversões de tipos')
print(int('1'), type(int('1')))
print(type(float('1')+1))
print(bool(''), (str('ou')), bool(' '))

#separador
print('\n')
#

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

#separador
print('\n')
#

#ordem de execução aritmética
#1. (n + n) os parenteses sempre são executados de dentro para fora ex: (4(3(2(1))))
#2. **
#3. * / // % em operações de mesmo "nivel" é executado da esquerda para a direita
#4. + -
conta_1 = 1 + 1 ** 5 + 5 = 7
conta_1_ordenada = (1 + 1) ** (5 + 5) = 1024

#solicitar dados do usuario, o programa para na linha até o usuario interagir, 
# saida sempre é no tipo string
# input('Qual o seu nome? ')
# numero_1 = input ('Digite um numero: ')
# int_numero_1 = int(numero_1)
# int_numero_2 = int(numero_2)
# print(f'A soma dos números é: {numero_1 + numero_2})'

