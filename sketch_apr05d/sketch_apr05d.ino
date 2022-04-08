
//B31DG Assignment 3
//Ethan Thomas Hunking H00272332
//The code below satisfies the same requirements of assignment 2 (implementation
//of 9 different tasks) in addition to an additional requirement 10. 
//The program uses FreeRTOS to implement each task at the different desired  
//frequency. It uses a queue, struct and a FreeRTOS semaphore
//The report attached to this code, explains this functionality. 

//************************************************************************************8
//Declaring pins, constants, structures, queues and semaphores

//Project Pins
#define timer_pin 32    //Pin allocation for Timer output (Task 2 Execution)
#define t1_pin 21       //Pin allocation for Task 1
#define t2_pin 22       //Pin allocation for Task 2
#define t3_pin 13       //Pin allocation for Task 3
#define t4_pin 14       //Pin allocation for Task 4
#define t8_pin 15       //Pin allocation for Task 8

//Frequency of Tasks (Hz) - Allows for modular code
#define t1_freq 30.67
#define t2_freq 24
#define t3_freq 1
#define t4_freq 24
#define t5_freq 24
#define t6_freq 10
#define t7_freq 3
#define t8_freq 3
#define t9_freq 0.2 

//Time Period Calculator - Allows for modular code
#define Tperiod 1000    //Converts s to ms within period calculation 

//Calculates period of task in ms using frequency 
double t1_period = Tperiod/t1_freq;
double t2_period = Tperiod/t2_freq;
double t3_period = Tperiod/t3_freq;
double t4_period = Tperiod/t4_freq;
double t5_period = Tperiod/t5_freq;
double t6_period = Tperiod/t6_freq;
double t7_period = Tperiod/t7_freq;
double t8_period = Tperiod/t8_freq;
double t9_period = Tperiod/t9_freq;

//Creates a structure for task 9 data 
struct task9_Data{
  int t2_switchstate;   //Recording Task 2 Switch Status
  int t3_frequency;     //Recording Task 3 Frequency
  int t5_potavg;        //Recording Task 5 Potentiometer Average
};

//Task 9 structure being initiated  
task9_Data t9_Data;

//Decleration of semaphore called mutex
static SemaphoreHandle_t mutex;

//Decleration of 3 Queues
static QueueHandle_t t4_queue;      //Queue for task 4->5
static const int t4_queue_len = 2;  //Sets max length of queue to 2
static QueueHandle_t t5_queue;      //Queue for task 5->7
static const int t5_queue_len = 1;  //Sets max length of queue to 1 
static QueueHandle_t t7_queue;      //Queue for task 7->8
static const int t7_queue_len = 1;  //Sets max length of queue to 1


//********************************************************************************************************************************
//Functions for each task


//Task1 watchdog 30Hz 
void task1(void *parameter){
  //Continuous loop
  while(1){
    digitalWrite(t1_pin, HIGH);   //Sets Output High
    delayMicroseconds(50);        //Delays signal by 50us   
    digitalWrite(t1_pin, LOW);    //Sets Output Low again

    //FreeRTOS delay at period of Task 1
    vTaskDelay(t1_period / portTICK_PERIOD_MS);
  }
}


//Task2 High/Low In 5Hz
void task2(void *parameter){ 
  //Declaring variables used within Task 2
  int t2_state = 0;       //Button State for Task 2
  int t2_debounce = 0;    //Button debounce

  //Continuous loop
  while(1){ 
    //Timer pin for T2 execution time
    digitalWrite(timer_pin, HIGH);   //High to measure time   

    t2_debounce = t2_state;         //Saves previous status              
    t2_state = digitalRead(t2_pin); //Reads state of button
    vTaskDelay(0.25 / portTICK_PERIOD_MS); //Delays signal by 250us for debouncing     
  
    //Checking for button bouncing
    if(t2_state != digitalRead(t2_pin)){
      if(t2_state != t2_debounce){
        t2_state = 0;
      }
    } 
    //Semaphore to protect access to T9 struct
    xSemaphoreTake(mutex, portMAX_DELAY);
    t9_Data.t2_switchstate = t2_state;    //Saves button state to struct
    xSemaphoreGive(mutex);

    //Timer pin for T2 execution time
    digitalWrite(timer_pin, LOW);    //Low to end measure time  

    //FreeRTOS delay at period of Task 2
    vTaskDelay(t2_period / portTICK_PERIOD_MS);
  }
}


