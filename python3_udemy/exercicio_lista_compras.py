# importa a biblioteca os que utilizei para limpar o terminal
import os

# declara a variavel lista para armazenar os valores
lista = []

while True:
    # pede uma ação ao usuario
    opcao = input('Selecione uma opção\n' \
    '[i]nserir [a]pagar [l]istar:')

    # declara quais opções podem ser digitadas
    opcoes_validas = 'ial'

    # verifica se a opção esta nas opções validas
    if opcao not in opcoes_validas:
        os.system('cls')
        print('Digite uma opção válida!')
    
    # verifica se foi digitada apenas 1 opção
    if len(opcao) >1:
        os.system('cls')
        print('Digite apenas uma opção!')
        

    # se a opção em minusculo é a letra 'i' executa a ação de adicionar um item a lista e limpa o terminal
    if opcao.lower() == 'i':
        os.system('cls')
        novo_item = input('Digite o item para inserir na lista:')
        lista.append(novo_item)

    # se a opção em minusculo é a letra 'a' tenta apagar o indice da lista que o usuario digitar
    elif opcao.lower() == 'a':
        os.system('cls')
        for indice, item in enumerate(lista):
            print(indice, item)
        apagar_item = input('Digite o indice que gostaria de apagar da lista:')
        try:
            del lista[int(apagar_item)]
        except:
            print('Digite um indice valido!')

    # se a opção em minusculo é a letra 'l' mostra os valores que estao na lista
    elif opcao.lower() == 'l':
        os.system('cls')
        for indice, item in enumerate(lista):
            print(indice, item)

    