#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include "../Lib/AT_function.h"
#include "../Lib/SIM_UART.h"
#include "../Lib/common.h"
#include "../Lib/SDCard.h"
#include "../Lib/mypH.h"
#include "../Lib/myDO.h"
#include "../Lib/myTDS.h"
#include "../Lib/i2cdev.h"
#include "../Lib/ds1307.h"
#include "esp_adc_cal.h"
#include "../Lib/ds3231.h"
#include "../Lib/EC.h"
#include "../Lib/ssd1306.h"
#include "../Lib/font.h"
#include "freertos/semphr.h"

SemaphoreHandle_t I2C_mutex;

#define PERIOD 60000
#define RESISTOR 100000.0
#define DO_ON_PIN1 GPIO_NUM_18
#define EC_ON_PIN1 GPIO_NUM_19
#define PH_ON_PIN GPIO_NUM_25
#define DO_ON_PIN GPIO_NUM_26
#define EC_ON_PIN GPIO_NUM_27

#define EC_AdC_CHANNEL ADC1_CHANNEL_4
#define pH_ADC_CHANNEL ADC1_CHANNEL_6
#define DO_ADC_CHANNEL ADC1_CHANNEL_7

volatile uint32_t count_restart = 0;

const char *TAG_UART = "UART";
TaskHandle_t uart_rx_task_handle;
TaskHandle_t check_timeout_task_handle;
TaskHandle_t send_data;
TaskHandle_t get_data;
TaskHandle_t test_ds3231;
TaskHandle_t p_sd_card;
TaskHandle_t p_task_control;
TaskHandle_t p_task_display;
nvs_handle_t nvsHandle;

i2c_dev_t dev;

