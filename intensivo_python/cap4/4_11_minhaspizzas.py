lista_pizzas = ['portuguesa', 'frango com catupiry', 'escarola', 'quatro queijos']

for pizza in lista_pizzas:
    print(f'Eu gosto da pizza de {pizza}.')

print('\nEu amo pizza!')

amigo_pizzas = lista_pizzas[:]

lista_pizzas.append('baianinha')

amigo_pizzas.append('pizza vegana')


print('\nMinhas pizzas favoritas são: ')
for pizza in lista_pizzas:
    print(pizza)

print('\nAs pizzas favoritas do meu amigo são: ')
for pizza in amigo_pizzas:
    print(pizza)