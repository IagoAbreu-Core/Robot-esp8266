#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"
#include "driver/pwm.h"

//PWM perido
#define PWM_PERIOD 20000

//configuracao da conexao da rede
#define SSID	"NOME_DA_REDE"
#define PASSWORD "SENHA"
#define PORT 1234

static const char *TAG = "ROBOT";

// pwm pin
const uint32_t pin_num[8] = {
	14,15,2,0,	// motores DC
	12,13,5,4	// servos motores
};

uint32_t duties[8] = {
	0,0, // motor 1
	0,0, // motor 2
	// servos
	450, // garra
	900, // y
	900, // z
	1425 // x
};

float phase[8] = {
	0,0,0,0,	//	motores DC
	90,90,90,90	// servos motores
};

void pwm_control(void) {
	ESP_LOGI(TAG, "Robô iniciado!\n");

	pwm_init(PWM_PERIOD, duties, 8, pin_num);
	pwm_set_phases(phase);
	pwm_start();

}

static void event_handler(void* arg,esp_event_base_t event_base,int32_t event_id, void* event_data) {
	if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
		esp_wifi_connect();
	else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED){
	esp_wifi_connect();
	ESP_LOGI(TAG, "Tentado reconectar..");
	}else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t *event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "Conectado! IP: " IPSTR, IP2STR(&event->ip_info.ip));
	}
}

void wifi_init_sta(void) {
	ESP_LOGI(TAG, "Iniciado Wi-fi...");

	tcpip_adapter_init();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg);

	esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
	esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL);

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = SSID,
			.password = PASSWORD
		}
	};

	esp_wifi_set_mode(WIFI_MODE_STA);
	esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
	esp_wifi_start();

}

static void udp_server_task(void *pvParameters) {
	char rx_buffer[128];
	char addr_str[128];
	int addr_family;
	int ip_protocol;

	while(1) {
		struct sockaddr_in destAddr = {
			.sin_addr.s_addr = htonl(INADDR_ANY),
			.sin_family = AF_INET,
			.sin_port = htons(PORT)
		};
		addr_family = AF_INET;
		ip_protocol = IPPROTO_IP;
		inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) -1);

		int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
		if (sock < 0) {
			ESP_LOGE(TAG, "tenta tiva de criar socket: errno %d",errno);
			break;
		}
		ESP_LOGI(TAG, "Socket criado!");

		int err = bind(sock, (struct sockaddr*)&destAddr, sizeof(destAddr));
		if (err < 0) {	
			ESP_LOGE(TAG, "Erro ao conectar via socket: errno %d", errno);
			break;
		}
		ESP_LOGI(TAG, "Socket conectado!");

		while (1) {
			ESP_LOGI(TAG, "Waiting for data");

			struct sockaddr_in sourceAddr;
			socklen_t socklen = sizeof(sourceAddr);
			int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) -1 , 0, (struct sockaddr*)&sourceAddr, &socklen);

			if(len < 0) {
				ESP_LOGE(TAG, "Erro ao receber: errno %d",errno);
				break;
			}else {
				inet_ntoa_r(((struct sockaddr_in*)&sourceAddr)->sin_addr.s_addr,addr_str, sizeof(addr_str) -1);

				rx_buffer[len] = 0;
				ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
				ESP_LOGI(TAG, "%s", rx_buffer);
				int x,y,z,f;
				char cm;
				sscanf(rx_buffer, "%c %d %d %d %d", &cm, &x,&y,&z,&f);
				if(cm == 'M'){
					duties[0] = x;
					duties[1] = y;
					duties[2] = z;
					duties[3] = f;
					for(int i=0;i < 4;i++)
						pwm_set_duty(i,duties[i]);
        				pwm_start();
				}else if(cm == 'B'){
					if(f < 450)
						f=450;
					else if(f > 600)
						f=600;
					duties[4] = f;
					if(y < 900)
						y=900;
					duties[5] = y;
					if(z < 900)
						z=900;
					duties[6] = z;

					duties[7] = x;
					
					for(int i=4;i <8;i++)
						pwm_set_duty(i,duties[i]);
					pwm_start();
				}
				
				int err = sendto(sock, rx_buffer, len, 0, (struct sockaddr*)&sourceAddr, sizeof(sourceAddr));
				if (err < 0)
					ESP_LOGE(TAG, "Erro acoreu durante o envior: errno %d",errno);
			}
		}
		if(sock != -1) {
			ESP_LOGE(TAG, "desligar socket e resetar...");
			shutdown(sock, 0);
			close(sock);
		}
		vTaskDelay(150/ portTICK_RATE_MS);
	}
	vTaskDelete(NULL);

}

void app_main(void) {

	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	wifi_init_sta();
	pwm_control();
	xTaskCreate(udp_server_task, "udp_server", 2048, NULL, 10, NULL);

}

