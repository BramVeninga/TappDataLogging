#include <bluefruit.h>

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Serial.println("Stress Mode activated");
}

void loop() 
{
  stressMode();
}

void stressMode() 
{
  float root, division, power, logValue;
  for (volatile int i = 1; i <= 100; i++) 
  {
    root = sqrt(i);              
    division = i / 3.14159265;   
    power = pow(i, 1.5);         
    logValue = log(i);           
  }

  Serial.print("Root: ");
  Serial.print(root);
  Serial.print(" | Division: ");
  Serial.print(division);
  Serial.print(" | Power: ");
  Serial.print(power);
  Serial.print(" | Log: ");
  Serial.println(logValue);
  Serial.flush();

  volatile int largeArray[1000];  
  for (int i = 0; i < 1000; i++) 
  {
    largeArray[i] += i + (i * i) + (i * 3) - (i / 2);  
  }
  for (int i = 1; i < 1000; i++) 
  {
    largeArray[i] += largeArray[i - 1] * 2; 
  }

  for(int currentNumber : largeArray)
  {
    Serial.println(String(currentNumber) + " ");
  }
}