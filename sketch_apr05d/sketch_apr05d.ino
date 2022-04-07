
//B31DG Assignment 3
//Ethan Thomas Hunking H00272332

//Project Pins
#define timer_pin 32    //Pin allocation for timer output
#define t1_pin 21       //Pin allocation for Task 1
#define t2_pin 22       //Pin allocation for Task 2
#define t3_pin 13       //Pin allocation for Task 3
#define t4_pin 14       //Pin allocation for Task 4
#define t8_pin 15       //Pin allocation for Task 8

//Frequency of Tasks
#define t1_freq 30.67
#define t2_freq 5
#define t3_freq 1
#define t4_freq 24
#define t5_freq 24
#define t6_freq 10
#define t7_freq 3
#define t8_freq 3
#define t9_freq 0.2

//Time Period Calculator
#define Tperiod 1000    //Converts s to ms within period calc

//Period of Task in ms
double t1_period = Tperiod/t1_freq;
double t2_period = Tperiod/t2_freq;
double t3_period = Tperiod/t3_freq;
double t4_period = Tperiod/t4_freq;
double t5_period = Tperiod/t5_freq;
double t6_period = Tperiod/t6_freq;
double t7_period = Tperiod/t7_freq;
double t8_period = Tperiod/t8_freq;
double t9_period = Tperiod/t9_freq;

//Creating structure for task 9 information 
struct task9_Data{
  int t2_switchstate;
  int t3_frequency;
  int t5_potavg;
};

//Structure being intiated  
task9_Data t9_Data;

//Delcleation of semaphore called mutex.
static SemaphoreHandle_t mutex;

//Queues
static QueueHandle_t t4_queue;
static QueueHandle_t t5_queue;
static const int t5_queue_len = 2; 
static QueueHandle_t t7_queue;
static const int t7_queue_len = 2; 


//tbm
 int t4_state = 0;       //integer for analogue value read

//********************************************************************************************************************************
//Functions for each task
//Task1 watchdog 30Hz 
void task1(void *parameter){
  while(1){
    vTaskDelay(t1_period / portTICK_PERIOD_MS);
    digitalWrite(t1_pin, HIGH);  //Sets Output High
    vTaskDelay(0.05 / portTICK_PERIOD_MS); //Delays signal by 50us   
    digitalWrite(t1_pin, LOW);//Sets Output Low again
  }
}


//Task2 High/Low In 5Hz
void task2(void *parameter){ 

  int t2_state = 0;       //Button State for Task 2
  int t2_debounce = 0;    //Button debounce
  
  while(1){ 
    vTaskDelay(t2_period / portTICK_PERIOD_MS);
    t2_debounce = t2_state;         //Saves previous status              
    t2_state = digitalRead(t2_pin); //Reads state of button
    vTaskDelay(0.25 / portTICK_PERIOD_MS); //Delays signal by 250us       
  
    //Checking for button bouncing
    if(t2_state != digitalRead(t2_pin)){
      if(t2_state != t2_debounce){
        t2_state = 0;
      }
    } 
 
     xSemaphoreTake(mutex, portMAX_DELAY);
     t9_Data.t2_switchstate = t2_state;
     xSemaphoreGive(mutex);
  }
}


//Task3 Freq In 1Hz
void task3(void *parameter){
  
  float t3_duration1low;  //float counter for time of low
  float t3_durationperiod;//float for period of waveform
  int t3_frequency;       //integer wavegen frequency
    
  while(1){
     vTaskDelay(t3_period / portTICK_PERIOD_MS);
     t3_duration1low = pulseIn(t3_pin, LOW);
     t3_durationperiod = t3_duration1low *2;
     t3_frequency = (1 / (t3_durationperiod/1000))*1000; 

     xSemaphoreTake(mutex, portMAX_DELAY);
     t9_Data.t3_frequency = t3_frequency; 
     xSemaphoreGive(mutex);    
  }            
}

//Task4 Poteniotmeter 24Hz (des.) 25Hz (expt.)
void task4(void *parameter){

 
  
  while(1){
    vTaskDelay(t4_period / portTICK_PERIOD_MS);
    //digitalWrite(timer_pin, HIGH);   //High to measure time   
    t4_state = analogRead(t4_pin);//Reads analog input  
    //digitalWrite(timer_pin, LOW);    //Low to end measure time  
  }          
}


//Task5 Avg 4 Pot. 24Hz (des.) 25Hz (expt.)
void task5(void *parameter){ 

  int t5_sto1 = 0;        //Task 5 Pre-Calc. Avg Storage 1
  int t5_sto2 = 0;        //Task 5 Pre-Calc. Avg Storage 2
  int t5_sto3 = 0;        //Task 5 Pre-Calc. Avg Storage 3
  int t5_sto4 = 0;        //Task 5 Pre-Calc. Avg Storage 4
  int t5_avg;
  
  while(1){
    vTaskDelay(t5_period / portTICK_PERIOD_MS); 

    t5_sto4 = t5_sto3;         //Shifts values by one position
    t5_sto3 = t5_sto2;         //Shifts values by one position         
    t5_sto2 = t5_sto1;         //Shifts values by one position 
    t5_sto1 = t4_state;        //Shifts values by one position
  
    //Uses 4 values to calculate average
    t5_avg = (t5_sto4 + t5_sto3 + t5_sto2 + t5_sto1)/4; 
   // Serial.println(t5_avg);

   xSemaphoreTake(mutex, portMAX_DELAY);
   t9_Data.t5_potavg = t5_avg;
   xSemaphoreGive(mutex);

   xQueueSend(t5_queue, (void *)&t5_avg, 1);
  }                     
}


