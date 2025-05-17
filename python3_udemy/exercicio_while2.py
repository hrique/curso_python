frase = 'O Python é uma linguagem de ' \
    'programação multiparadigma. ' \
    'Python foi criado por Guido ' \
    'van Rossum.'

i = 0
qtd_apareceu = 0
letra_apareceu_mais_vezes = ''

while i < len(frase):
    letra_atual = frase[i]

    if letra_atual == ' ':
        i += 1
        continue

    contagem_letra_atual = frase.count(letra_atual)

    if qtd_apareceu < contagem_letra_atual:
        qtd_apareceu = contagem_letra_atual
        letra_apareceu_mais_vezes = letra_atual

    i += 1

print(
    'A letra que apareceu mais vezes foi '
    f'"{letra_apareceu_mais_vezes}" que apareceu '
    f'{qtd_apareceu}x'
)