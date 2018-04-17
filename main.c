#include <stdio.h>
#include <time.h>
#include "control.c"

#define CLOCK_RATE 1000000 //@Justen this is the clock rate of the processor I was testing it on, but it will probably need changed
#define TICKS_PER_SECOND 100


main() {
    printf("starting main\n");
    clock_t t;
    control_init();
    control_enable();
    while(1){
        t = clock();
        if(t%(CLOCK_RATE/TICKS_PER_SECOND) == 0){
            controlTick();
        }
    }
}

    //some test code I was using....
    // clock_t t;
    // int counter = 0;
    // while(1){
    //     t = clock();
    //     if(t%10 == 0){
    //         counter++;
    //         printf("counter is %d\n",counter);
    //     }else if(counter ==10){
    //         double seconds = t/1000000.0;
    //         printf("time is %f\n",seconds);
    //         break;
    //     }
    // }
