/**
 * @file LedController.cpp
 * @brief Implementação da classe LedController para WS2812B no STM32F103 com core Maple
 * @version 1.0
 * @date 2025-12-04
 * 
 * @copyright Copyright (c) 2025
 */

#include "LedController.h"
#include <string.h>

// ==================== Construtores/Destrutor ====================

LedController::LedController(uint16_t numLeds) :
    _numLeds(numLeds),
    _brightness(255),
    _begun(false),
    _pixelBuffer(nullptr),
    _dmaBuffer(nullptr),
    _lastShowTime(0)
{
    _config.numLeds = numLeds;
    _config.pin = PA7;
    _config.timer = TIMER3;
    _config.timerChannel = 2;
    _config.dma = DMA1;
    _config.dmaChannel = DMA_CH3;  // TIM3_UP usa DMA1_CH3
    _config.colorOrder = ORDER_GRB;
}

LedController::LedController(const LedConfig& config) :
    _config(config),
    _numLeds(config.numLeds),
    _brightness(255),
    _begun(false),
    _pixelBuffer(nullptr),
    _dmaBuffer(nullptr),
    _lastShowTime(0)
{
}

LedController::~LedController()
{
    if (_pixelBuffer) {
        free(_pixelBuffer);
        _pixelBuffer = nullptr;
    }
    if (_dmaBuffer) {
        free(_dmaBuffer);
        _dmaBuffer = nullptr;
    }
    
    // Desabilitar DMA e Timer
    if (_begun) {
        dma_disable(_config.dma, _config.dmaChannel);
        timer_pause(_config.timer);
    }
}

// ==================== Inicialização ====================

bool LedController::begin()
{
    if (_begun) return true;
    
    // Calcular tamanho dos buffers
    uint8_t bytesPerLed = (_config.colorOrder == ORDER_GRBW || _config.colorOrder == ORDER_RGBW) 
                          ? BYTES_PER_LED_RGBW : BYTES_PER_LED_RGB;
    
    _pixelBufferSize = _numLeds * bytesPerLed;
    // Buffer DMA: bits por LED + reset pulses
    _dmaBufferSize = (_numLeds * bytesPerLed * BITS_PER_BYTE) + WS2812_RESET_CYCLES;
    
    // Alocar buffer de pixels
    _pixelBuffer = (uint8_t*)malloc(_pixelBufferSize);
    if (!_pixelBuffer) {
        return false;
    }
    memset(_pixelBuffer, 0, _pixelBufferSize);
    
    // Alocar buffer DMA (uint16_t para valores PWM)
    _dmaBuffer = (uint16_t*)malloc(_dmaBufferSize * sizeof(uint16_t));
    if (!_dmaBuffer) {
        free(_pixelBuffer);
        _pixelBuffer = nullptr;
        return false;
    }
    memset(_dmaBuffer, 0, _dmaBufferSize * sizeof(uint16_t));
    
    // Inicializar periféricos
    _initGPIO();
    _initTimer();
    _initDMA();
    
    _begun = true;
    
    // Enviar dados iniciais (todos apagados)
    clear();
    show();
    
    return true;
}

void LedController::_initGPIO()
{
    // Habilitar clock AFIO para remapeamento se necessário
    rcc_clk_enable(RCC_AFIO);
    
    // Configurar pino como saída alternativa push-pull
    gpio_dev* port;
    uint8_t pinNum;
    
    // Determinar porta e pino
    if (_config.pin >= PA0 && _config.pin <= PA15) {
        port = GPIOA;
        pinNum = _config.pin - PA0;
        rcc_clk_enable(RCC_GPIOA);
    } else if (_config.pin >= PB0 && _config.pin <= PB15) {
        port = GPIOB;
        pinNum = _config.pin - PB0;
        rcc_clk_enable(RCC_GPIOB);
    } else {
        port = GPIOA;
        pinNum = 0;
        rcc_clk_enable(RCC_GPIOA);
    }
    
    // Configurar pino como alternate function push-pull (50MHz)
    gpio_set_mode(port, pinNum, GPIO_AF_OUTPUT_PP);
}

