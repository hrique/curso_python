convidados = ['einstein', 'tesla', 'fabio akita']
print(f'A lista de convidados tem as seguintes pessoas: {convidados}')


print(f'\nOlá {convidados[1].title()} vamos sair para jantar?')

print(f'\nOlá {convidados[2].title()} vamos sair para jantar?')

print(f'\nOlá {convidados[0].title()} vamos sair para jantar?')

print(f'\nInfelizmente o {convidados[1].title()}, não poderá comparecer ao Jantar.')

convidados[1] = 'faker'

print(f'\nOlá {convidados[1].title()} gostaria de sair para jantar?')

print(f'\nOlá pessoal, consegui uma mesa maior e posso chamar mais 3 pessoas!')

convidados.insert(1,'fabio brazza')
convidados.insert(1,'tom morello')
convidados.append('hideo kojima')

print(convidados)

print(f'\nOlá {convidados[1].title()} gostaria de sair para jantar?')
print(f'\nOlá {convidados[2].title()} gostaria de sair para jantar?')
print(f'\nOlá {convidados[-1].title()} gostaria de sair para jantar?')
