convidados = ['einstein', 'tesla', 'fabio akita']
lista_convidados = f'A lista de convidados tem as seguintes pessoas: {convidados}'
print(lista_convidados)

msg_conv_1 = f'Olá {convidados[1].title()} vamos sair para jantar?'
print(msg_conv_1)

msg_conv_2 = f'Olá {convidados[2].title()} vamos sair para jantar?'
print(msg_conv_2)

msg_conv_3 = f'Olá {convidados[0].title()} vamos sair para jantar?'
print(msg_conv_3)

print(f'Infelizmente o {convidados[1].title()}, não poderá comparecer ao Jantar.')

convidados[1] = 'faker'

msg_conv_4 = f'Olá {convidados[1].title()} gostaria de sair para jantar?'
print(msg_conv_4)