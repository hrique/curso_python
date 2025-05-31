convidados = ['einstein', 'tesla', 'fabio akita']
lista_convidados = f'A lista de convidados tem as seguintes pessoas: {convidados}'
print(lista_convidados)

print(f'\nOlá {convidados[1].title()} vamos sair para jantar?')

print(f'\nOlá {convidados[2].title()} vamos sair para jantar?')

print(f'\nOlá {convidados[0].title()} vamos sair para jantar?')

print(f'\nInfelizmente o {convidados[1].title()}, não poderá comparecer ao Jantar.')

convidados[1] = 'faker'

print(f'\nOlá {convidados[1].title()} gostaria de sair para jantar?')
