nome = input('Digite o seu nome e sobrenome: ')
mensagem = f'Olá {nome.lower().lstrip().rstrip().title()} gostaria de aprender um pouco de Python hoje?'
nome_maius = f'Seu nome todo maiusculo fica: {nome.upper()}'
nome_minus = f'Seu nome todo minusculo fica: {nome.lower()}'
nome_titulo = f'Seu nome com as iniciais maiusculas fica: {nome.title()}'
print(mensagem)
print(nome_maius)
print(nome_minus)
print(nome_titulo)
