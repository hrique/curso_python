numero = input('Digite um numero inteiro: ')

try:
    numero_int = int(numero)
    par_ou_impar = numero_int % 2
    if par_ou_impar == 0:
        print(f'O número digitado {numero_int} é par.')
    elif par_ou_impar != 0:
        print(f'O número digitado {numero_int} é impar.')
except:
    print(f'O valor digitado {numero} não é um número!')