void LedController::_initTimer()
{
    timer_dev* tim = _config.timer;
    uint8_t ch = _config.timerChannel;
    
    // Habilitar clock do timer
    if (tim == TIMER2) {
        rcc_clk_enable(RCC_TIMER2);
    } else if (tim == TIMER3) {
        rcc_clk_enable(RCC_TIMER3);
    } else if (tim == TIMER4) {
        rcc_clk_enable(RCC_TIMER4);
    } else if (tim == TIMER1) {
        rcc_clk_enable(RCC_TIMER1);
    }
    
    // Pausar timer
    timer_pause(tim);
    
    // Reset completo do timer
    tim->regs.gen->CR1 = 0;
    tim->regs.gen->CR2 = 0;
    tim->regs.gen->DIER = 0;
    
    // Prescaler = 0 (sem divisão, 72MHz direto)
    tim->regs.gen->PSC = 0;
    
    // Auto-reload = 89 (período de 90 ciclos = 1.25us @ 72MHz)
    tim->regs.gen->ARR = WS2812_PWM_PERIOD - 1;
    
    // Configurar canal PWM
    // CCMRx: OC1M = 110 (PWM mode 1), OC1PE = 1 (preload enable)
    uint16_t ccmr_val = (0x6 << 4) | (1 << 3);  // PWM mode 1 + preload
    
    if (ch == 1) {
        tim->regs.gen->CCMR1 = ccmr_val;
        tim->regs.gen->CCER |= (1 << 0);  // CC1E = 1 (enable output)
        tim->regs.gen->CCR1 = 0;
        tim->regs.gen->DIER |= (1 << 9);  // CC1DE = 1 (DMA request enable)
    } else if (ch == 2) {
        tim->regs.gen->CCMR1 = (ccmr_val << 8);
        tim->regs.gen->CCER |= (1 << 4);  // CC2E
        tim->regs.gen->CCR2 = 0;
        tim->regs.gen->DIER |= (1 << 10); // CC2DE
    } else if (ch == 3) {
        tim->regs.gen->CCMR2 = ccmr_val;
        tim->regs.gen->CCER |= (1 << 8);  // CC3E
        tim->regs.gen->CCR3 = 0;
        tim->regs.gen->DIER |= (1 << 11); // CC3DE
    } else if (ch == 4) {
        tim->regs.gen->CCMR2 = (ccmr_val << 8);
        tim->regs.gen->CCER |= (1 << 12); // CC4E
        tim->regs.gen->CCR4 = 0;
        tim->regs.gen->DIER |= (1 << 12); // CC4DE
    }
    
    // Para Timer1 (advanced timer), habilitar MOE
    if (tim == TIMER1) {
        TIMER1->regs.adv->BDTR |= (1 << 15);  // MOE
    }
    
    // CR1: ARPE = 1 (auto-reload preload)
    tim->regs.gen->CR1 = (1 << 7);
    
    // Gerar update event para carregar registradores
    tim->regs.gen->EGR = 1;
}

void LedController::_initDMA()
{
    // Habilitar clock do DMA
    rcc_clk_enable(RCC_DMA1);
    
    // Inicializar DMA
    dma_init(_config.dma);
}

volatile uint32_t* LedController::_getTimerCCR()
{
    timer_dev* tim = _config.timer;
    uint8_t ch = _config.timerChannel;
    
    // Retornar ponteiro para o registrador CCR correto
    switch (ch) {
        case 1: return &(tim->regs.gen->CCR1);
        case 2: return &(tim->regs.gen->CCR2);
        case 3: return &(tim->regs.gen->CCR3);
        case 4: return &(tim->regs.gen->CCR4);
        default: return &(tim->regs.gen->CCR1);
    }
}

// ==================== Transmissão ====================