volatile char buff[126];
volatile char buff_sd[126];
volatile char time_buff[50];
volatile data_type data;
volatile char name_file[50];
void send_data_task(void *arg)
{
	int count = 1;
	bool is_send_server = false;
	//	xTaskCreatePinnedToCore(Timeout_task, "timeout task", 4096*2, NULL, 3, &check_timeout_task_handle, tskNO_AFFINITY);
	while (1)
	{
		ESP_LOGI(TAG_UART, "Ready to send server");

		is_send_server = Send_Data_to_Server(buff);
		if (is_send_server == true || count > 2)
		{
			vTaskDelete(NULL);
		}
		printf("count send:%d\n", count);
		count++;
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}

void uart_rx_task(void *arg)
{
	uart_config_t uart_config = {
		.baud_rate = ECHO_UART_BAUD_RATE,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_REF_TICK,
		//.source_clk = UART_SCLK_APB,
	};
	int intr_alloc_flags = 0;
#if CONFIG_UART_ISR_IN_IRAM
	intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif
	uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags);
	uart_param_config(ECHO_UART_PORT_NUM, &uart_config);
	uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
	uint8_t data[BUF_SIZE];
	while (1)
	{
		int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
		if (len > 0)
		{
			data[len] = 0;
			ESP_LOGI(TAG_UART, "Rec: \r\n%s", data);
			if (SIMCOM_ATCommand.ExpectResponseFromATC[0] != 0 && strstr((const char *)data, SIMCOM_ATCommand.ExpectResponseFromATC))
			{
				SIMCOM_ATCommand.ExpectResponseFromATC[0] = 0;
				if (SIMCOM_ATCommand.SendATCallBack != NULL)
				{
					SIMCOM_ATCommand.TimeoutATC = 0;
					SIMCOM_ATCommand.SendATCallBack(EVENT_OK, data);
				}
			}
			if (strstr((const char *)data, "ERROR"))
			{

				if (SIMCOM_ATCommand.SendATCallBack != NULL)
				{
					SIMCOM_ATCommand.SendATCallBack(EVENT_ERROR, data);
				}
			}
			//			if(strstr((char *)data,"CLOSE")){
			//				Flag_Connected_Server = false;
			//			}
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}
float get_temp(adc1_channel_t ADC_CHAN)
{
	uint8_t i = 50; // lower power consumption
	esp_err_t err;

	int buf[50];
	uint32_t avgValue;
	float ntc_res;
	float voltage;

	for (i = 50; i > 0; i--) //
	{
		buf[i] = adc1_get_raw(ADC_CHAN);
		// pH->_pH_4_Voltage += (analogRead(pin) * Vref) / ADC_resolution;
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

	for (int i = 0; i < 50; i++) // sort the analog from small to large
	{
		for (int j = i + 1; j < 50; j++)
		{
			if (buf[i] > buf[j])
			{
				int temp = buf[i];
				buf[i] = buf[j];
				buf[j] = temp;
			}
		}
	}
	avgValue = 0;
	for (int i = 10; i < 40; i++)
	{
		avgValue += buf[i];
	}
	avgValue /= 30;
	voltage = (float)avgValue / 4096.0 * 3.3;
	ntc_res = (3.3 - voltage) / voltage * RESISTOR;
	float Temp = 1 / ((log(ntc_res / RESISTOR) / 3940.0) + 1 / 298.15) - 273;
	return Temp + 18;
}

void sd_task(void *arg)
{
	esp_err_t err_init = sd_card_intialize();
	//	sprintf(name_file, "log_3_7");
	if (name_file == NULL)
	{
		sprintf(name_file, "null.txt");
	}
	strcpy(buff_sd, buff);
	strcat(buff_sd, "\n");
	printf("buff_sd:%s", buff_sd);
	esp_err_t err = sd_write_file(name_file, buff_sd);
	if (err != ESP_OK)
	{
		ESP_LOGI(__func__, "Save data error");
	}
	else
	{
		ESP_LOGI(__func__, "Save data success");
	}
	if (err_init == ESP_OK)
	{
		sd_deinitialize();
	}
	while (1)
	{

		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}
void display_task(void *pvParameter)
{
	SSD1306_Init();
	while (1)
	{
		if (xSemaphoreTake(I2C_mutex, portMAX_DELAY) == pdTRUE)
		{
			char str[15];
			SSD1306_Clear();
			SSD1306_GotoXY(0, 0);
			sprintf(str, "Temp:%.2f", data.temp);
			SSD1306_Puts(str, &Font_7x10, 1);
			SSD1306_GotoXY(0, 10);
			sprintf(str, "pH :%.2f", data.pH);
			SSD1306_Puts(str, &Font_7x10, 1);
			SSD1306_GotoXY(0, 20);
			sprintf(str, "DO :%.2f", data.DO);
			SSD1306_Puts(str, &Font_7x10, 1);
			SSD1306_GotoXY(0, 30);
			sprintf(str, "EC :%.2f", data.EC);
			SSD1306_Puts(str, &Font_7x10, 1);
			SSD1306_GotoXY(0, 40);

			SSD1306_Puts(time_buff, &Font_7x10, 1);
			SSD1306_UpdateScreen();
			break;
		}
	}
	vTaskDelete(NULL);
}
void task_get_data(void *arg)
{

	//	init_param_DO(&nvsHandle);
	//	init_param_pH(&nvsHandle);
	//	TDS_init_param(&nvsHandle);

	data.temp = get_temp(ADC1_CHANNEL_5) - 5.0;
	printf("\nTemp value:%f\n", data.temp);

	gpio_set_level(EC_ON_PIN, 1);
	vTaskDelay(10000 / portTICK_PERIOD_MS);
	data.EC = EC_get_value(EC_AdC_CHANNEL, 4096.0, 3300.0, 26.0);
	gpio_set_level(EC_ON_PIN, 0);
	vTaskDelay(6000 / portTICK_PERIOD_MS);

	gpio_set_level(PH_ON_PIN, 1);
	vTaskDelay(10000 / portTICK_PERIOD_MS);
	data.pH = get_pH(nvsHandle, pH_ADC_CHANNEL, 3300.0, 4095.0);
	printf("pH value:%f\n", data.pH);
	gpio_set_level(PH_ON_PIN, 0);
	vTaskDelay(6000 / portTICK_PERIOD_MS);

	gpio_set_level(DO_ON_PIN, 1);
	vTaskDelay(10000 / portTICK_PERIOD_MS);
	data.DO = get_DO(nvsHandle, DO_ADC_CHANNEL, 3300.0, 4095.0);
	printf("DO value:%f \t", data.DO);
	gpio_set_level(DO_ON_PIN, 0);
	vTaskDelay(6000 / portTICK_PERIOD_MS);

	sprintf(buff, "%s|%.2f|%.2f|%.2f|%.2f", time_buff, data.temp, data.pH, data.DO, data.EC);
	// vTaskDelay(6000/portTICK_PERIOD_MS);
	xTaskCreatePinnedToCore(send_data_task, "send data uart task", 4096 * 2, NULL, 32, &send_data, tskNO_AFFINITY);
	xTaskCreatePinnedToCore(sd_task, "sd_task", 2048 * 2, NULL, 5, p_sd_card, tskNO_AFFINITY);
	xTaskCreatePinnedToCore(display_task, "oled Task", 2048 * 2, NULL, 20, &p_task_display, tskNO_AFFINITY);
	vTaskDelete(NULL);
}

void ds1307_task(void *pvParameters)
{
	//	ESP_ERROR_CHECK(i2cdev_init());
	//	i2c_dev_t dev;
	//	memset(&dev, 0, sizeof(i2c_dev_t));

	//	ESP_ERROR_CHECK(ds1307_init_desc(&dev, 0, 21, 22));

	struct tm time;
	time.tm_year = 123;
	time.tm_mon = 7;
	time.tm_mday = 14;
	time.tm_hour = 16;
	time.tm_min = 34;
	time.tm_sec = 10;
	ds1307_set_time(&dev, &time);

	ds1307_get_time(&dev, &time);
	sprintf(time_buff, "%04d|%02d|%02d|%02d|%02d|%02d", time.tm_year + 1900 /*Add 1900 for better readability*/, time.tm_mon,
			time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
	printf("%04d-%02d-%02d %02d:%02d:%02d\n", time.tm_year + 1900 /*Add 1900 for better readability*/, time.tm_mon,
		   time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);

	sprintf(name_file, "log%02d%02d", time.tm_mon,
			time.tm_mday);

	xTaskCreatePinnedToCore(task_get_data, "get_data", 4096 * 2, NULL, 1, &get_data, tskNO_AFFINITY);
	vTaskDelete(NULL);
}

void ds3231_task(void *pvParameter)
{
	// Initialize RTC
	while (1)
	{
		// Tiếp tục thực hiện công việc của luồng

		// Trước khi truy cập vào tài nguyên chia sẻ, khóa mutex
		if (xSemaphoreTake(I2C_mutex, portMAX_DELAY) == pdTRUE)
		{
			// Thực hiện truy cập vào tài nguyên chia sẻ ở đây
			i2c_dev_t dev;

			if (ds3231_init_desc(&dev, 0, 21, 22) != ESP_OK)
			{
				ESP_LOGE(pcTaskGetName(0), "Could not init device descriptor.");
				while (1)
				{
					vTaskDelay(1);
				}
			}

			struct tm time;
			//		time.tm_year = 2023;
			//		time.tm_mon = 7;
			//		time.tm_mday= 14;
			//		time.tm_hour = 17;
			//		time.tm_min = 15;
			//		time.tm_sec = 30;
			//		ds3231_set_time(&dev, &time);
			ds3231_get_time(&dev, &time);
			sprintf(time_buff, "%04d|%02d|%02d|%02d|%02d|%02d", time.tm_year /*Add 1900 for better readability*/, time.tm_mon,
					time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
			printf("%04d-%02d-%02d %02d:%02d:%02d\n", time.tm_year /*Add 1900 for better readability*/, time.tm_mon,
				   time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);

			sprintf(name_file, "log%02d%02d", time.tm_mon,
					time.tm_mday);
			xSemaphoreGive(I2C_mutex);
			break;
			// Sau khi hoàn thành việc truy cập, mở khóa mutex
		}

		// Tiếp tục thực hiện công việc của luồng
	}
	xTaskCreatePinnedToCore(task_get_data, "get_data", 2048 * 2, NULL, 1, &get_data, tskNO_AFFINITY);
	vTaskDelete(NULL);
}

void task_control(void *arg)
{

	for (;;)
	{
		xTaskCreatePinnedToCore(ds3231_task, "ds3231_task", 2048 * 2, NULL, 4, &test_ds3231, tskNO_AFFINITY);
		vTaskDelay(PERIOD / portTICK_PERIOD_MS);
		count_restart++;
		if (count_restart > 60)
		{ // after 60*period second esp will restart
			esp_restart();
		}
	}
}

void app_main(void)
{
	nvs_flash_init();
	ESP_ERROR_CHECK(i2cdev_init());
	I2C_mutex = xSemaphoreCreateMutex();
	memset(&dev, 0, sizeof(i2c_dev_t));

	esp_rom_gpio_pad_select_gpio(DO_ON_PIN1);
	gpio_set_direction(DO_ON_PIN1, GPIO_MODE_OUTPUT);

	esp_rom_gpio_pad_select_gpio(EC_ON_PIN1);
	gpio_set_direction(EC_ON_PIN1, GPIO_MODE_OUTPUT);

	esp_rom_gpio_pad_select_gpio(PH_ON_PIN);
	gpio_set_direction(PH_ON_PIN, GPIO_MODE_OUTPUT);
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(pH_ADC_CHANNEL, ADC_ATTEN_DB_11);

	esp_rom_gpio_pad_select_gpio(DO_ON_PIN);
	gpio_set_direction(DO_ON_PIN, GPIO_MODE_OUTPUT);
	adc1_config_channel_atten(DO_ADC_CHANNEL, ADC_ATTEN_DB_11);

	esp_rom_gpio_pad_select_gpio(EC_ON_PIN);
	gpio_set_direction(EC_ON_PIN, GPIO_MODE_OUTPUT);
	adc1_config_channel_atten(EC_AdC_CHANNEL, ADC_ATTEN_DB_11);

	adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);

	gpio_set_level(EC_ON_PIN, 0);
	gpio_set_level(PH_ON_PIN, 0);
	gpio_set_level(DO_ON_PIN, 0);
	gpio_set_level(EC_ON_PIN1, 0);
	gpio_set_level(DO_ON_PIN1, 0);

	// gpio_set_level(EC_ON_PIN1, 1);
	// gpio_set_level(EC_ON_PIN, 1);
	// gpio_set_level(PH_ON_PIN, 1);
	gpio_set_level(DO_ON_PIN1, 1);
	gpio_set_level(DO_ON_PIN, 1);

	init_param_pH(&nvsHandle);
	init_param_DO(&nvsHandle);
	EC_init_param(&nvsHandle);

	//	//	//xTaskCreatePinnedToCore(sd_task, "sd_task", 4096*2, NULL, 2, &p_sd_card, tskNO_AFFINITY);
	// xTaskCreatePinnedToCore(task_control, "Control task", 4096*2, NULL, 1, &p_task_control, tskNO_AFFINITY);
	// xTaskCreatePinnedToCore(Timeout_task, "timeout task", 4096*2, NULL, 3, &check_timeout_task_handle, tskNO_AFFINITY);
	// xTaskCreatePinnedToCore(uart_rx_task, "uart rx task", 4096*2, NULL, 1, &uart_rx_task_handle, tskNO_AFFINITY);
	// xTaskCreatePinnedToCore(send_data_task, "send data uart task", 4096*2, NULL, 32, &send_data, tskNO_AFFINITY);

	DO_Calib(ADC1_CHANNEL_7, 3300.0, 4096.0, nvsHandle, "storage", do_calib_100);
	while (1)
	{

		data.DO = get_DO(nvsHandle, DO_ADC_CHANNEL, 3300.0, 4095.0);
		printf("DO value:%f \t", data.DO);
		vTaskDelay(500);
		//
	}
	//	//xTaskCreatePinnedToCore(task_get_data, "get_data", 4096*2, NULL, 1, &get_data, tskNO_AFFINITY);
	//	//	xTaskCreatePinnedToCore(ds1307_task, "ds1307_test", 1046*2, NULL, 4, &test_ds1307, tskNO_AFFINITY);
}
