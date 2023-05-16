# geiger_stationary
simple Geiger counter junkbox

Written by BingAI and fixed a bit by me. 

for geiger_oled2.ino :

-expects geiger counter pulses to come into D2 pin. 

-beeps using speaker or piezo using D3 pin once pulse arrives

-pulses are counted by interrupt on pin change 

-average of pulse count is updated each second

-longer averages are emitted in CSV format along with timestamp on serial out
 so larger display like a computer can use the data to display long term trend
 or determine doses absorbed in certain time frames
 it is not designed as long term measurement instrument, maybe i will fix it in the future. 

-analog knob on A0 pin determines update interval of internal OLED display, from 1 to 120 seconds. 

-has few options to customize display 

-graph auto-scales for minimum and maximum values present in the graph

TODO : millis timestamp overflow is not handled anyhow, it will overflow in 50 days of use. 

TODO : separate buffer to store averages for long term doses, like daily dose. 

TODO : more display options like alternate graph modes, display of min and max value etc. 

TODO : add 300V DC-DC based on PID and using PWM pin to drive DC-DC step up so the counter is totally standalone. 
