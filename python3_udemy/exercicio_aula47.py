nome = input('Qual é o seu nome? ')
idade = input('Qual é a sua idade? ')

idade_digitada = int(idade)
nome_invertido = nome[::-1]
letras_nome = len(nome)
nome_primeira_letra = nome[0] 
nome_ultima_letra = nome[-1]


if nome != '' and idade_digitada != '':
    print(f'Seu nome é {nome}')
    print(f'Você tem {idade_digitada} anos.')
    print(f'Seu nome invertido é {nome_invertido}')
    print(f'Seu nome tem {letras_nome} letras.')
    print(f'A primeira letra do seu nome é: {nome_primeira_letra}.')
    print(f'A ultima letra do seu nome é: {nome_ultima_letra}.')
    if ' ' in nome:
            print('Seu nome tem espaços.')
    else:
            print('Seu nome não tem espaços.')
else:
    print(f'Informações incompletas ou inválidas.')


