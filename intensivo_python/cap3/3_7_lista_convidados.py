convidados = ['einstein', 'tesla', 'fabio akita']
print(f'A lista de convidados tem as seguintes pessoas: {convidados}')

#apresenta um convite de jantar para cada convidado da lista
for convidado in convidados:
    print(f'\nOlá {convidado.title()}, vamos sair para jantar?')

#informa que um convidado não vai comparecer e adiciona outro em seu lugar
convidado_ocupado = 'tesla'
print(f'\nInfelizmente o {convidado_ocupado.title()}, não poderá comparecer ao Jantar.')
convidados.remove(convidado_ocupado)
convidado_novo = 'faker'
convidados.append(convidado_novo)

#apresenta um convite de jantar para o novo convidado
print(f'\nOlá {convidado_novo.title()} gostaria de sair para jantar?')

print(f'\nOlá pessoal, consegui uma mesa maior e posso chamar mais 3 pessoas!')

#adicionando novos convidados
convidados.insert(1,'fabio brazza')
convidados.insert(2,'tom morello')
convidados.append('hideo kojima')

print(f'\nEssa é a nova lista de convidados: {convidados}, a lista esta com um total de {len(convidados)} pessoas.')

#confirma a presença de cada convidado da lista
for convidado in convidados:
    print(f'\nOlá {convidado.title()}, gostaria de confirmar sua presença no jantar.')

#remove os convidados até ter apenas 2 na lista e manda uma mensagem de desculpas para os removidos
while len(convidados) > 2:
    conv_removido = convidados.pop()
    print(f'\nOlá {conv_removido.title()}, infelizmente a mesa não está mais disponível, então vamos ter que marcar para outro dia.')

print(f'\nEssa é a nova lista de convidados: {convidados}')

#informa os convidados restantes que eles ainda estao convidados
for convidado in convidados:
    print(f'\nOlá {convidado.title()}, você ainda está convidado para o jantar!')

print(f'\nEssa é a nova lista de convidados: {convidados}')

#limpando a lista de convidados
del convidados[-1]

print(f'\nEssa é a nova lista de convidados: {convidados}')

del convidados[-1]

print(f'\nEssa é a nova lista de convidados: {convidados}')