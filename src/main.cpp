/* Game Controller */
#include <mbed.h>
#include <EthernetInterface.h>
#include <rtos.h>
#include <mbed_events.h>
#include <udp.h>
#include <FXOS8700Q.h>
#include <C12832.h>

/* display */
C12832 lcd(D11, D13, D12, D7, D10);

/* event queue and thread support */
Thread dispatch;
EventQueue periodic;

/*LED outputs*/
DigitalOut red(LED_RED);
DigitalOut green(LED_GREEN);
void greenOn(void) { green.write(0); }
void greenOff(void) { green.write(1); }
void redOn(void) { red.write(0); }
void redOff(void) { red.write(1); }

/* Input from Potentiometers */
AnalogIn  left(A0);
AnalogIn right(A1);

/* User input states */
float throttle;
float roll;

/* YOU will have to hardwire the IP address in here */
SocketAddress lander("192.168.1.217",65200);
SocketAddress dash("192.168.1.217",65250);

EthernetInterface eth;
UDPSocket udp;

/* States from Lander */
char altitude [50];
char fuel [50];
char flying [50];
char crashed [50];
char orientation [50];
char velocityX [50];
char velocityY [50];

int send(char *m, size_t s, SocketAddress dest)
{
  nsapi_size_or_error_t r = udp.sendto(dest, m, s);
  return r;
}
int receive(char *m, size_t s, SocketAddress reply)
{
  nsapi_size_or_error_t r = udp.recvfrom(&reply, m, s);
  return r;
}

/* Task for synchronous UDP communications with lander */
void communications(void){
    SocketAddress source;

    char throttleString [50];
    itoa (throttle, throttleString, 10);

    char rollString[50];
    if(roll > 0)
    {
      sprintf(rollString, "+%.1lf" , roll);
    }
    else
    {
      sprintf(rollString, "%.1lf" , roll);
    }

    char buffer [512];
    sprintf(buffer, "command:!\nthrottle:%s\nroll:%s\n", throttleString, rollString);
    udp.sendto( lander, buffer, strlen(buffer));
    printf("Message sent to Lander...\n");
    printf("Waiting for response...\n");
    nsapi_size_or_error_t n = udp.recvfrom(&source, buffer,sizeof(buffer));
    printf("Message received!\n");
    buffer[n] = '\0';
    printf("%s\n", buffer);

    char line1 [50];
    char line2 [50];
    char line3 [50];
    char line4 [50];
    char line5 [50];
    char line6 [50];
    char line7 [50];
    char line8 [50];

    char *nextLine;

    nextLine = strtok(buffer, "\n");
    strcpy(line1, nextLine);
    printf("%s\n", line1);

    nextLine = strtok(NULL, "\n");
    strcpy(line2, nextLine);

    printf("%s\n", line2);

    nextLine = strtok(NULL, "\n");
    strcpy(line3, nextLine);
    printf("%s\n", line3);

    nextLine = strtok(NULL, "\n");
    strcpy(line4, nextLine);
    printf("%s\n", line4);

    nextLine = strtok(NULL, "\n");
    strcpy(line5, nextLine);
    printf("%s\n", line5);

    nextLine = strtok(NULL, "\n");
    strcpy(line6, nextLine);
    printf("%s\n", line6);

    nextLine = strtok(NULL, "\n");
    strcpy(line7, nextLine);
    printf("%s\n", line7);

    nextLine = strtok(NULL, "\n");
    strcpy(line8, nextLine);
    printf("%s\n", line8);

    buffer[0] = '\0';

    nextLine = strtok(line2, ":");
    nextLine = strtok(NULL, "\n");
    strcpy(altitude, nextLine);

    nextLine = strtok(line3, ":");
    nextLine = strtok(NULL, "\n");
    strcpy(fuel, nextLine);

    nextLine = strtok(line4, ":");
    nextLine = strtok(NULL, "\n");
    strcpy(flying, nextLine);

    nextLine = strtok(line5, ":");
    nextLine = strtok(NULL, "\n");
    strcpy(crashed, nextLine);

    nextLine = strtok(line6, ":");
    nextLine = strtok(NULL, "\n");
    strcpy(orientation, nextLine);

    nextLine = strtok(line7, ":");
    nextLine = strtok(NULL, "\n");
    strcpy(velocityX, nextLine);

    nextLine = strtok(line8, ":");
    nextLine = strtok(NULL, "\n");
    strcpy(velocityY, nextLine);

    sprintf(buffer, "altitude:%s\nfuel:%s\nflying:%s\ncrashed:%s\norientation:%s\nVx:%s\nVy:%s\n"
    , altitude, fuel, flying, crashed, orientation, velocityX, velocityY);

    lcd.locate(0,0);
    lcd.printf("fuel: %s\n", fuel);

    send(buffer, strlen(buffer), dash);
}

/* Task for asynchronous UDP communications with dashboard */
void dashboard(void){
    /*TODO create and format a message to the Dashboard */
    /*TODO send the message to the dashboard:
        udp.sendto( dash, <message>, strlen(<message>));
    */
}

/* Task for polling sensors */
void user_input(void){
    roll = (right.read() * 2) - 1;
    throttle = (int)round(left.read() * 100);
    lcd.locate(0,0);
    lcd.printf("\nThrottle: %5.2f roll:%5.2f", throttle, roll);
    if (throttle >= 90)
    {
      redOn();
      greenOff();
      lcd.printf("\nWatch your fuel!!");
    }
    else
    {
      greenOn();
      redOff();
      lcd.printf("\n                   ");
    }
    communications();
}

int main() {
    eth.connect();
    /* ethernet connection : usually takes a few seconds */
    printf("connecting \n");
    /* write obtained IP address to serial monitor */
    const char *ip = eth.get_ip_address();
    printf("IP Address is: %s\n", ip ? ip : "No IP" );
    /* open udp for communications on the ethernet */
    udp.open(&eth);

    printf("lander is on %s/%d\n",lander.get_ip_address(),lander.get_port() );
    printf("dash   is on %s/%d\n",dash.get_ip_address(),dash.get_port() );

    /* periodic tasks */
    periodic.call_every(0.5, user_input);

    /* start event dispatching thread */
    dispatch.start( callback(&periodic, &EventQueue::dispatch_forever) );

    while(1) {}

}
