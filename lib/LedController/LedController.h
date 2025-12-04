/**
 * @file LedController.h
 * @brief Classe de controle de LEDs endereçáveis WS2812B para STM32F103C8T6 com core Maple
 * @version 1.0
 * @date 2025-12-04
 * 
 * Utiliza Timer PWM + DMA para gerar o sinal de dados dos LEDs WS2812B.
 * O método DMA permite transmissão não-bloqueante e timing preciso.
 * 
 * Timings WS2812B @ 800kHz:
 * - T0H: 0.4us (0.35-0.50us)
 * - T0L: 0.85us (0.70-1.00us)  
 * - T1H: 0.8us (0.70-1.00us)
 * - T1L: 0.45us (0.35-0.50us)
 * - Reset: >50us
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef __LED_CONTROLLER_H__
#define __LED_CONTROLLER_H__

#include <Arduino.h>
#include <libmaple/timer.h>
#include <libmaple/dma.h>
#include <libmaple/rcc.h>
#include <libmaple/gpio.h>

// Configurações de timing para WS2812B @ 72MHz
// Período PWM = 90 ciclos = 1.25us @ 72MHz (800kHz)
#define WS2812_PWM_PERIOD       90
#define WS2812_PWM_HIGH         58      // ~0.8us para bit 1
#define WS2812_PWM_LOW          29      // ~0.4us para bit 0
#define WS2812_RESET_CYCLES     50      // Ciclos de reset (>50us)

// Bytes por LED (GRB = 3 bytes, GRBW = 4 bytes)
#define BYTES_PER_LED_RGB       3
#define BYTES_PER_LED_RGBW      4
#define BITS_PER_BYTE           8

// Cor empacotada em 32 bits
typedef uint32_t Color;

/**
 * @brief Enumeração para ordem de cores
 */
enum ColorOrder {
    ORDER_GRB = 0,  // WS2812B padrão
    ORDER_RGB,
    ORDER_BRG,
    ORDER_BGR,
    ORDER_GRBW,     // SK6812 RGBW
    ORDER_RGBW
};

/**
 * @brief Estrutura para configuração do LED Controller
 */
struct LedConfig {
    uint16_t numLeds;           // Número de LEDs
    uint8_t pin;                // Pino de dados (PA0-PA3 para TIM2, PA6-PA7 para TIM3, etc)
    timer_dev* timer;           // Timer device (TIMER2, TIMER3, etc)
    uint8_t timerChannel;       // Canal do timer (1-4)
    dma_dev* dma;               // DMA device (DMA1)
    dma_channel dmaChannel;     // Canal DMA
    ColorOrder colorOrder;      // Ordem das cores
    
    // Construtor com valores padrão para pino PA0 (TIM2_CH1)
    LedConfig() : 
        numLeds(1),
        pin(PA0),
        timer(TIMER2),
        timerChannel(1),
        dma(DMA1),
        dmaChannel(DMA_CH2),  // TIM2_CH1 usa DMA1_CH2
        colorOrder(ORDER_GRB) {}
};

/**
 * @brief Classe principal para controle de LEDs WS2812B
 */
class LedController {
public:
    /**
     * @brief Construtor
     * @param numLeds Número de LEDs na fita
     */
    LedController(uint16_t numLeds);
    
    /**
     * @brief Construtor com configuração completa
     * @param config Estrutura de configuração
     */
    LedController(const LedConfig& config);
    
    /**
     * @brief Destrutor
     */
    ~LedController();
    
    /**
     * @brief Inicializa o controlador
     * @return true se inicializado com sucesso
     */
    bool begin();
    
    /**
     * @brief Envia os dados para os LEDs
     */
    void show();
    
    /**
     * @brief Verifica se uma transmissão DMA está em andamento
     * @return true se ocupado
     */
    bool isBusy();
    
    /**
     * @brief Aguarda o fim da transmissão atual
     */
    void wait();
    
    // ==================== Métodos de cor ====================
    
