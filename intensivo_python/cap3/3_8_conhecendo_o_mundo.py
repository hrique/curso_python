lugares_mundo = ['japão', 'china', 'europa', 'canada', 'estados unidos', 'dubai']

#exibindo os dados da lista
print(f'Essa é a lista de lugares que eu gostaria de conhecer: {lugares_mundo}.\n')

#exibindo os dados da lista em ordem alfabetica
print(f'Essa é a lista de lugares em ordem alfabetica: {sorted(lugares_mundo)}.\n')

#exibindo os dados da lista originais
print(f'Lista original: {lugares_mundo}.\n')

#invertendo a ordem dos dados
lugares_mundo.reverse()

print(f'Essa é a lista de lugares em ordem reversa: {lugares_mundo}.\n')

#invertendo novamente os dados para que voltem ao estado original
lugares_mundo.reverse()

print(f'Lista original: {lugares_mundo}.\n')

#ordenando os dados em ordem alfabetica
lugares_mundo.sort()

print(f'Lista armazenada em ordem alfabética: {lugares_mundo}.\n')

#ordenando os dados em ordem alfabetica reversa
lugares_mundo.sort(reverse=True)

print(f'Lista armazenada em ordem alfabética reversa: {lugares_mundo}.\n')