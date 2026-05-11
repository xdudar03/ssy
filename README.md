## Obsah repozitáře

- `project/` - hlavní semestrální projekt.
- `cv1/` - projekt ze cvičení 1.
- `SSY-CV2-4/` - projekt kombinující práci ze cvičení 2 až 4.

# Semestrální projekt SSY

Tento projekt je ukázkou vestavěného systému postaveného na platformě STM32F4. Program běží na desce `NUCLEO-F439ZI` a propojuje několik periferií: Ethernet, MQTT komunikaci, tlakový a teplotní senzor BMP180, IR senzor a SPI TFT displej ILI9341.

Cílem projektu je vytvořit jednoduché IoT zařízení, které získává data ze senzorů, zpracovává je v mikrokontroléru a odesílá je přes síť do MQTT brokeru. Naměřené hodnoty je díky tomu možné dál zobrazovat, ukládat nebo zpracovávat v jiné aplikaci.

## Popis programu

Po spuštění firmware nejdříve inicializuje mikrokontrolér a všechny použité periferie. Nastaví GPIO piny, sériovou linku `USART3`, Ethernet s LwIP stackem, sběrnici `I2C1` pro BMP180, `ADC1` pro analogový výstup IR senzoru a `SPI1` pro displej ILI9341.

Následně se inicializuje MQTT klient, spustí se získání IP adresy pomocí DHCP a připraví se senzor BMP180. Program také provede jednoduchý test displeje, při kterém vyplní obrazovku bílou barvou a do rohů vykreslí barevné obdélníky. Tento test slouží jako kontrola, že SPI komunikace s displejem funguje.

Hlavní část programu běží v nekonečné smyčce. V každém průchodu se obsluhuje síťová vrstva LwIP a stav MQTT klienta. Program po úspěšném připojení k MQTT brokeru periodicky odesílá naměřená data.

## Měřená a odesílaná data

Program pracuje se dvěma skupinami dat:

- `BMP180` měří teplotu a atmosférický tlak.
- IR senzor poskytuje digitální informaci o detekci a analogovou hodnotu přes ADC.

Data z BMP180 se publikují přibližně každých 5 sekund na MQTT topic `SSY/bmp180` ve formátu:

```json
{ "temp_c10": 25, "pressure_pa": 101325 }
```

Data z IR senzoru se publikují přibližně každých 500 ms na MQTT topic `SSY/ir` ve formátu:

```json
{ "detected": 1, "adc": 1234 }
```

Hodnota `detected` vychází z digitálního vstupu senzoru. Hodnota `adc` odpovídá analogové hodnotě přečtené převodníkem `ADC1`.

## Síťová komunikace

Síťová komunikace je realizována přes Ethernet. Projekt používá LwIP bez operačního systému a IP adresa se získává pomocí DHCP. Stav sítě je průběžně zpracováván voláním `MX_LWIP_Process()` v hlavní smyčce.

MQTT klient je implementovaný v souboru `project/Core/Src/mqtt_client.c`. Po získání IP adresy se klient pokusí připojit k brokeru, přihlásí se k odběru topicu `SSY/cmd` a poté umožňuje publikovat zprávy se senzorickými daty. Pokud připojení není dostupné, klient se periodicky pokouší o nové připojení.

Aktuální nastavení MQTT:

- broker: `192.168.53.15`,
- port: `1883`,
- client ID: `stm32_ssy_01`,
- subscribe topic: `SSY/cmd`.

## Použitý hardware

- Vývojová deska `NUCLEO-F439ZI`.
- Mikrokontrolér `STM32F439ZITx`.
- Ethernet PHY `LAN8742`.
- Tlakový a teplotní senzor `BMP180` připojený přes `I2C1`.
- TFT displej `ILI9341` připojený přes `SPI1`.
- IR senzor s digitálním výstupem na `PA6` a analogovým výstupem přivedeným na ADC.

Konkrétní konfigurace pinů je uložená v souboru `project/project.ioc`.

## Struktura implementace

- `project/Core/Src/main.c` obsahuje inicializaci periferií a hlavní smyčku programu.
- `project/Core/Src/mqtt_client.c` obsahuje MQTT klienta, připojení k brokeru, subscribe a publish logiku.
- `project/Core/Src/bmp180.c` a `project/Core/Inc/bmp180.h` zajišťují komunikaci se senzorem BMP180.
- `project/Core/Src/ili9341.c` a `project/Core/Inc/ili9341.h` obsahují obsluhu displeje ILI9341.
- `project/LWIP/App/lwip.c` obsahuje inicializaci LwIP, síťového rozhraní a DHCP.
- `project/LWIP/Target/lwipopts.h` obsahuje nastavení LwIP stacku.
