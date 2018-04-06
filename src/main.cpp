/* Game Controller */
#include <mbed.h>
#include <EthernetInterface.h>
#include <rtos.h>
#include <mbed_events.h>

#include <FXOS8700Q.h>
#include <C12832.h>

/* display */
C12832 lcd(D11, D13, D12, D7, D10);

/* event queue and thread support */
Thread dispatch;
EventQueue periodic;

/* Accelerometer */
I2C i2c(PTE25, PTE24);
FXOS8700QAccelerometer acc(i2c, FXOS8700CQ_SLAVE_ADDR1);

/* Input from Potentiometers */
AnalogIn  left(A0);

/* User input states */
/*TODO define variables to hold the users desired actions.
    (roll rate and throttle setting)
*/
/* Task for polling sensors */
void user_input(void){
    motion_data_units_t a;
    acc.getAxis(a);
    lcd.locate(0,0);
    float magnitude = sqrt(a.x*a.x+a.x*a.x+a.x*a.x);
    lcd.printf("axis x:%3.1f (%3.1f)\n",a.x, magnitude);
    a.x = a.x/magnitude;
    a.y = a.y/magnitude;
    a.y = a.y/magnitude;
    float angle = asin(a.x);
    /*TODO decide on what roll rate -1..+1 to ask for */

    /*TODO decide on what throttle setting 0..100 to ask for */
}
/* States from Lander */
/*TODO Variables to hold the state of the lander as returned to
    the MBED board, including altitude,fuel,isflying,iscrashed
*/
/*YOU will have to hardwire the IP address in here */
SocketAddress lander("192.168.1.165",65200);
SocketAddress dash("192.168.1.13",65250);
EthernetInterface eth;
UDPSocket udp;

/* Task for synchronous UDP communications with lander */
void communications(void){
    SocketAddress source;
    /*Create and format the message to send to the Lander */
    char buffer[512];
    sprintf(buffer, "command:!\n....");
    /* Send and receive messages*/
    udp.sendto( lander, buffer, strlen(buffer));
    nsapi_size_or_error_t  n =
                 udp.recvfrom(&source, buffer, sizeof(buffer));
    buffer[n] = '\0';
    /* Unpack incomming message */
    /*split message into lines*/
    /*for each line */
    char *nextline, *line;
    for(
      line = strtok_r(buffer, "\r\n", &nextline);
      line != NULL;
      line = strtok_r(NULL, "\r\n", &nextline)
    ){
      /* split into key value pairs */
    char *key, *value;
    for(
      key = strtok(line, ":");
      value = strtok(NULL, ":");
    )
     /*convert value strings into state variables */

      if( strcmp(key,"altitude")==0 ){
        /*do something with the 'altitude' value*/
      }
   }
}
/* Task for asynchronous UDP communications with dashboard */
void dashboard(void){
    /*TODO create and format a message to the Dashboard */
    /*TODO send the message to the dashboard:
        udp.sendto( dash, <message>, strlen(<message>));
    */
}

int main() {
    acc.enable();

    /* ethernet connection : usually takes a few seconds */
    printf("connecting \n");
    eth.connect();
    /* write obtained IP address to serial monitor */
    const char *ip = eth.get_ip_address();
    printf("IP address is: %s\n", ip ? ip : "No IP");

    /* open udp for communications on the ethernet */
    udp.open( &eth);

    printf("lander is on %s/%d\n",lander.get_ip_address(),lander.get_port() );
    printf("dash   is on %s/%d\n",dash.get_ip_address(),dash.get_port() );

    /* periodic tasks */
    /* call periodic tasks;
            communications, user_input, dashboard
            at desired rates.*/

    periodic.call_every(50, user_input);


    /* start event dispatching thread */
    dispatch.start( callback(&periodic, &EventQueue::dispatch_forever) );

    while(1) {
        /* update display at whatever rate is possible */
        /*TODO show user information on the LCD */
        /*TODO set LEDs as appropriate to show boolean states */

        wait(1);/*TODO you may want to change this time
                    to get a responsive display */
    }
}
