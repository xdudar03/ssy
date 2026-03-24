/*
 * cli_usart.c
 *
 *  Created on: 3. 3. 2026
 *      Author: Student
 */

#include "cli_uart.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "main.h"
#include "usart.h"
#include "cv2.h"
#include "tim.h"

/** Velikost RX kruhového bufferu (znaky přijaté v ISR) */
#define CLI_RX_BUF_SIZE    256

/** Velikost řádkového bufferu (sestavovaný příkaz) */
#define CLI_LINE_BUF_SIZE  128

/* RX kruhový buffer */
static volatile uint8_t  cli_rx_buf[CLI_RX_BUF_SIZE];
static volatile uint16_t cli_rx_head = 0;
static volatile uint16_t cli_rx_tail = 0;

/* Sestavovaný řádek (příkaz) */
static char     cli_line_buf[CLI_LINE_BUF_SIZE];
static uint16_t cli_line_len = 0;

/* Echo (0 = vypnuto, 1 = zapnuto) */
static uint8_t echo = 0;

/* 1-bajtový buffer pro HAL_UART_Receive_IT() */
static uint8_t cli_uart_rx1 = 0;

/* Dopředná deklarace parseru řádku */
static void CLI_HandleLine(char *line);

/**
 * @brief Odeslání dat přes UART3 (blocking).
 *
 * Pro CLI je blocking TX typicky v pohodě (krátké řetězce). Pokud bys chtěl
 * plně neblokující výstup, udělej TX frontu + HAL_UART_Transmit_IT/DMA.
 *
 * @param data Pointer na data
 * @param len  Délka v bajtech
 */
static void CLI_UART_Send(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0) {
        return;
    }

    /* HAL_UART_Transmit může vrátit HAL_BUSY, pokud někdo používá IT/DMA TX.
       V tom případě to jednoduše zkusíme znovu. */
    while (HAL_UART_Transmit(&huart3, (uint8_t*)data, len, 1000) == HAL_BUSY) {
        /* krátké "odpočkat" – pro terminál stačí */
        HAL_Delay(1);
    }
}

/**
 * @brief Odeslání NUL-terminated stringu do UART terminálu.
 */
void CLI_UART_Print(const char *s)
{
    if (s == NULL) return;
    CLI_UART_Send((const uint8_t*)s, (uint16_t)strlen(s));
}

/**
 * @brief Formátovaný výstup do UART terminálu (printf-like).
 */
void CLI_UART_Printf(const char *fmt, ...)
{
    char buf[128];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    CLI_UART_Print(buf);
}

/**
 * @brief Vloží jeden přijatý bajt do kruhového bufferu.
 *
 * Volat z ISR/callbacku (HAL_UART_RxCpltCallback), aby hlavní smyčka nebyla blokovaná.
 * Při overflow se znak zahodí.
 */
void CLI_UART_OnRxByte(uint8_t b)
{
    uint16_t next = (cli_rx_head + 1u) % CLI_RX_BUF_SIZE;

    if (next != cli_rx_tail) {
        cli_rx_buf[cli_rx_head] = b;
        cli_rx_head = next;
    } else {
        /* Overflow – buffer je plný, znak zahodíme.
           (Volitelně lze přidat počitadlo overflowů.) */
    }
}

/**
 * @brief Přečte 1 znak z RX ring-bufferu.
 *
 * @return >=0 znak, -1 pokud je buffer prázdný
 */
static int CLI_RX_GetChar(void)
{
    if (cli_rx_head == cli_rx_tail) {
        return -1; /* nic k přečtení */
    }

    uint8_t c = cli_rx_buf[cli_rx_tail];
    cli_rx_tail = (cli_rx_tail + 1u) % CLI_RX_BUF_SIZE;
    return (int)c;
}

/**
 * @brief Inicializace UART CLI.
 *

 * - vynuluje interní stavy,
 * - vypíše uvítací text,
 * - spustí RX přes přerušení (Receive_IT) po 1 bajtu.
 */
void CLI_UART_Init()
{

    cli_rx_head = 0;
    cli_rx_tail = 0;
    cli_line_len = 0;
    echo = 1;

    CLI_UART_Print("\r\nCLI ready (UART3). Type 'help' for commands.\r\n> ");

    /* Spuštění příjmu 1 bajtu v přerušení.
       Pokud USART3 ještě není inicializovaný, HAL vrátí chybu. */
    (void)HAL_UART_Receive_IT(&huart3, &cli_uart_rx1, 1);
}

/**
 * @brief Zpracování CLI – volej často v main loop.
 *
 * Funkce vybírá znaky z RX ring-bufferu, skládá řádek (příkaz) a po Enter
 * zavolá parser CLI_HandleLine().
 *
 * Podporuje:
 *  - CR/LF jako Enter
 *  - backspace (0x08) a DEL (0x7F)
 *  - volitelné echo (echoon/echooff)
 */