    /**
     * @brief Define a cor de um LED específico
     * @param index Índice do LED (0 a numLeds-1)
     * @param r Componente vermelho (0-255)
     * @param g Componente verde (0-255)
     * @param b Componente azul (0-255)
     */
    void setPixelColor(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
    
    /**
     * @brief Define a cor de um LED específico (RGBW)
     * @param index Índice do LED
     * @param r Vermelho
     * @param g Verde
     * @param b Azul
     * @param w Branco
     */
    void setPixelColor(uint16_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    
    /**
     * @brief Define a cor de um LED usando cor empacotada
     * @param index Índice do LED
     * @param color Cor em formato 0x00RRGGBB ou 0xWWRRGGBB
     */
    void setPixelColor(uint16_t index, Color color);
    
    /**
     * @brief Obtém a cor atual de um LED
     * @param index Índice do LED
     * @return Cor empacotada
     */
    Color getPixelColor(uint16_t index);
    
    /**
     * @brief Define todos os LEDs com a mesma cor
     * @param r Vermelho
     * @param g Verde
     * @param b Azul
     */
    void fill(uint8_t r, uint8_t g, uint8_t b);
    
    /**
     * @brief Define todos os LEDs com a mesma cor
     * @param color Cor empacotada
     */
    void fill(Color color);
    
    /**
     * @brief Define uma faixa de LEDs
     * @param startIndex Índice inicial
     * @param count Quantidade de LEDs
     * @param color Cor
     */
    void fillRange(uint16_t startIndex, uint16_t count, Color color);
    
    /**
     * @brief Apaga todos os LEDs
     */
    void clear();
    
    /**
     * @brief Define o brilho global (aplicado no show())
     * @param brightness Brilho (0-255)
     */
    void setBrightness(uint8_t brightness);
    
    /**
     * @brief Obtém o brilho atual
     * @return Brilho (0-255)
     */
    uint8_t getBrightness() const;
    
    // ==================== Métodos estáticos de cor ====================
    
    /**
     * @brief Cria uma cor RGB empacotada
     */
    static Color Color_RGB(uint8_t r, uint8_t g, uint8_t b);
    
    /**
     * @brief Cria uma cor RGBW empacotada
     */
    static Color Color_RGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    
    /**
     * @brief Cria uma cor a partir do modelo HSV
     * @param h Matiz (0-255)
     * @param s Saturação (0-255)
     * @param v Valor/Brilho (0-255)
     */
    static Color ColorHSV(uint8_t h, uint8_t s, uint8_t v);
    
    /**
     * @brief Interpola entre duas cores
     * @param color1 Cor inicial
     * @param color2 Cor final
     * @param amount Quantidade de interpolação (0-255)
     */
    static Color blend(Color color1, Color color2, uint8_t amount);
    
    // ==================== Efeitos ====================
    
    /**
     * @brief Efeito rainbow (arco-íris)
     * @param startHue Matiz inicial
     */
    void rainbow(uint8_t startHue = 0);
    
    /**
     * @brief Efeito de rotação dos LEDs
     * @param positions Número de posições para rotacionar (positivo = direita)
     */
    void rotate(int16_t positions = 1);
    
    /**
     * @brief Shift dos LEDs (similar a rotate, mas preenche com preto)
     * @param positions Posições para shiftar
     */
    void shift(int16_t positions = 1);
    
    // ==================== Getters ====================
    
    uint16_t numPixels() const { return _numLeds; }
    uint8_t getPin() const { return _config.pin; }
    
    /**
     * @brief Verifica se pode enviar (respeitando tempo de reset)
     */
    bool canShow();

private:
    LedConfig _config;
    uint16_t _numLeds;
    uint8_t _brightness;
    bool _begun;
    
    // Buffer de cores (RGB ou RGBW por LED)
    uint8_t* _pixelBuffer;
    uint16_t _pixelBufferSize;
    
    // Buffer DMA (valores PWM para cada bit + reset)
    uint16_t* _dmaBuffer;
    uint16_t _dmaBufferSize;
    
    // Controle de timing
    uint32_t _lastShowTime;
    static const uint32_t RESET_TIME_US = 300;  // Tempo de reset em microsegundos
    
    // Métodos internos
    void _initTimer();
    void _initDMA();
    void _initGPIO();
    void _encodePixels();
    void _encodeByte(uint8_t byte, uint16_t* dest);
    uint8_t _applyBrightness(uint8_t value);
    
    // Mapeamento Timer->DMA Channel
    dma_channel _getTimerDMAChannel();
    volatile uint32_t* _getTimerCCR();
};

#endif // __LED_CONTROLLER_H__
