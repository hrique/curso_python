velocidade_carro = 70
local_carro = 115

LIMITE_RADAR_1 = 75
LOCAL_RADAR_1 = 115
RANGE_RADAR_1 = 5

carro_chegou_radar_1 = (local_carro) >= (LOCAL_RADAR_1 - RANGE_RADAR_1)
carro_passou_radar_1 = (local_carro) <= (LOCAL_RADAR_1 + RANGE_RADAR_1)
carro_multado_radar_1 = (velocidade_carro > LIMITE_RADAR_1) and (carro_chegou_radar_1)
carro_dentro_do_limite = (velocidade_carro == LIMITE_RADAR_1) and (carro_chegou_radar_1)
carro_fora_range = (local_carro) < (LOCAL_RADAR_1 - RANGE_RADAR_1)
# carro_nao_multado_radar_1 = (velocidade_carro < LIMITE_RADAR_1) or (carro_fora_range)



if carro_chegou_radar_1:
    print(f'O carro chegou no radar 1 na velocidade de: {velocidade_carro} Km/h.')

if carro_passou_radar_1:
    print(f'O carro passou pelo radar 1 na velocidade de: {velocidade_carro} Km/h.')

if carro_multado_radar_1:
    print('O carro foi multado no radar 1.')

# if carro_nao_multado_radar_1:
    # print('O carro não foi multado no radar 1.')

if carro_dentro_do_limite:
    print(f'O carro não foi multado, mas estava no limite de {LIMITE_RADAR_1} Km/h.')

if carro_fora_range:
    print(f'O carro não chegou no radar ainda.')