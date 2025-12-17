# HAN_PROG5

PROG5 and PROG5-LAB: Software Design with UML and C++

Lab assignments P1:
* 1 Make a basic stepper motor library and submit it to GitHub classroom
* 2 Create a UML Use Case diagram with description (see Markdown document) for a sensor application
* 3 Create a library for a sensor (bring your own or use BMP280) in Arduino in C++
* 4 Create a library for an i2c sensor or function on a Raspberry Pi in C++
* 5 Extend the Pi sensor Library to connect and publish to an MQTT broker in C++
* 6 Create a UML class diagram and sequence diagram for your sensor library
* 7 Perform Unit test on your Pi sensor library in C++
* 8 Check your sensor library on DRY, KISS, SOLID and Loose Coupling / Strong Cohesion principles, refactor your code and report which principles you improved on.

--------------------------------------------------------------------------
My own experiences of the assignments:
* **OPDRACHT 1:** Contacted Johan, Due to this being an introduction to git, this assignment was not needed.
* **OPDRACHT 2:** In PLANTUML i wrote a basic use case diagram for my BME280 library code, file name usecasediagram.txt
<img width="727" height="507" alt="image" src="https://github.com/user-attachments/assets/d4ed0c5b-8eff-4a3b-82c4-6b75d467b722" />

* **OPDRACHT 3:** I wrote a library for the bosch BME280 sensor, it works with the Wire library to handle i2c commands.
Written the library based on my work from last year.
* **OPDRACHT 4:** I ported the Arduino library to the raspberry pi. Porting my own library was not hard, but porting the i2c library was harder.
* **OPDRACHT 5:** I added MQTT support with some help from the internet. i needed to compile a github mqtt c++ library for the raspberry pi because there is no support at first.
Used my own MQTT server for testing 
commands for running:
--> g++ main.cpp bme280.cpp i2c.cpp -o bme280_mqtt -lpaho-mqttpp3 -lpaho-mqtt3as -pthread
--> sudo ./bme280_mqtt
* **OPDRACHT 6:** Since i modulairly programmed the code in the last assignments i could easily Generate an PLANTUML class and sequence diagram semi-automatically. It uses the right functions and connections and shows great patterns
* Class diagram
<img width="584" height="828" alt="image" src="https://github.com/user-attachments/assets/7d083862-6d8a-408a-b08d-a18cb75b7bf0" />

* Sequence Diagram
<img width="742" height="962" alt="image" src="https://github.com/user-attachments/assets/1d82d298-4423-4147-bdf7-3fce9ebe1b74" />

* **OPDRACHT 7:** I made an very basic unit test of the BME280 sensor, to confirm if the sensor behaviour actually works.
* Possible to apply in an eviromental chamber together with an calibrated meter to confirm the exact readings.

* Unit test of functions of the sensor
<img width="819" height="421" alt="image" src="https://github.com/user-attachments/assets/0c019841-af8c-40f7-b7af-444f06fc309e" />


* **OPDRACHT 8:
  After some research and analysing my code I found the following optimisations.**

* DRY: For each type of data collection function I check the value range and return the output using similar logic. This logic can be combined to reduce code duplication.
* KISS: There are many magic numbers coming from the datasheet calculations, such as register addresses and formula constants. These should be defined as named constants to improve readability.
* SOLID: Currently my BME280 library is also responsible for logging errors. This mixes sensor logic with logging responsibility.
* Loose Coupling / Strong Cohesion: Currently readPressure() and readHumidity() trigger readTemperature() internally to update t_fine. This creates an unwanted coupling between these functions.

 **In order to be compliant with my findings I will implement the following:**
  
* DRY: Improve the logic of returning the correct values to make the code more compact.
* KISS: Define magic numbers such as register addresses and formula constants.
* SOLID: Remove error logging from the library to better comply with the Single Responsibility Principle.
* Loose Coupling: Introduce an updateTFine() function so pressure and humidity no longer depend directly on readTemperature().

  
