nome = "Henrique"
tamanho_nome = len(nome)
contador_nome = 0
novo_nome = '$'

while contador_nome < tamanho_nome:
    novo_nome += nome[contador_nome] + '$'
    contador_nome += 1

print(novo_nome)