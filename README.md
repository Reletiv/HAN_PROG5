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
* OPDRACHT 1: Tried to find the material but i cant remember it being discussed in class, contacted Johan.
* OPDRACHT 2: In PLANTUML i wrote a basic use case diagram for my BME280 library code, file name usecasediagram.txt
* OPDRACHT 3: I wrote a library for the bosch BME280 sensor, it works with the Wire library to handle i2c commands.
Written the library based on my work from last year.
* OPDRACHT 4: I ported the Arduino library to the raspberry pi. Porting my own library was not hard, but porting the i2c library was harder.
* OPDRACHT 5: I added MQTT support with some help from the internet. i needed to compile a github mqtt c++ library for the raspberry pi because there is no support at first.
Used my own MQTT server for testing 
commands for running:
--> g++ main.cpp bme280.cpp i2c.cpp -o bme280_mqtt -lpaho-mqttpp3 -lpaho-mqtt3as -pthread
--> sudo ./bme280_mqtt
* OPDRACHT 6: Since i modulairly programmed the code in the last assignments i could easily Generate an PLANTUML class and sequence diagram semi-automatically. It uses the right functions and connections and shows great patterns
* Class diagram
<img width="584" height="828" alt="image" src="https://github.com/user-attachments/assets/7d083862-6d8a-408a-b08d-a18cb75b7bf0" />
* Sequence Diagram
<img width="742" height="962" alt="image" src="https://github.com/user-attachments/assets/1d82d298-4423-4147-bdf7-3fce9ebe1b74" />

* OPDRACHT 7:
* OPDRACHT 8:
