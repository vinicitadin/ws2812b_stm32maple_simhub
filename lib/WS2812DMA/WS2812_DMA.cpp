#include <Arduino.h>
#include <libmaple/dma.h>
#include <libmaple/timer.h>
#include "WS2812_DMA.h"

const uint16_t T0H = 30;   // ~0.4us
const uint16_t T1H = 60;   // ~0.8us
const uint16_t PERIOD = 89; // ~1.25us total

struct PinConfig {
    uint8_t pin;
    timer_dev* timer;
    uint8_t timerChannel;
    dma_channel dmaChannel;
    volatile uint32_t* ccrReg;
    bool isAdvancedTimer;
};

static timer_dev* selectedTimer = nullptr;
static uint8_t selectedTimerChannel = 0;
static dma_channel selectedDmaChannel = DMA_CH1;
static volatile uint32_t* selectedCCR = nullptr;
static bool isAdvancedTimer = false;

WS2812_DMA::WS2812_DMA(uint8_t pin, uint8_t count) {
    this->pin = pin;
    this->count = count;
    this->bytes = count * 24;

    bufferSize = count * 24 + 50;
    ws2812Buffer = new uint16_t[bufferSize];
    
    configurePinMapping(pin);
}

void WS2812_DMA::configurePinMapping(uint8_t pin) {
    switch(pin) {
        case PA8:  // TIM1_CH1
            selectedTimer = TIMER1;
            selectedTimerChannel = TIMER_CH1;
            selectedDmaChannel = DMA_CH5;  // TIM1_UP
            selectedCCR = &(TIMER1->regs.gen->CCR1);
            isAdvancedTimer = true;
            break;
            
        case PA7:  // TIM3_CH2
            selectedTimer = TIMER3;
            selectedTimerChannel = TIMER_CH2;
            selectedDmaChannel = DMA_CH3;  // TIM3_UP
            selectedCCR = &(TIMER3->regs.gen->CCR2);
            isAdvancedTimer = false;
            break;
            
        case PB5:  // TIM3_CH2 (remap)
            selectedTimer = TIMER3;
            selectedTimerChannel = TIMER_CH2;
            selectedDmaChannel = DMA_CH3;  // TIM3_UP
            selectedCCR = &(TIMER3->regs.gen->CCR2);
            isAdvancedTimer = false;
            break;
            
        case PA6:  // TIM3_CH1
            selectedTimer = TIMER3;
            selectedTimerChannel = TIMER_CH1;
            selectedDmaChannel = DMA_CH3;  // TIM3_UP
            selectedCCR = &(TIMER3->regs.gen->CCR1);
            isAdvancedTimer = false;
            break;
            
        case PB0:  // TIM3_CH3
            selectedTimer = TIMER3;
            selectedTimerChannel = TIMER_CH3;
            selectedDmaChannel = DMA_CH3;  // TIM3_UP
            selectedCCR = &(TIMER3->regs.gen->CCR3);
            isAdvancedTimer = false;
            break;
            
        default:
            selectedTimer = TIMER1;
            selectedTimerChannel = TIMER_CH1;
            selectedDmaChannel = DMA_CH5;
            selectedCCR = &(TIMER1->regs.gen->CCR1);
            isAdvancedTimer = true;
            break;
    }
}

void WS2812_DMA::setPixelColor(uint16_t led, uint8_t r, uint8_t g, uint8_t b) {
    uint16_t i = led * 24;

    uint32_t color = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;

    for (int bit = 23; bit >= 0; bit--) {
        ws2812Buffer[i++] = (color & (1UL << bit)) ? T1H : T0H;
    }
}

void WS2812_DMA::show() {
    dma_tube_reg_map *tube = dma_tube_regs(DMA1, selectedDmaChannel);

    tube->CCR = 0;
    
    dma_clear_isr_bits(DMA1, selectedDmaChannel);

    tube->CMAR = (uint32_t)ws2812Buffer;
    tube->CPAR = (uint32_t)selectedCCR;
    tube->CNDTR = bufferSize;

    tube->CCR = DMA_CCR_PL_HIGH | DMA_CCR_MSIZE_16BITS | DMA_CCR_PSIZE_16BITS |
                DMA_CCR_MINC | DMA_CCR_DIR_FROM_MEM | DMA_CCR_TCIE;

    selectedTimer->regs.gen->DIER |= TIMER_DIER_UDE;

    tube->CCR |= DMA_CCR_EN;

    selectedTimer->regs.gen->CR1 |= TIMER_CR1_CEN;

    while (!(dma_get_isr_bits(DMA1, selectedDmaChannel) & DMA_ISR_TCIF)) { }

    selectedTimer->regs.gen->CR1 &= ~TIMER_CR1_CEN;
    selectedTimer->regs.gen->DIER &= ~TIMER_DIER_UDE;
    tube->CCR = 0;
    dma_clear_isr_bits(DMA1, selectedDmaChannel);
}

void WS2812_DMA::begin() {
    if (pin == PB5) {
        afio_remap(AFIO_REMAP_TIM3_PARTIAL);
    }
    
    pinMode(pin, PWM);

    timer_init(selectedTimer);
    timer_pause(selectedTimer);

    timer_set_prescaler(selectedTimer, 0);
    timer_set_reload(selectedTimer, PERIOD);
    timer_set_compare(selectedTimer, selectedTimerChannel, 0);

    timer_set_mode(selectedTimer, selectedTimerChannel, TIMER_PWM);
    
    if (isAdvancedTimer) {
        selectedTimer->regs.adv->BDTR |= TIMER_BDTR_MOE;
    }

    dma_init(DMA1);

    for (uint16_t i = count * 24; i < bufferSize; i++) {
        ws2812Buffer[i] = 0;
    }
}
