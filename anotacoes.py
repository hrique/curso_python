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

#asdkjaosidji