void CLI_UART_Process(void)
{
    int ch;

    while ((ch = CLI_RX_GetChar()) >= 0) {
        char c = (char)ch;

        /* Enter (CR nebo LF) */
        if (c == '\r') {

            if (echo) {
                /* Standardní "nový řádek" v terminálu */
                CLI_UART_Print("\r\n");
            }

            if (cli_line_len > 0) {
                cli_line_buf[cli_line_len] = '\0';
                CLI_HandleLine(cli_line_buf);
                cli_line_len = 0;
                CLI_UART_Print("> ");
            } else {
                /* prázdný řádek – jen prompt */
                CLI_UART_Print("> ");
            }

            continue;
        }
        if (c == '\n')
        {
            continue;
        }

        /* Backspace nebo DEL */
        if (c == '\b' || c == 0x7F) {
            if (cli_line_len > 0) {
                cli_line_len--;

                if (echo) {
                    /* Většina terminálů: smazání znaku = "\b \b" */
                    CLI_UART_Print("\b \b");
                }
            } else {
                /* nic k mazání – volitelně lze poslat bell '\a' */
            }
            continue;
        }

        /* Běžný znak – přidat do line bufferu */
        if (cli_line_len < (CLI_LINE_BUF_SIZE - 1u)) {
            cli_line_buf[cli_line_len++] = c;

            if (echo) {
                CLI_UART_Send((uint8_t*)&c, 1);
            }
        } else {
            /* Overflow řádku – zahodit a začít znovu */
            cli_line_len = 0;
            CLI_UART_Print("\r\nLine too long, clearing.\r\n> ");
        }
    }
}

static void CLI_ShowHelp(void)
{
    CLI_UART_Print(
        "Available commands:\r\n"
        "  help, ?                      - show this help\r\n"
    	"  rled on 						- turn on red LED\r\n"
    	"  rled off 					- turn off red LED\r\n"
    	"  led stop 					- stop blue LED from blinking\r\n"
    	"  ECHO 						- turn on/off echo\r\n"
    	"  STATE 						- button state\r\n"
    );
}

static void CLI_RLED_On(void) {
	LED_init(GPIOB, 14);
	LED_ON(GPIOB, 14);
}

static void CLI_RLED_Off(void) {
	LED_OFF(GPIOB, 14);
}

static void CLI_LED_Stop(void) {
	HAL_TIM_OC_Stop_IT(&htim4, TIM_CHANNEL_2);
}

static void CLI_LED_Set(uint32_t ms) {
	uint32_t ticks = ms / 100;   // krok 100 ms

	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, ticks);

	HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_2);
}

static void CLI_PWM_Stop(void) {
	HAL_TIM_PWM_Stop_IT(&htim1, TIM_CHANNEL_2);
}

static void CLI_PWM_Set(uint32_t duty) {
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, duty);
}

static void CLI_Echo(void) {
	if(echo == 0 ) {
		echo = 1;
	} else {
		echo = 0;
	}
}

static void CLI_State(void) {
	GPIO_PinState state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

	if (state == GPIO_PIN_RESET)
	{
		CLI_UART_Print("Button: RELEASED\r\n");
	} else {
		CLI_UART_Print("Button: PRESSED\r\n");
	}

}


/**
 * @brief Parsuje a vykoná jeden řádek příkazu.
 *
 * Používá jednoduché tokenizování pomocí strtok().
 * Příkazy jsou zachované z cli_usb.c.
 */
static void CLI_HandleLine(char *line)
{
    /* Trim: odstranění mezer na začátku a na konci */
    char *p = line;
    while (*p == ' ' || *p == '\t') p++;

    char *end = p + strlen(p);
    while (end > p && (end[-1] == ' ' || end[-1] == '\t')) {
        *--end = '\0';
    }

    if (*p == '\0') {
        return; /* prázdný příkaz */
    }

    /* první token = příkaz */
    char *cmd = strtok(p, " ");
    if (cmd == NULL) return;

    // pro pokracovani retezcu
    char *arg = strtok(NULL, " ");

    /* help */
    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) {
        CLI_ShowHelp();
        return;
    }

    /* RLED */
    if (strcmp(cmd, "rled") == 0)
    {
        if (arg && strcmp(arg, "on") == 0)
        {
            CLI_RLED_On();
            return;
        }
        if (arg && strcmp(arg, "off") == 0)
        {
            CLI_RLED_Off();
            return;
        }

        CLI_UART_Print("Usage: rled on|off\r\n");
        return;
    }

    /* LED */
	if (strcmp(cmd, "led") == 0)
	{
		if (arg && strcmp(arg, "stop") == 0)
		{
			CLI_LED_Stop();
			return;
		}
		if (arg)
		{
			uint32_t ms = atoi(arg);
			CLI_LED_Set(ms);
			return;
		}
	}

	/* PWM */
	if (strcmp(cmd, "pwm") == 0)
		{
			if (arg && strcmp(arg, "stop") == 0)
			{
				CLI_PWM_Stop();
				return;
			}
			if (arg)
			{
				uint32_t duty = atoi(arg);
				CLI_PWM_Set(duty);
				return;
			}
		}

    /* ECHO */
    if(strcmp(cmd, "ECHO") == 0){
    	CLI_Echo();
    	CLI_UART_Printf("ECHO is now %d. 0 - turned off, 1 - turned on.", echo);
    	return;
    }

    /* STATE */
    if(strcmp(cmd, "STATE") == 0){
		CLI_State();
		return;
	}

    /* Neznámý příkaz */
    CLI_UART_Print("Unknown command. Type 'help'.\r\n");
}
void CLI_UART_RxCallback(void)
{
    CLI_UART_OnRxByte(cli_uart_rx1);
    HAL_UART_Receive_IT(&huart3, &cli_uart_rx1, 1);
}