void LedController::show()
{
    if (!_begun) return;
    
    // Aguardar tempo de reset
    while (!canShow()) { /* esperar */ }
    
    // Codificar pixels para o buffer DMA
    _encodePixels();
    
    // Obter ponteiro para registradores DMA
    dma_tube_reg_map* tube = dma_tube_regs(_config.dma, _config.dmaChannel);
    
    // Desabilitar DMA
    tube->CCR = 0;
    
    // Limpar flags
    dma_clear_isr_bits(_config.dma, _config.dmaChannel);
    
    // Configurar DMA
    tube->CMAR = (uint32_t)_dmaBuffer;
    tube->CPAR = (uint32_t)_getTimerCCR();
    tube->CNDTR = _dmaBufferSize;
    
    // CCR: PL=high, MSIZE=16bit, PSIZE=16bit, MINC=1, DIR=mem2periph, TCIE=1
    tube->CCR = DMA_CCR_PL_HIGH | DMA_CCR_MSIZE_16BITS | DMA_CCR_PSIZE_16BITS |
                DMA_CCR_MINC | DMA_CCR_DIR_FROM_MEM | DMA_CCR_TCIE;
    
    // Habilitar DMA request no timer (UDE = Update DMA Enable)
    _config.timer->regs.gen->DIER |= TIMER_DIER_UDE;
    
    // Habilitar DMA
    tube->CCR |= DMA_CCR_EN;
    
    // Zerar contador e iniciar timer
    _config.timer->regs.gen->CNT = 0;
    _config.timer->regs.gen->CR1 |= TIMER_CR1_CEN;
    
    // Aguardar transferência completar
    while (!(dma_get_isr_bits(_config.dma, _config.dmaChannel) & DMA_ISR_TCIF)) { }
    
    // Parar timer
    _config.timer->regs.gen->CR1 &= ~TIMER_CR1_CEN;
    
    // Desabilitar DMA request
    _config.timer->regs.gen->DIER &= ~TIMER_DIER_UDE;
    
    // Desabilitar DMA
    tube->CCR = 0;
    
    // Limpar flags
    dma_clear_isr_bits(_config.dma, _config.dmaChannel);
    
    // Zerar CCR do timer para garantir saída em LOW
    volatile uint32_t* ccr = _getTimerCCR();
    *ccr = 0;
    
    // Registrar tempo
    _lastShowTime = micros();
}

bool LedController::isBusy()
{
    if (!_begun) return false;
    
    // Verificar se DMA ainda está transferindo
    return (dma_get_isr_bits(_config.dma, _config.dmaChannel) & DMA_ISR_TCIF) == 0;
}

void LedController::wait()
{
    while (isBusy()) { /* esperar */ }
}

bool LedController::canShow()
{
    return (micros() - _lastShowTime) >= RESET_TIME_US;
}

// ==================== Codificação ====================

void LedController::_encodePixels()
{
    uint16_t* dmaPtr = _dmaBuffer;
    uint8_t bytesPerLed = (_config.colorOrder == ORDER_GRBW || _config.colorOrder == ORDER_RGBW) 
                          ? BYTES_PER_LED_RGBW : BYTES_PER_LED_RGB;
    
    for (uint16_t i = 0; i < _numLeds; i++) {
        uint8_t* pixel = &_pixelBuffer[i * bytesPerLed];
        uint8_t r, g, b, w = 0;
        
        // Extrair componentes do buffer (já está na ordem correta)
        switch (_config.colorOrder) {
            case ORDER_GRB:
                g = pixel[0]; r = pixel[1]; b = pixel[2];
                break;
            case ORDER_RGB:
                r = pixel[0]; g = pixel[1]; b = pixel[2];
                break;
            case ORDER_BRG:
                b = pixel[0]; r = pixel[1]; g = pixel[2];
                break;
            case ORDER_BGR:
                b = pixel[0]; g = pixel[1]; r = pixel[2];
                break;
            case ORDER_GRBW:
                g = pixel[0]; r = pixel[1]; b = pixel[2]; w = pixel[3];
                break;
            case ORDER_RGBW:
                r = pixel[0]; g = pixel[1]; b = pixel[2]; w = pixel[3];
                break;
            default:
                g = pixel[0]; r = pixel[1]; b = pixel[2];
        }
        
        // Aplicar brilho
        r = _applyBrightness(r);
        g = _applyBrightness(g);
        b = _applyBrightness(b);
        w = _applyBrightness(w);
        
        // Codificar na ordem do protocolo WS2812B (GRB)
        _encodeByte(g, dmaPtr); dmaPtr += 8;
        _encodeByte(r, dmaPtr); dmaPtr += 8;
        _encodeByte(b, dmaPtr); dmaPtr += 8;
        
        if (bytesPerLed == 4) {
            _encodeByte(w, dmaPtr); dmaPtr += 8;
        }
    }
    
    // Adicionar período de reset (valores 0)
    for (uint16_t i = 0; i < WS2812_RESET_CYCLES; i++) {
        *dmaPtr++ = 0;
    }
}

void LedController::_encodeByte(uint8_t byte, uint16_t* dest)
{
    // Codificar 8 bits, MSB primeiro
    for (int8_t bit = 7; bit >= 0; bit--) {
        if (byte & (1 << bit)) {
            *dest++ = WS2812_PWM_HIGH;  // Bit 1: duty cycle alto
        } else {
            *dest++ = WS2812_PWM_LOW;   // Bit 0: duty cycle baixo
        }
    }
}