//Task6 Volatile 10Hz
void task6(void *parameter){
  while(1){
    vTaskDelay(t6_period / portTICK_PERIOD_MS); 
    //for loop as defined in lab sheet
    for(int C_Loop = 1; C_Loop == 1000; C_Loop++){  
      __asm__ __volatile__("nop");
    }
  }
}

//Task7 checker 3Hz
void task7(void *parameter){

  int t5_avg_rec;
  int error_code;
  
  while(1){
    if (xQueueReceive(t5_queue, (void *)&t5_avg_rec, 0) == pdTRUE){
      vTaskDelay(t7_period / portTICK_PERIOD_MS); 
      //if statment as defined in lab sheet
      if(t5_avg_rec > (4096/2)){
        error_code = 1; 
      }
      else{
        error_code = 0;
      }

      xQueueSend(t7_queue, (void *)&error_code, 1);
    }
  }
}


//Task8 LED 3Hz
void task8(void *parameter){

  int error_code_rec;
  
  while(1){
    if (xQueueReceive(t7_queue, (void *)&error_code_rec, 0) == pdTRUE){
      vTaskDelay(t8_period / portTICK_PERIOD_MS); 
      //Reads error code and sets LED high/low if error_code 1/0
      if(error_code_rec == 1){
        digitalWrite(t8_pin, HIGH);
      }
      else{
        digitalWrite(t8_pin, LOW);
      }
    }
  }
}

//Task9 Print Resuts
void task9(void *parameter){
  while(1){
    vTaskDelay(t9_period / portTICK_PERIOD_MS); 
   if(t9_Data.t2_switchstate){
      xSemaphoreTake(mutex, portMAX_DELAY);
      //Prints in serial a csv as defined in lab sheet
      Serial.printf("%d, %d, %d \n",  t9_Data.t2_switchstate, t9_Data.t3_frequency, t9_Data.t5_potavg);
      xSemaphoreGive(mutex);
    }
    
    else{
    }
  }
}


//************************************************************************************************************************
void setup() {

  // Configure pin
  pinMode(t1_pin, OUTPUT);   //Task 1 Watchdog 
  pinMode(t2_pin, INPUT_PULLDOWN);    //Task 2 Button dig. read
  pinMode(t3_pin, INPUT);    //Task 3 Square wave in.
  pinMode(t4_pin, INPUT);    //Task 4 Analogue input
  pinMode(t8_pin, OUTPUT);   //Task 8 Error LED output
  pinMode(timer_pin, OUTPUT);//Output for time testing
  
  //Creates Serial Port
  Serial.begin(115200);

  //Create mutex
  mutex = xSemaphoreCreateMutex();

  // Create queues
  t5_queue = xQueueCreate(t5_queue_len, sizeof(int));
  t7_queue = xQueueCreate(t7_queue_len, sizeof(int));

  //These tasks are set to run forever
  xTaskCreate(  
              task1,  // Function to be called
              "task1",   // Name of task
              1024,         // Stack size (bytes in ESP32, words in FreeRTOS)
              NULL,         // Parameter to pass to function
              1,            // Task priority (0 to configMAX_PRIORITIES - 1)
              NULL         // Task handle
              );     

  xTaskCreate( 
              task2,  // Function to be called
              "task2",   // Name of task
              1024,         // Stack size (bytes in ESP32, words in FreeRTOS)
              NULL,         // Parameter to pass to function
              1,            // Task priority (0 to configMAX_PRIORITIES - 1)
              NULL         // Task handle
              );                
                
  xTaskCreate(  
              task4,  // Function to be called
              "task4",   // Name of task
              1024,         // Stack size (bytes in ESP32, words in FreeRTOS)
              NULL,         // Parameter to pass to function
              1,            // Task priority (0 to configMAX_PRIORITIES - 1)
              NULL         // Task handle
              );     // Run on one core for demo purposes (ESP32 only)   
          
  xTaskCreate(  
              task5,  // Function to be called
              "task5",   // Name of task
              1024,         // Stack size (bytes in ESP32, words in FreeRTOS)
              NULL,         // Parameter to pass to function
              1,            // Task priority (0 to configMAX_PRIORITIES - 1)
              NULL         // Task handle
              );     // Run on one core for demo purposes (ESP32 only)   

  xTaskCreate(  
              task6,  // Function to be called
              "task6",   // Name of task
              1024,         // Stack size (bytes in ESP32, words in FreeRTOS)
              NULL,         // Parameter to pass to function
              1,            // Task priority (0 to configMAX_PRIORITIES - 1)
              NULL         // Task handle
              );     // Run on one core for demo purposes (ESP32 only)   
          
  xTaskCreate(  
              task7,  // Function to be called
              "task7",   // Name of task
              1024,         // Stack size (bytes in ESP32, words in FreeRTOS)
              NULL,         // Parameter to pass to function
              1,            // Task priority (0 to configMAX_PRIORITIES - 1)
              NULL         // Task handle
              );     // Run on one core for demo purposes (ESP32 only)  

  xTaskCreate( 
              task8,  // Function to be called
              "task8",   // Name of task
              1024,         // Stack size (bytes in ESP32, words in FreeRTOS)
              NULL,         // Parameter to pass to function
              1,            // Task priority (0 to configMAX_PRIORITIES - 1)
              NULL         // Task handle
              );                   

  xTaskCreate( 
              task9,  // Function to be called
              "task9",   // Name of task
              1024,         // Stack size (bytes in ESP32, words in FreeRTOS)
              NULL,         // Parameter to pass to function
              1,            // Task priority (0 to configMAX_PRIORITIES - 1)
              NULL         // Task handle
              );  

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {
  // No Tasks within this loop
}