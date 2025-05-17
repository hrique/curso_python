while True:
    num_1 = input('Digite um numero: ')
    num_2 = input('Digite outro numero: ')
    operador = input('Digite o operador (+, -, *, /): ')

    numeros_validos = None
    float_num_1 = 0
    float_num_2 = 0

    try:
        float_num_1 = float(num_1)
        float_num_2 = float(num_2)
        numeros_validos = True
    except:
        numero_validos = None
        
    if numeros_validos is None:
        print('Você não digitou numeros.')
        continue

    operadores_permitidos = '+-/*'

    if operador not in operadores_permitidos:
        print('Você não digitou um operador válido.')
        continue

    if len(operador) > 1:
        print('Digite apenas um operador.')
        continue

    print('Calculando... Confira o resultado:')

    if operador == '+':
        print(f'{num_1}+{num_2}=', float_num_1 + float_num_2)
    elif operador == '-':
        print(f'{num_1}-{num_2}=', float_num_1 - float_num_2)
    elif operador == '/':
        print(f'{num_1}/{num_2}=', float_num_1 / float_num_2)
    elif operador == '*':
        print(f'{num_1}*{num_2}=', float_num_1 * float_num_2)
    else:
        print('Algo deu errado.')



    sair = input('Quer sair? [s]im: ').lower().startswith('s')

    if sair is True:
        break