uint8_t LedController::_applyBrightness(uint8_t value)
{
    if (_brightness == 255) return value;
    return (uint16_t)value * _brightness / 255;
}

// ==================== Métodos de Cor ====================

void LedController::setPixelColor(uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index >= _numLeds || !_pixelBuffer) return;
    
    uint8_t bytesPerLed = (_config.colorOrder == ORDER_GRBW || _config.colorOrder == ORDER_RGBW) 
                          ? BYTES_PER_LED_RGBW : BYTES_PER_LED_RGB;
    uint8_t* pixel = &_pixelBuffer[index * bytesPerLed];
    
    // Armazenar na ordem GRB (padrão WS2812B)
    switch (_config.colorOrder) {
        case ORDER_GRB:
        case ORDER_GRBW:
            pixel[0] = g; pixel[1] = r; pixel[2] = b;
            break;
        case ORDER_RGB:
        case ORDER_RGBW:
            pixel[0] = r; pixel[1] = g; pixel[2] = b;
            break;
        case ORDER_BRG:
            pixel[0] = b; pixel[1] = r; pixel[2] = g;
            break;
        case ORDER_BGR:
            pixel[0] = b; pixel[1] = g; pixel[2] = r;
            break;
        default:
            pixel[0] = g; pixel[1] = r; pixel[2] = b;
    }
}

void LedController::setPixelColor(uint16_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
    if (index >= _numLeds || !_pixelBuffer) return;
    
    setPixelColor(index, r, g, b);
    
    // Se for RGBW, adicionar componente W
    if (_config.colorOrder == ORDER_GRBW || _config.colorOrder == ORDER_RGBW) {
        uint8_t* pixel = &_pixelBuffer[index * BYTES_PER_LED_RGBW];
        pixel[3] = w;
    }
}

void LedController::setPixelColor(uint16_t index, Color color)
{
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    uint8_t w = (color >> 24) & 0xFF;
    
    setPixelColor(index, r, g, b, w);
}

