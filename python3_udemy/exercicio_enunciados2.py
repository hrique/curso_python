hora = input('Digite que horas são: (digite um numero entre 0 e 23)')

try:
    hora_int = int(hora)
    # bom_dia = (hora_int <= 11)
    # boa_tarde = (hora_int > bom_dia) and (hora_int <= 17)
    # boa_noite = (hora_int > boa_tarde) and (hora_int <= 23)
    if hora_int >= 0 and hora_int <= 11:
        print('Bom dia!')
    elif hora_int <= 17:
        print('Boa tarde!')
    elif hora_int <= 23:
        print('Boa noite!')
    else:
        print('Não conheço essa hora.')
except:
    print(f'O valor digitado {hora} não é um valor valido!')