#ifndef WS2813FAST_H
#define WS2813FAST_H
#include "resouce/teenst32_FastReg.h"

#define NSEC(x) (x)
#define USEC(x) (NSEC(1000 * x))
#define MSEC(x) (USEC(1000 * x))
#define SEC(x) (MSEC(1000 * x))
#define STOCLOCK(x) (1.0 * x / 13.8 + 0.5 - 3)

constexpr uint32_t Ws2813fast_t0h = STOCLOCK(NSEC(300)); //600
constexpr uint32_t Ws2813fast_t0l = STOCLOCK(NSEC(400));
constexpr uint32_t Ws2813fast_t1h = STOCLOCK(NSEC(750)); //1050
constexpr uint32_t Ws2813fast_t1l = STOCLOCK(NSEC(400));
constexpr uint32_t Ws2813fast_trst = STOCLOCK(USEC(250));

#undef NSEC
#undef USEC
#undef MSEC
#undef SEC
#undef STOCLOCK

template<std::size_t NUMPIXEL, uint8_t DATAPIN>
class Ws2813fast{
public:
    Ws2813fast()
    : dataTrans{ &Ws2813fast<NUMPIXEL,DATAPIN>::dataZero
                ,&Ws2813fast<NUMPIXEL,DATAPIN>::dataOne
               }
    , dataTransLast{ &Ws2813fast<NUMPIXEL,DATAPIN>::dataZeroLast
                    ,&Ws2813fast<NUMPIXEL,DATAPIN>::dataOneLast
               }
    {}

    void init(){
        t32fr_FastRegs fastReg;
        dataSetReg = fastReg.getRagPack(DATAPIN).cppSet;
        dataClerReg = fastReg.getRagPack(DATAPIN).cppClear;
        dataMask = fastReg.getRagPack(DATAPIN).bMask;
        t32fr_util::changeModeDigitalWrite(DATAPIN,false); // OPENDRAIN
        dataTrans[0] = &Ws2813fast<NUMPIXEL,DATAPIN>::dataZero;
        dataTrans[1] = &Ws2813fast<NUMPIXEL,DATAPIN>::dataOne;
    }

    volatile void show(){
        disableInterapt();
        for(pixelCounter = 0 ; pixelCounter < NUMPIXEL ; ++pixelCounter){
            indexCounter = pixelCounter * 3;
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 1].bit7])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 1].bit6])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 1].bit5])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 1].bit4])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 1].bit3])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 1].bit2])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 1].bit1])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 1].bit0])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 0].bit7])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 0].bit6])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 0].bit5])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 0].bit4])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 0].bit3])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 0].bit2])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 0].bit1])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 0].bit0])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 2].bit7])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 2].bit6])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 2].bit5])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 2].bit4])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 2].bit3])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 2].bit2])();
            (this->*dataTrans[colorBuffer.colorArray[indexCounter + 2].bit1])();
            (this->*dataTransLast[colorBuffer.colorArray[indexCounter + 2].bit0])();
        }
        dataReset();
        enableInterapt();
    }

    uint8_t& operator [](const std::size_t index){
        return this -> colorBuffer.colorArray[index].raw;
    }

private:
    volatile void dataZero(){
        dataHigh();
        waitClock((Ws2813fast_t0h >> 2) + 2);
        dataLow();
        waitClock((Ws2813fast_t0l >> 2) - 2);
    }

    volatile void dataOne(){
        dataHigh();
        waitClock((Ws2813fast_t1h >> 2) + 2);
        dataLow();
        waitClock((Ws2813fast_t1l >> 2) - 2);
    }

    volatile void dataZeroLast(){
        dataHigh();
        waitClock((Ws2813fast_t0h >> 2) + 2);
        dataLow();
    }

    volatile void dataOneLast(){
        dataHigh();
        waitClock((Ws2813fast_t1h >> 2) + 2);
        dataLow();
    }

    volatile void dataReset(){
        waitClock(Ws2813fast_trst >> 1);
    }


    volatile inline void waitClock(uint32_t clockNum){
        asm volatile(
            "L_%=_waitClock:subs %0, #1"   "\n\t"
            "bne L_%=_waitClock"           "\n\t"
            : "+r" (clockNum) :
        );
    }

    struct ColorBuffer{
        ColorBuffer(){}
        union BitSet{
            struct{
                uint8_t bit0:1;
                uint8_t bit1:1;
                uint8_t bit2:1;
                uint8_t bit3:1;
                uint8_t bit4:1;
                uint8_t bit5:1;
                uint8_t bit6:1;
                uint8_t bit7:1;
            };
            uint8_t raw;
        };
        BitSet colorArray[NUMPIXEL * 3];
    };
public:
    ColorBuffer colorBuffer;

private:

    volatile inline void dataHigh(){
        *dataSetReg = dataMask;
    }
    volatile inline void dataLow(){
        *dataClerReg = dataMask;
    }

    inline void disableInterapt(){
        asm volatile("CPSID i":::"memory");
    }

    inline void enableInterapt(){
        asm volatile("CPSIE i":::"memory");
    }

    uint32_t pixelCounter;
    uint32_t indexCounter;
    volatile uint32_t* dataSetReg;
    volatile uint32_t* dataClerReg;
    uint32_t dataMask;
    volatile void (Ws2813fast<NUMPIXEL,DATAPIN>::*dataTrans[2])();
    volatile void (Ws2813fast<NUMPIXEL,DATAPIN>::*dataTransLast[2])();

};
#endif /* end of include guard: WS2813FAST_H */