Color LedController::getPixelColor(uint16_t index)
{
    if (index >= _numLeds || !_pixelBuffer) return 0;
    
    uint8_t bytesPerLed = (_config.colorOrder == ORDER_GRBW || _config.colorOrder == ORDER_RGBW) 
                          ? BYTES_PER_LED_RGBW : BYTES_PER_LED_RGB;
    uint8_t* pixel = &_pixelBuffer[index * bytesPerLed];
    
    uint8_t r, g, b, w = 0;
    
    switch (_config.colorOrder) {
        case ORDER_GRB:
            g = pixel[0]; r = pixel[1]; b = pixel[2];
            break;
        case ORDER_RGB:
            r = pixel[0]; g = pixel[1]; b = pixel[2];
            break;
        case ORDER_BRG:
            b = pixel[0]; r = pixel[1]; g = pixel[2];
            break;
        case ORDER_BGR:
            b = pixel[0]; g = pixel[1]; r = pixel[2];
            break;
        case ORDER_GRBW:
            g = pixel[0]; r = pixel[1]; b = pixel[2]; w = pixel[3];
            break;
        case ORDER_RGBW:
            r = pixel[0]; g = pixel[1]; b = pixel[2]; w = pixel[3];
            break;
        default:
            g = pixel[0]; r = pixel[1]; b = pixel[2];
    }
    
    return ((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

void LedController::fill(uint8_t r, uint8_t g, uint8_t b)
{
    for (uint16_t i = 0; i < _numLeds; i++) {
        setPixelColor(i, r, g, b);
    }
}

void LedController::fill(Color color)
{
    for (uint16_t i = 0; i < _numLeds; i++) {
        setPixelColor(i, color);
    }
}

void LedController::fillRange(uint16_t startIndex, uint16_t count, Color color)
{
    uint16_t end = startIndex + count;
    if (end > _numLeds) end = _numLeds;
    
    for (uint16_t i = startIndex; i < end; i++) {
        setPixelColor(i, color);
    }
}

void LedController::clear()
{
    if (_pixelBuffer) {
        memset(_pixelBuffer, 0, _pixelBufferSize);
    }
}

void LedController::setBrightness(uint8_t brightness)
{
    _brightness = brightness;
}

uint8_t LedController::getBrightness() const
{
    return _brightness;
}

// ==================== Métodos Estáticos de Cor ====================

Color LedController::Color_RGB(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

Color LedController::Color_RGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
    return ((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

Color LedController::ColorHSV(uint8_t h, uint8_t s, uint8_t v)
{
    uint8_t r, g, b;
    
    if (s == 0) {
        r = g = b = v;
    } else {
        uint8_t region = h / 43;
        uint8_t remainder = (h - (region * 43)) * 6;
        
        uint8_t p = (v * (255 - s)) >> 8;
        uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
        uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
        
        switch (region) {
            case 0:  r = v; g = t; b = p; break;
            case 1:  r = q; g = v; b = p; break;
            case 2:  r = p; g = v; b = t; break;
            case 3:  r = p; g = q; b = v; break;
            case 4:  r = t; g = p; b = v; break;
            default: r = v; g = p; b = q; break;
        }
    }
    
    return Color_RGB(r, g, b);
}

Color LedController::blend(Color color1, Color color2, uint8_t amount)
{
    uint8_t r1 = (color1 >> 16) & 0xFF;
    uint8_t g1 = (color1 >> 8) & 0xFF;
    uint8_t b1 = color1 & 0xFF;
    
    uint8_t r2 = (color2 >> 16) & 0xFF;
    uint8_t g2 = (color2 >> 8) & 0xFF;
    uint8_t b2 = color2 & 0xFF;
    
    uint8_t r = r1 + (((r2 - r1) * amount) >> 8);
    uint8_t g = g1 + (((g2 - g1) * amount) >> 8);
    uint8_t b = b1 + (((b2 - b1) * amount) >> 8);
    
    return Color_RGB(r, g, b);
}

// ==================== Efeitos ====================

void LedController::rainbow(uint8_t startHue)
{
    for (uint16_t i = 0; i < _numLeds; i++) {
        uint8_t hue = startHue + (i * 256 / _numLeds);
        setPixelColor(i, ColorHSV(hue, 255, 255));
    }
}

void LedController::rotate(int16_t positions)
{
    if (_numLeds < 2 || !_pixelBuffer) return;
    
    uint8_t bytesPerLed = (_config.colorOrder == ORDER_GRBW || _config.colorOrder == ORDER_RGBW) 
                          ? BYTES_PER_LED_RGBW : BYTES_PER_LED_RGB;
    
    // Normalizar posições
    positions = positions % (int16_t)_numLeds;
    if (positions < 0) positions += _numLeds;
    if (positions == 0) return;
    
    // Usar buffer temporário
    uint8_t* temp = (uint8_t*)malloc(_pixelBufferSize);
    if (!temp) return;
    
    // Copiar rotacionado
    for (uint16_t i = 0; i < _numLeds; i++) {
        uint16_t srcIndex = (i + positions) % _numLeds;
        memcpy(&temp[i * bytesPerLed], &_pixelBuffer[srcIndex * bytesPerLed], bytesPerLed);
    }
    
    // Copiar de volta
    memcpy(_pixelBuffer, temp, _pixelBufferSize);
    free(temp);
}

void LedController::shift(int16_t positions)
{
    if (_numLeds < 2 || !_pixelBuffer || positions == 0) return;
    
    uint8_t bytesPerLed = (_config.colorOrder == ORDER_GRBW || _config.colorOrder == ORDER_RGBW) 
                          ? BYTES_PER_LED_RGBW : BYTES_PER_LED_RGB;
    
    if (positions > 0) {
        // Shift para direita
        for (int16_t i = _numLeds - 1; i >= positions; i--) {
            memcpy(&_pixelBuffer[i * bytesPerLed], 
                   &_pixelBuffer[(i - positions) * bytesPerLed], 
                   bytesPerLed);
        }
        // Limpar LEDs iniciais
        memset(_pixelBuffer, 0, positions * bytesPerLed);
    } else {
        // Shift para esquerda
        positions = -positions;
        for (uint16_t i = 0; i < _numLeds - positions; i++) {
            memcpy(&_pixelBuffer[i * bytesPerLed], 
                   &_pixelBuffer[(i + positions) * bytesPerLed], 
                   bytesPerLed);
        }
        // Limpar LEDs finais
        memset(&_pixelBuffer[(_numLeds - positions) * bytesPerLed], 0, positions * bytesPerLed);
    }
}
