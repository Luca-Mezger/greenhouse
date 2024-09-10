#include <mbed.h>
using namespace mbed;
using namespace rtos;
using namespace std::literals::chrono_literals;

Thread t1;
Thread t2;

void f1(){
  ThisThread::sleep_for(300ms);
}

void f2(){
  ThisThread::sleep_for(1000ms);
}


void setup() {
  // put your setup code here, to run once:
  t1.start(f1);
  t2.start(f2);
}

void loop() {
  // put your main code here, to run repeatedly:

}