//Task3 Freq In 1Hz
void task3(void *parameter){
  //Declaring variables used within Task 3
  float t3_duration1low;    //float counter for time of low
  float t3_durationperiod;  //float for period of waveform
  int t3_frequency;         //integer wavegen frequency

  //Continuous loop    
  while(1){
     t3_duration1low = pulseIn(t3_pin, LOW);    //Reads pin low time
     t3_durationperiod = t3_duration1low *2;    //Multiplies by 2 to get period
     t3_frequency = (1 / (t3_durationperiod/1000))*1000; //Converts period to freq.

     //Semaphore to protect access to T9 struct
     xSemaphoreTake(mutex, portMAX_DELAY);
     t9_Data.t3_frequency = t3_frequency;    //Saves freq. to struct 
     xSemaphoreGive(mutex);  

    //FreeRTOS delay at period of Task 3
     vTaskDelay(t3_period / portTICK_PERIOD_MS);
  }            
}


//Task4 Poteniotmeter 24Hz (des.) 25Hz (expt.)
void task4(void *parameter){
  //Declaring variables used within Task 5
  int t4_state;         //integer for analogue value read  
    
  //Continuous loop  
  while(1){
    //Reads analog input
    t4_state = analogRead(t4_pin);  

    //Sends pot. avg to queue for task 7
    xQueueSend(t4_queue, (void *)&t4_state, 1);
    
    //FreeRTOS delay at period of Task 4
    vTaskDelay(t4_period / portTICK_PERIOD_MS);
  }          
}


//Task5 Avg 4 Pot. 24Hz (des.) 25Hz (expt.)
void task5(void *parameter){ 
  //Declaring variables used within Task 5
  int t5_sto1 = 0;        //Task 5 Pre-Calc. Avg Storage 1
  int t5_sto2 = 0;        //Task 5 Pre-Calc. Avg Storage 2
  int t5_sto3 = 0;        //Task 5 Pre-Calc. Avg Storage 3
  int t5_sto4 = 0;        //Task 5 Pre-Calc. Avg Storage 4
  int t5_avg;             //Task 5 Pot. Avg.
  int t4_state_rec;           //integer for analogue value read

  //Continuous loop  
  while(1){
    //Checks if a message in the queue
    if (xQueueReceive(t4_queue, (void *)&t4_state_rec, 0) == pdTRUE){      
      t5_sto4 = t5_sto3;         //Shifts values by one position
      t5_sto3 = t5_sto2;         //Shifts values by one position         
      t5_sto2 = t5_sto1;         //Shifts values by one position 
      t5_sto1 = t4_state_rec;        //Shifts values by one position
    
      //Uses 4 values to calculate average
      t5_avg = (t5_sto4 + t5_sto3 + t5_sto2 + t5_sto1)/4; 
  
      //Semaphore to protect access to T9 struct
      xSemaphoreTake(mutex, portMAX_DELAY);
      t9_Data.t5_potavg = t5_avg;    //Saves pot. avg to struct
      xSemaphoreGive(mutex);
  
      //Sends pot. avg to queue for task 7
      xQueueSend(t5_queue, (void *)&t5_avg, 1);    
  
      //FreeRTOS delay at period of Task 5
      vTaskDelay(t5_period / portTICK_PERIOD_MS); 
    }
  }                     
}


//Task6 Volatile 10Hz
void task6(void *parameter){
  //Continuous loop
  while(1){
    //for loop as defined in lab sheet
    for(int C_Loop = 1; C_Loop == 1000; C_Loop++){  
      __asm__ __volatile__("nop");
    }

    //FreeRTOS delay at period of Task 6
    vTaskDelay(t6_period / portTICK_PERIOD_MS); 
  }
}


//Task7 checker 3Hz
void task7(void *parameter){
  //Declaring variables used within Task 7
  int t5_avg_rec;
  int error_code;

  //Continuous loop  
  while(1){
    //Checks if a message in the queue
    if (xQueueReceive(t5_queue, (void *)&t5_avg_rec, 0) == pdTRUE){      
      //if statment as defined in lab sheet
      if(t5_avg_rec > (4096/2)){
        error_code = 1; 
      }
      else{
        error_code = 0;
      }
      //Sends error code to queue for task 8
      xQueueSend(t7_queue, (void *)&error_code, 1);

      //FreeRTOS delay at period of Task 7
      vTaskDelay(t7_period / portTICK_PERIOD_MS); 
    }
  }
}


//Task8 LED 3Hz
void task8(void *parameter){
  //Declaring variables used within Task 8
  int error_code_rec;

  //Continuous loop  
  while(1){
    //Checks if a message in the queue
    if (xQueueReceive(t7_queue, (void *)&error_code_rec, 0) == pdTRUE){
      //Reads error code and sets LED high/low if error_code 1/0
      if(error_code_rec == 1){
        digitalWrite(t8_pin, HIGH);
      }
      else{
        digitalWrite(t8_pin, LOW);
      }
      //FreeRTOS delay at period of Task 8
      vTaskDelay(t8_period / portTICK_PERIOD_MS);
    }
  }
}


