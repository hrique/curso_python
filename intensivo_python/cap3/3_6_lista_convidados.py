# cria a lista com os 3 primeiros valores
convidados = ['einstein', 'tesla', 'fabio akita']

# print da lista no momento
print(f'A lista de convidados tem as seguintes pessoas: {convidados}')

# prints dos convites para cada um da lista
print(f'\nOlá {convidados[1].title()} vamos sair para jantar?')
print(f'\nOlá {convidados[2].title()} vamos sair para jantar?')
print(f'\nOlá {convidados[0].title()} vamos sair para jantar?')

# print para o convidado que não podera comparecer
print(f'\nInfelizmente o {convidados[1].title()}, não poderá comparecer ao Jantar.')

# altera o convidado que não podera comparecer para um novo
convidados[1] = 'faker'

# convite para a nova pessoa
print(f'\nOlá {convidados[1].title()} gostaria de sair para jantar?')


print(f'\nOlá pessoal, consegui uma mesa maior e posso chamar mais 3 pessoas!')

convidados.insert(1,'fabio brazza')
convidados.insert(1,'tom morello')
convidados.append('hideo kojima')

print(f'\nconvidados')

print(f'\nOlá {convidados[1].title()} gostaria de sair para jantar?')
print(f'\nOlá {convidados[2].title()} gostaria de sair para jantar?')
print(f'\nOlá {convidados[-1].title()} gostaria de sair para jantar?')

