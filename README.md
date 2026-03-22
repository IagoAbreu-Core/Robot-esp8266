# Sistema Robótico Móvel com Comunicação UDP e RTOS

Desenvolvimento de um robô móvel integrado com braço robótico com 4 servos sg90 e motores dc para se mover. O projeto foca em sistemas embarcados de baixa latência e controle remoto via wifi com peças de baixo custo.

![Robot Imagen](./media/imagen_2.jpeg)

## Ferramentas usadas
Um computador com sistema Debian com sistema virtualização com pyenv para rodar python3.12 sem dar problema com sistema e com python3.12 e podere usar [ESP8266_RTOS_SDK](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/index.html) que foi usado para programar esp8266

### Destaques Técnicos:

1. Firmware: Desenvolvido em C utilizando o SDK oficial da Espressif e FreeRTOS para gerenciamento de tarefas (servidor UDP e controle PWM).
<br>
O firmware foi desenvolvido utilizando como base os exemplos oficiais do SDK, com customizações para integração do sistema Wi-Fi e do servidor UDP.

2. Protocolo de Comunicação: Implementação de socket UDP para controle
em tempo real, permitindo execução de comandos manuais.

3. Software de Controle: Interface via terminal desenvolvida em Lua para envio de pacotes de controle.
<br>
O sistema de controle é simples. pode usar comandos como "stop" e "turnl" que enviará comando pré-definido ou manualmente, um exemplo de mover as rodas para uma direção e "M 10000 0 0 10000" ou um script que vai ler comando por comando com delay definido.

4. Hardware: Integração de ESP8266 em uma placa desenvolvimento wemos d1, ponte H, reguladores de tensão e baterias Li-ion em arquitetura customizada em placa perfurada (5cm x 7cm) com suporte fabricado via impressão 3D (PLA).

### Amostra do projeto
![Robot GIF](./media/robo.gif)

![Robot Imagen](./media/imagen_1.jpeg)