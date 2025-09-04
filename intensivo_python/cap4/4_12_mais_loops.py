minhas_comidas = ['pizza', 'falafel', 'carrot cake']
comidas_amigo = minhas_comidas [:]

minhas_comidas.append('cannoli')
comidas_amigo.append('sorvete')

print('Minhas comidas favoritas são: ')
for i in minhas_comidas:
    print(i)

print('\nAs comidas favoritas do meu amigo são: ')
for i in comidas_amigo:
    print(i)