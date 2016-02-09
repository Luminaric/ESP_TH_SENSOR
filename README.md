This is a simple Temperature and Humidity sensor based on the ESP8266 ESP12 chip. It really could be done with any of the ESPs it's just easier to connect the RST pin to GPIO16 on the 12. 
In this sketch I send data to a Raspberry PI running an Apache server which records the values to a MySQL database. The sketch also uses io.adafruit.com so I can see the results from anywhere.
