#include <stdio.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"


#include "esp_log.h"
#include "esp_websocket_client.h"
#include "esp_event.h"

#include "driver/gpio.h"
#include "driver/mcpwm.h"

#include "mfrc522.h"
#include "spi.h"

#define PIN_NUM_MISO 12
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   15
#define PIN_NUM_RST  GPIO_NUM_4

#define PWM_HZ 1000
#define PIN_M1 19
#define PIN_M2 18
#define PIN_EN1 23
#define PIN_EN2 22

#define ESP_INTR_FLAG_DEFAULT 0 

#define MINSPEED 5
#define MAXSPEED 99
#define KSPEED 0.01
#define PRECISION 7

static const char *TAG = "WEBSOCKET";
static const char *WEBSOCKET_ECHO_ENDPOINT = "ws://academia.go.ro:3024";


esp_websocket_client_handle_t wsClient;
volatile int status = 0;

int gspeed = 0;
volatile int counter = 0;
volatile uint32_t oldB1 = 0;

int q[100], qnext = 0, qlast = 0, qn = 0, qsize=99;
int qp[100], qpnext = 0, qplast = 0, qpn = 0, qpsize=99;

int spotDist = 16000;
int spotOffset = 0;
int currentSpot = 0;
int parkingSpots = 8;

const int parkID = 8192;

void sock_log_handeling(int32_t event_id, esp_websocket_event_data_t *data, int level);

inline int max(int a, int b){return a > b ? a : b;}
inline int min(int a, int b){return a < b ? a : b;}
void updateSpeed(int speed)
{
    gspeed = speed;
    int norm = speed == 0 ? 0 : max(MINSPEED, max(MAXSPEED, abs(speed)));
    // printf("runnin' speed: %d", norm);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, speed > 0 ? MCPWM_OPR_A : MCPWM_OPR_B, 0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, speed > 0 ? MCPWM_OPR_B : MCPWM_OPR_A, norm);
}

