convidados = ['einstein', 'tesla', 'fabio akita']
lista_convidados = f'A lista de convidados tem as seguintes pessoas: {convidados}'
print(lista_convidados)

convidado_1 = convidados.pop()
msg_conv_1 = f'Olá {convidado_1.title()} vamos sair para jantar?'
print(msg_conv_1)

convidado_2 = convidados.pop(1)
msg_conv_2 = f'Olá {convidado_2.title()} vamos sair para jantar?'
print(msg_conv_2)

convidado_3 = 'einstein'
convidados.remove('einstein')
msg_conv_3 = f'Olá {convidado_3.title()} vamos sair para jantar?'
print(msg_conv_3)

print(convidados)
