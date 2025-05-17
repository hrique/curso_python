nome = input('Qual é o seu nome completo? ')

ano_nascimento = input('Em que ano você nasceu? ')
int_ano_nascimento = int(ano_nascimento)

ano_atual = input('Em que ano estamos? ')
int_ano_atual = int(ano_atual)

altura_metros = input('Qual a sua altura? ')
float_altura_metros = float(altura_metros)

peso = input('Qual o seu peso? ')
int_peso = int(peso)

imc= (int_peso / float_altura_metros ** 2)

print(f'Seu nome é {nome}, você tem {int_ano_atual - int_ano_nascimento} anos de idade')
print(f'Você tem {float_altura_metros} metros de altura, e pesa {int_peso} Kg')
print(f'Seu IMC é :{imc:.2f}')