nome = input('Digite o seu primeiro nome: ')
letras_nome = len(nome)

try:
    if letras_nome <= 4:
        print('Seu nome é curto.')
    elif letras_nome <= 6:
        print('Seu nome é normal.')
    elif letras_nome > 6:
        print('Seu nome é muito grande.')
except:
    print('O valor digitado é invalido.')


