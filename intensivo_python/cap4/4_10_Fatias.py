numeros = list(range(1,1000001))

numeros_meio = len(numeros) // 2
meio_mais1 = numeros_meio + 1
meio_menos2 = numeros_meio -2

print(f'Os três primeiros números da lista são: {numeros[0:3]}')

print(f'Os três números do meio da lista são: {numeros[meio_menos2:meio_mais1]}')

print(f'Os três últimos números da lista são: {numeros[-3:]}')