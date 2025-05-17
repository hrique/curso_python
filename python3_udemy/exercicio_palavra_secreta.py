import os

palavra_secreta = 'thais'
tentativas = 0
letra_acertada = ''

while True:
    
    letra = input(f'Digite uma letra: ')
    tentativas += 1

    if len(letra) > 1:
        print('Digite apenas uma letra!')
        continue
    elif letra.isalpha() != True:
        print('Digite apenas letras!')
        continue              
        
    if letra in palavra_secreta:
        letra_acertada += letra

    palavra_formada = ''
    for letra_chute in palavra_secreta:
        if letra_chute in letra_acertada:
            palavra_formada += letra_chute
        else:
            palavra_formada += '*'
    print(f'Palavra formada:{palavra_formada}')

    if palavra_formada == palavra_secreta:
        os.system('cls')
        print('PARABÉNS!!!!') 
        print(f'A palavra secreta era: {palavra_formada}')
        print(f'Quantidade de tentativas: {tentativas}')
        break
