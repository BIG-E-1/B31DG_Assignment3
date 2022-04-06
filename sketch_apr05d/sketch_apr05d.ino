
//B31DG Assignment 3
//Ethan Thomas Hunking H00272332

#define timer_pin 32    //Pin allocation for timer output

#define t1_pin 21       //Pin allocation for Task 1

#define t2_pin 22       //Pin allocation for Task 2
int t2_state = 0;       //Button State for Task 2
int t2_debounce = 0;    //Button debounce

#define t3_pin 13       //Pin allocation for Task 3
float t3_duration1low;  //float counter for time of low
float t3_durationperiod;//float for period of waveform
int t3_frequency;       //integer wavegen frequency

#define t4_pin 14       //Pin allocation for Task 4
int t4_state = 0;       //integer for analogue value read

int t5_sto1 = 0;        //Task 5 Pre-Calc. Avg Storage 1
int t5_sto2 = 0;        //Task 5 Pre-Calc. Avg Storage 2
int t5_sto3 = 0;        //Task 5 Pre-Calc. Avg Storage 3
int t5_sto4 = 0;        //Task 5 Pre-Calc. Avg Storage 4
int t5_avg = 0;         //Task 5 Calculated Average

int error_code = 0;     //Task 7 Error Code

#define t8_pin 15       //Pin allocation for Task 8



//Functions for each task
//Task1 watchdog 30Hz 
void task1(void *parameter){
  while(1){
    vTaskDelay(33 / portTICK_PERIOD_MS);
    digitalWrite(t1_pin, HIGH);  //Sets Output High
    vTaskDelay(0.05 / portTICK_PERIOD_MS); //Delays signal by 50us   
    digitalWrite(t1_pin, LOW);//Sets Output Low again
  }
}


//Task2 High/Low In 5Hz
void task2(void *parameter){ 
  while(1){ 
    vTaskDelay(200 / portTICK_PERIOD_MS);
    t2_debounce = t2_state;         //Saves previous status              
    t2_state = digitalRead(t2_pin); //Reads state of button
    vTaskDelay(0.25 / portTICK_PERIOD_MS); //Delays signal by 250us       
  
    //Checking for button bouncing
    if(t2_state != digitalRead(t2_pin)){
      if(t2_state != t2_debounce){
        t2_state = 0;
      }
    } 
     //Serial.println(t2_state); 
  }
}


//Task3 Freq In 1Hz
void task3(void *parameter){  
  while(1){
     vTaskDelay(1000 / portTICK_PERIOD_MS);
     t3_duration1low = pulseIn(t3_pin, LOW);
     t3_durationperiod = t3_duration1low *2;
     t3_frequency = (1 / (t3_durationperiod/1000))*1000;       
  }            
}

//Task4 Poteniotmeter 24Hz (des.) 25Hz (expt.)
void task4(void *parameter){
  while(1){
    vTaskDelay(42 / portTICK_PERIOD_MS);
    //digitalWrite(timer_pin, HIGH);   //High to measure time   
    t4_state = analogRead(t4_pin);//Reads analog input  
    //digitalWrite(timer_pin, LOW);    //Low to end measure time  
  }          
}


//Task5 Avg 4 Pot. 24Hz (des.) 25Hz (expt.)
void task5(void *parameter){ 
  while(1){
    vTaskDelay(42 / portTICK_PERIOD_MS); 
    t5_sto4 = t5_sto3;         //Shifts values by one position
    t5_sto3 = t5_sto2;         //Shifts values by one position         
    t5_sto2 = t5_sto1;         //Shifts values by one position 
    t5_sto1 = t4_state;        //Shifts values by one position
  
    //Uses 4 values to calculate average
    t5_avg = (t5_sto4 + t5_sto3 + t5_sto2 + t5_sto1)/4; 
   // Serial.println(t5_avg);
  }                     
}


//Task6 Volatile 10Hz
void task6(void *parameter){
  while(1){
    vTaskDelay(100 / portTICK_PERIOD_MS); 
    //for loop as defined in lab sheet
    for(int C_Loop = 1; C_Loop == 1000; C_Loop++){  
      __asm__ __volatile__("nop");
    }
  }
}

//Task7 checker 3Hz
void task7(void *parameter){
  while(1){
    vTaskDelay(333 / portTICK_PERIOD_MS); 
    //if statment as defined in lab sheet
    if(t5_avg > (4096/2)){
      error_code = 1; 
    }
    else{
      error_code = 0;
    }
  }
}


//Task8 LED 3Hz
void task8(void *parameter){
  while(1){
    vTaskDelay(333 / portTICK_PERIOD_MS); 
    //Reads error code and sets LED high/low if error_code 1/0
    if(error_code == 1){
      digitalWrite(t8_pin, HIGH);
    }
    else{
      digitalWrite(t8_pin, LOW);
      }
  }
}

//Task9 Print Resuts
void task9(void *parameter){
  while(1){
    vTaskDelay(5000 / portTICK_PERIOD_MS); 
    if(t2_state == 1){
      //Prints in serial a csv as defined in lab sheet
      Serial.println("");
      Serial.print(t2_state); //Prints task 2
      Serial.print(" , ");
      Serial.print(t3_frequency); //Prints task 3
      Serial.print(" , ");
      Serial.print(t5_avg); //Prints task 5
    }
    else{
    }
  }
}

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
}

void loop() {
  // No Tasks within this loop
}