void rfid_task(void *pvParameter)
{
    printf("\tinitialising RFID... ");
    spi_init(PIN_NUM_CLK, PIN_NUM_MOSI, PIN_NUM_MISO);  // Init Driver SPI
    MFRC522_Init(PIN_NUM_RST, PIN_NUM_CS); // Init MFRC522
    printf("done\n");
    uint8_t CardID[30];
    while (true)
    {
        if (MFRC522_Check(CardID) == MI_OK)
        {
            char s[100];
            int len = sprintf(s, "{\"tag\": \"newCard\", \"id\": \"[%02x-%02x-%02x-%02x-%02x]\", \"currentSpot\": %d}", CardID[0], CardID[1], CardID[2], CardID[3], CardID[4], currentSpot);

            ESP_LOGI("MFRC", "%s \r\n", s);
            if (esp_websocket_client_is_connected(wsClient))
            {
                ESP_LOGI(TAG, "Sending %s", s);
                esp_websocket_client_send(wsClient, s, len, portMAX_DELAY);
            }
            
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void mc_gpio_init()
{
    printf("initializing mcpwm gpio...\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PIN_M1);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, PIN_M2);
    printf("Configuring Initial Parameters of mcpwm...\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = PWM_HZ;    //frequency = 500Hz,
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
}


static void gpio_isr_handler(void* arg)
{
    // uint32_t gpio_num = (uint32_t) arg;
    uint32_t newB0 = gpio_get_level(PIN_EN1);
    uint32_t newB1 = gpio_get_level(PIN_EN2);
    counter += ((newB0 == oldB1) ? 1 : -1);
    oldB1 = newB1;
}

void encoder_init()
{
    printf("initializing encoder... ");
    counter = 0;
    gpio_set_direction(PIN_EN1, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_EN2, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_EN1, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(PIN_EN2, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(PIN_EN1, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(PIN_EN2, GPIO_INTR_ANYEDGE);

    printf("starting interrupts... ");
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(PIN_EN1, gpio_isr_handler, (void*) PIN_EN1);
    gpio_isr_handler_add(PIN_EN2, gpio_isr_handler, (void*) PIN_EN2);

    printf("done\n");
}

void moveToPos(int pos)
{
    printf("moving to pos: %d\n", pos);

    int err = pos - counter;
    int lastdir = 0;
    while(abs(err) > PRECISION)
    {
        updateSpeed((int)err*KSPEED);
        lastdir = err/abs(err);
        vTaskDelay(10/portTICK_PERIOD_MS);
        err = pos - counter;
    }
    updateSpeed(-lastdir*60);
    vTaskDelay(30/portTICK_PERIOD_MS);
    updateSpeed(0);

    printf("arrived to: %d", counter);
}

void moveToSpot(int spot)
{
    //if(spot == currentSpot) return;
    spot %= parkingSpots;
    moveToPos(spotOffset + spot*spotDist);
    currentSpot = spot;
}


void motorTask()
{
    int next;
    while(true)
    {
        if(qpn > 0)
        {
            next = qp[qnext++];
            qpnext %= qpsize;
            qpn--;
            moveToPos(next);
        }
        if(qn > 0)
        {
            next = q[qnext++];
            qnext %= qsize;
            qn--;
            moveToSpot(next);
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    // esp_websocket_client_handle_t client = (esp_websocket_client_handle_t)handler_args;
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    sock_log_handeling(event_id, data, 2);
    if(event_id != WEBSOCKET_EVENT_DATA || data->op_code != 1) return;
    char *s = (char*)data->data_ptr;
    int n = data->data_len;
    s[n] = '\0';
    if(s[0] != '_') return;

    if(strncmp(s, "_spot", 5) == 0)
    {
        int newSpot;
        sscanf(s+6, "%d", &newSpot);
        q[qlast++] = newSpot;
        qlast %= qsize;
        qn++;
    }
    else if(strncmp(s, "_speed", 6) == 0)
    {
        int speed;
        printf("new speed");
        sscanf(s+7, "%d", &speed);
        printf(": %d\n", speed);
        updateSpeed(speed);
    }
    else if(strncmp(s, "_pos", 4) == 0)
    {
        int pos;
        sscanf(s+5, "%d", &pos);
        qp[qlast++] = pos;
        qplast %= qsize;
        qpn++;
    }
    else if(strncmp(s, "_set_dist", 9) == 0)
    {
        int dist;
        sscanf(s+10, "%d", &dist);
        spotDist = dist;
    }
    else if(strncmp(s, "_set_offset", 11) == 0)
    {
        int off;
        sscanf(s+12, "%d", &off);
        spotOffset = off;
    }
    else if(strncmp(s, "_set_current", 12) == 0)
    {
        int crt;
        sscanf(s+13, "%d", &crt);
        currentSpot = crt;
    }
    else if(strncmp(s, "_get_status", 11) == 0)
    {
        char res[100];
        int len = sprintf(res, "{\"tag\": \"status\", \"status\": %d, \"crtSpot\": %d, \"pos\": %d}", status, currentSpot, counter);
        esp_websocket_client_send(wsClient, res, len, portMAX_DELAY);
    }
    else if(strncmp(s, "_get_info", 9) == 0)
    {
        char res[100];
        int len = sprintf(res, "{\"tag\": \"info\", \"parkID\": %d, \"parkingSpots\": %d, \"spotDist\": %d, \"spotOffset\": %d}", parkID, parkingSpots, spotDist, spotOffset);
        esp_websocket_client_send(wsClient, res, len, portMAX_DELAY);
    }
    else if(strncmp(s, "_reset", 6) == 0)
    {
        counter = 0;
    }
}

static esp_websocket_client_handle_t websocket_app_start(void)
{
    ESP_LOGI(TAG, "Connectiong to %s...", WEBSOCKET_ECHO_ENDPOINT);
    const esp_websocket_client_config_t websocket_cfg = {
        .uri = WEBSOCKET_ECHO_ENDPOINT, 
    };
    esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    esp_websocket_client_start(client);
    return client;
}

void logCounter()
{
    int lastCounter = counter;
    while(true)
    {
        if(counter != lastCounter)
            printf("counter: %d\n", counter);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    ESP_LOGI(TAG, "[APP] Startup..");
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("WEBSOCKET_CLIENT", ESP_LOG_DEBUG);
    esp_log_level_set("TRANS_TCP", ESP_LOG_DEBUG);

    ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    wsClient = websocket_app_start();
    mc_gpio_init();
    encoder_init();

    printf("creating rfid task...\n");
	xTaskCreate(&rfid_task, "rfid_task", 4096, NULL, 4, NULL);
    printf("done\n");

    printf("creating motor task...\n");
	xTaskCreate(&motorTask, "motorTask", 4096, NULL, 4, NULL);
    printf("done\n");

    xTaskCreate(&logCounter, "logCounter", 2048, NULL, 4, NULL);

}





void sock_log_handeling(int32_t event_id, esp_websocket_event_data_t *data, int level)
{
    if(level <= 0) return;
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
            break;
        case WEBSOCKET_EVENT_DATA:
            if(data->op_code == 10 && level < 3)
                break;
            if(level >= 2)
            {
                ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
                ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
                ESP_LOGW(TAG, "Received=%.*s\r\n", data->data_len, (char*)data->data_ptr);
            }
            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
            break;
    }
}