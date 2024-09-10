/* Semaphores and task communication: queues + semaphores 

Author: Alberto Ferrante

Based on the mBed tutorials: 
https://os.mbed.com/docs/mbed-os/v6.16/apis/semaphore.html
https://os.mbed.com/docs/mbed-os/v6.16/apis/queue.html
https://os.mbed.com/docs/mbed-os/v6.16/apis/memorypool.html
*/

#include <mbed.h>
using namespace mbed;
using namespace rtos;
using namespace std::literals::chrono_literals;

#include <Arduino_HTS221.h>
#include <Arduino_APDS9960.h>

#define TIME_MULTIPLIER 10
#define TIME_T1 500
#define TIME_T2 TIME_T1 *TIME_MULTIPLIER

Thread t1;
Thread t2;
Thread t3;

Semaphore bus_i2c(1); //declares a semaphore to be used as guard for the I2C bus

typedef struct {
  int value;
} message_t;

Queue<message_t, 1> t1_data; //creates a queue for thread t1
Queue<message_t, TIME_MULTIPLIER> t2_data; //creates a queue for thread t2

MemoryPool<message_t, 1> mpool1; //memory areas associated to the queues
MemoryPool<message_t, TIME_MULTIPLIER> mpool2;

void f1() {
  int k = 0;
  message_t *message = mpool1.alloc();
  uint time = millis();
  uint timeupdated = time;
  uint w_time = 0;
  int r, g, b;
  bool coloravailable = false;

  while (1) {
    coloravailable = false;
    while (!coloravailable) {
      bus_i2c.acquire();
      coloravailable = APDS.colorAvailable();
      bus_i2c.release();
      ThisThread::sleep_for(10);
    }
  //I2C is a shared bus: access from multiple thread can create conflicts. We use a semaphore to guard the bus
    bus_i2c.acquire(); //checks if the bus is free (semaphore can be aqcuired) and acquires the bus (acquire semaphore); if not free, waits of it
    APDS.readColor(r, g, b);
    bus_i2c.release(); //releases the bus (i.e., releases the semaphore)
    message->value = r + g + b;
    t1_data.put(message); //puts a new element in the t1 queue
    timeupdated = millis();
    w_time = TIME_T1 - (timeupdated - time - w_time); //computes the waiting time: period-time consumed in the executed instructions in the current iteration of the loop 
    ThisThread::sleep_for(w_time);
    time = timeupdated;
  }
}

void f2() {
  message_t *message = mpool2.alloc();
  uint time = millis();
  uint timeupdated = time;
  uint w_time = 0;

  while (1) {
    bus_i2c.acquire();
    message->value = HTS.readTemperature();
    bus_i2c.release();
    t2_data.put(message);
    timeupdated = millis();
    w_time = TIME_T2 - (timeupdated - time - w_time);
    ThisThread::sleep_for(w_time);
    time = timeupdated;
  }
}

void print() {
  int number_t1 = 0;
  float avg_t1 = 0;
  int val_t2 = 0;
  bool t2_valid = false;
  osEvent evt1, evt2;
  message_t *message1;
  message_t *message2;

  while (1) {
    if (!t2_valid) {
      osEvent evt2 = t2_data.get(TIME_T2); //gets an element from the queue, if available
      if (evt2.status == osEventMessage) { //checks if a valid element was obtained
        message2 = (message_t *)evt2.value.p; //gets the contents of the message
        val_t2 = message2->value;
        mpool2.free(message2); //frees memory of the element pulled from the queue
        t2_valid = true;
      }
    }

    evt1 = t1_data.get(TIME_T1);
    while (evt1.status == osEventMessage && number_t1 < TIME_MULTIPLIER) {
      message1 = (message_t *)evt1.value.p;
      avg_t1 += message1->value;
      mpool1.free(message1);
      number_t1++;
      evt1 = t1_data.get(TIME_T1 / 2);
    }

    if (number_t1 >= TIME_MULTIPLIER && t2_valid) { //when all the expected elements are received from t1 and t2, print the results
      Serial.print(millis());
      Serial.print(": ");
      Serial.print(avg_t1 / number_t1);
      Serial.print(", ");
      Serial.println(val_t2);
      avg_t1 = 0;
      number_t1 = 0;
      t2_valid = false;
    }
  }
}

void setup() {
  Serial.begin(19200);

  if (!HTS.begin()) {
    Serial.println("Failed to initialize humidity temperature senfsor!");
    while (1)
      ;
  }

  if (!APDS.begin()) {
    Serial.println("Error initializing APDS9960 sensor!");
    while (1)
      ;
  }

  t1.start(f1); //start the three treads
  t2.start(f2);
  t3.start(print);
}

void loop() {
  // put your main code here, to run repeatedly:
}
