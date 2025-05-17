lista = ['Henrique', 'Thais', 'Lilian', 'Mauricio', '1', '2', '3']
tamanho_lista = len(lista)
index_lista = 0
lista.append('Rossi')
del lista[4]

for item in lista:
    print(f'{index_lista} - {item}')    
    index_lista += 1

print('Fim da lista')
