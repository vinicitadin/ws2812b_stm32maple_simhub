/**
 * @file LedController_Example.cpp
 * @brief Exemplo de uso da classe LedController para WS2812B no STM32F103C8T6
 * @date 2025-12-04
 * 
 * Este arquivo demonstra como usar a classe LedController para controlar
 * LEDs endereçáveis WS2812B usando Timer PWM + DMA no STM32F103.
 * 
 * Conexões recomendadas:
 * - PA0: TIM2_CH1 (padrão)
 * - PA6: TIM3_CH1
 * - PA7: TIM3_CH2
 * - PB0: TIM3_CH3
 * - PB1: TIM3_CH4
 * 
 * IMPORTANTE: Adicione um resistor de 330-470 ohms entre o pino de dados
 * e o primeiro LED da fita para proteger contra ruídos.
 */

#include <Arduino.h>
#include "LedController.h"

// Número de LEDs na fita
#define NUM_LEDS 30

// Criar controlador de LEDs (usa PA0/TIM2_CH1 por padrão)
LedController leds(NUM_LEDS);

// Variáveis para animações
uint8_t hue = 0;
uint32_t lastUpdate = 0;

void setup()
{
    // Inicializar LEDs
    if (!leds.begin()) {
        // Erro na inicialização - piscar LED onboard
        pinMode(PC13, OUTPUT);
        while (1) {
            digitalWrite(PC13, !digitalRead(PC13));
            delay(100);
        }
    }
    
    // Definir brilho (0-255)
    leds.setBrightness(50);
    
    // Exemplo 1: Acender todos os LEDs de vermelho
    leds.fill(255, 0, 0);
    leds.show();
    delay(1000);
    
    // Exemplo 2: Apagar todos
    leds.clear();
    leds.show();
    delay(500);
    
    // Exemplo 3: Acender LEDs individualmente
    for (int i = 0; i < NUM_LEDS; i++) {
        leds.setPixelColor(i, 0, 255, 0);  // Verde
        leds.show();
        delay(50);
    }
    delay(500);
    
    // Exemplo 4: Usar cor empacotada
    leds.clear();
    leds.setPixelColor(0, LedController::Color_RGB(255, 0, 0));    // Vermelho
    leds.setPixelColor(1, LedController::Color_RGB(0, 255, 0));    // Verde
    leds.setPixelColor(2, LedController::Color_RGB(0, 0, 255));    // Azul
    leds.setPixelColor(3, LedController::Color_RGB(255, 255, 0));  // Amarelo
    leds.setPixelColor(4, LedController::Color_RGB(255, 0, 255));  // Magenta
    leds.setPixelColor(5, LedController::Color_RGB(0, 255, 255));  // Ciano
    leds.show();
    delay(1000);
}

void loop()
{
    // Animação: Rainbow rotativo
    if (millis() - lastUpdate > 20) {
        lastUpdate = millis();
        
        // Criar arco-íris
        leds.rainbow(hue);
        leds.show();
        
        hue++;  // Incrementar matiz para animação
    }
}

// ==================== Configuração Alternativa com Pino Customizado ====================

/*
// Se você precisar usar outro pino, configure assim:

void setupCustomPin()
{
    LedConfig config;
    config.numLeds = 60;
    config.pin = PA6;           // Usar PA6
    config.timer = TIMER3;      // TIM3
    config.timerChannel = 1;    // Canal 1 (PA6 = TIM3_CH1)
    config.dma = DMA1;
    config.dmaChannel = DMA_CH6; // TIM3_CH1 usa DMA1_CH6
    config.colorOrder = ORDER_GRB;
    
    LedController customLeds(config);
    customLeds.begin();
}
*/

// ==================== Mapeamento de Pinos e DMA ====================
/*
 * Tabela de mapeamento Timer -> DMA para STM32F103:
 * 
 * Timer    Canal   Pino (sem remap)    DMA Channel
 * -------  ------  -----------------   -----------
 * TIM1     CH1     PA8                 DMA1_CH2
 * TIM1     CH2     PA9                 DMA1_CH3
 * TIM1     CH3     PA10                DMA1_CH6
 * TIM1     CH4     PA11                DMA1_CH4
 * 
 * TIM2     CH1     PA0                 DMA1_CH5
 * TIM2     CH2     PA1                 DMA1_CH7
 * TIM2     CH3     PA2                 DMA1_CH1
 * TIM2     CH4     PA3                 DMA1_CH7
 * 
 * TIM3     CH1     PA6                 DMA1_CH6
 * TIM3     CH2     PA7                 (não disponível via DMA)
 * TIM3     CH3     PB0                 DMA1_CH2
 * TIM3     CH4     PB1                 DMA1_CH3
 * 
 * TIM4     CH1     PB6                 DMA1_CH1
 * TIM4     CH2     PB7                 DMA1_CH4
 * TIM4     CH3     PB8                 DMA1_CH5
 * TIM4     CH4     PB9                 (não disponível via DMA)
 * 
 * NOTA: Alguns canais compartilham o mesmo canal DMA.
 * Verifique se não há conflitos no seu projeto!
 */

// ==================== Funções de Efeito Extras ====================

/**
 * @brief Efeito "Cylon" (LED vai e volta)
 */
void effectCylon(LedController& strip, Color color, uint16_t delayMs)
{
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.clear();
        strip.setPixelColor(i, color);
        strip.show();
        delay(delayMs);
    }
    for (int i = strip.numPixels() - 2; i > 0; i--) {
        strip.clear();
        strip.setPixelColor(i, color);
        strip.show();
        delay(delayMs);
    }
}

/**
 * @brief Efeito "Theater Chase"
 */
void effectTheaterChase(LedController& strip, Color color, uint16_t cycles, uint16_t delayMs)
{
    for (uint16_t c = 0; c < cycles; c++) {
        for (int q = 0; q < 3; q++) {
            strip.clear();
            for (int i = 0; i < strip.numPixels(); i += 3) {
                if (i + q < strip.numPixels()) {
                    strip.setPixelColor(i + q, color);
                }
            }
            strip.show();
            delay(delayMs);
        }
    }
}

/**
 * @brief Efeito "Color Wipe" - preenche a fita gradualmente
 */
void effectColorWipe(LedController& strip, Color color, uint16_t delayMs)
{
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, color);
        strip.show();
        delay(delayMs);
    }
}

/**
 * @brief Efeito "Sparkle" - pisca LEDs aleatórios
 */
void effectSparkle(LedController& strip, Color color, uint16_t count, uint16_t delayMs)
{
    for (uint16_t i = 0; i < count; i++) {
        int led = random(strip.numPixels());
        strip.setPixelColor(led, color);
        strip.show();
        delay(delayMs);
        strip.setPixelColor(led, 0);
    }
}

/**
 * @brief Efeito "Breathing" - pulsa o brilho
 */
void effectBreathing(LedController& strip, Color color, uint16_t cycles)
{
    for (uint16_t c = 0; c < cycles; c++) {
        // Aumentar brilho
        for (int b = 0; b < 255; b += 5) {
            strip.setBrightness(b);
            strip.fill(color);
            strip.show();
            delay(10);
        }
        // Diminuir brilho
        for (int b = 255; b > 0; b -= 5) {
            strip.setBrightness(b);
            strip.fill(color);
            strip.show();
            delay(10);
        }
    }
    strip.setBrightness(255); // Restaurar brilho
}