//Task9 Print Resuts
void task9(void *parameter){
  //Continuous loop  
  while(1){
    //Req.10 button pushed for output of code
    if(t9_Data.t2_switchstate){    
      //Semaphore to protect access to T9 struct
      xSemaphoreTake(mutex, portMAX_DELAY);
      
      //Prints in serial a csv as defined in lab sheet
      Serial.printf("%d, %d, %d \n",  t9_Data.t2_switchstate, t9_Data.t3_frequency, t9_Data.t5_potavg);
      xSemaphoreGive(mutex);
     }
     else{
   }
   //FreeRTOS delay at period of Task 9
   vTaskDelay(t9_period / portTICK_PERIOD_MS); 
  }
}

//Code below is used to measure stack size of tasks. Commented out as not used.
//With variables 
  //  UBaseType_t uxHighWaterMark;
  //  uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
//End of while loop
   // uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
   // Serial.println(uxHighWaterMark);

//************************************************************************************************************************
//Setup and Loop Functions 

void setup() {
  //Creates Serial Port
  Serial.begin(115200);
  
  // Configure pin
  pinMode(t1_pin, OUTPUT);            //Task 1 Watchdog 
  pinMode(t2_pin, INPUT_PULLDOWN);    //Task 2 Button dig. read
  pinMode(t3_pin, INPUT);             //Task 3 Square wave in.
  pinMode(t4_pin, INPUT);             //Task 4 Analogue input
  pinMode(t8_pin, OUTPUT);            //Task 8 Error LED output
  pinMode(timer_pin, OUTPUT);         //Output for time testing
  
  //Creates mutex for 
  mutex = xSemaphoreCreateMutex();

  //Creates 3 queues using prev. declared variables
  t4_queue = xQueueCreate(t4_queue_len, sizeof(int));   //Task 4 queue
  t5_queue = xQueueCreate(t5_queue_len, sizeof(int));   //Task 5 queue
  t7_queue = xQueueCreate(t7_queue_len, sizeof(int));   //Task 7 queue

  //Tasks below run forever
  xTaskCreate(  
              task1,        //Function to be called (Task 1)
              "task1",      //Task name 
              768,          //Stack size in bytes (ESP32)
              NULL,         //Parameter to pass to function
              1,            //Task priority (1 lowest)
              NULL          //Task handle
              );     

  xTaskCreate( 
              task2,        //Function to be called (Task 2)
              "task2",      //Task name
              768,          //Stack size in bytes (ESP32)
              NULL,         //Parameter to pass to function
              1,            //Task priority (1 lowest)
              NULL          //Task handle
              );                

  xTaskCreate( 
              task3,        //Function to be called (Task 3)
              "task3",      //Task name
              770,          //Stack size in bytes (ESP32)
              NULL,         //Parameter to pass to function
              2,            //Task priority (2 - highest in this program)
              NULL          //Task handle
              );  
                
  xTaskCreate(  
              task4,        //Function to be called (Task 4)
              "task4",      //Task name
              779,          //Stack size in bytes (ESP32)
              NULL,         //Parameter to pass to function
              1,            //Task priority (1 lowest)
              NULL          //Task handle
              );              
          
  xTaskCreate(  
              task5,        //Function to be called (Task 5)
              "task5",      //Task name
              768,          //Stack size in bytes (ESP32)
              NULL,         //Parameter to pass to function
              1,            //Task priority (1 lowest)
              NULL          //Task handle
              );              

  xTaskCreate(  
              task6,        //Function to be called (Task 6)
              "task6",      //Task name
              768,          //Stack size in bytes (ESP32)
              NULL,         //Parameter to pass to function
              1,            //Task priority (1 lowest)
              NULL          //Task handle
              );               
          
  xTaskCreate(  
              task7,        //Function to be called (Task 7)
              "task7",      //Task name
              780,          //Stack size in bytes (ESP32)
              NULL,         //Parameter to pass to function
              1,            //Task priority (1 lowest)
              NULL          //Task handle
              );       

  xTaskCreate( 
              task8,        //Function to be called (Task 8)
              "task8",      //Task name
              820,          //Stack size in bytes (ESP32)
              NULL,         //Parameter to pass to function
              1,            //Task priority (1 lowest)
              NULL          //Task handle
              );                   

  xTaskCreate( 
              task9,        //Function to be called (Task 9)
              "task9",      //Task name
              820,         //Stack size in bytes (ESP32)
              NULL,         //Parameter to pass to function
              1,            //Task priority (1 lowest)
              NULL          //Task handle
              );  

  //Deletes "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {
  // No Tasks within this loop
}