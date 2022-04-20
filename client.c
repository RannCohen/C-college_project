/*************************************************************/
         /* project made by Ran Cohen - 300907953 */
/* the client sending struct to the server using UDP protocol*/
/*************************************************************/

#include <stdio.h> 
#include <stdlib.h>  // for "EXIT_FAILURE", calloc, rand
#include <unistd.h> 
#include <string.h> // for strings, memset
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> // server struct
#include <netinet/in.h> 
#include <termios.h>
#include <stdint.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define port 1307 // port number is my birthday date :) 
#define maxBuff 1024 // socket buffer maximum size (1024 bytes)

// Prototypes:
uint32_t getKey(void);

typedef struct metrologicMessage
{
    unsigned short clientID;         // 2 Bytes
    unsigned short hh;               // 2 Bytes
    unsigned short mm;               // 2 Bytes
    unsigned short YY;               // 2 Bytes
    unsigned short MM;               // 2 Bytes
    unsigned short DD;               // 2 Bytes
    int numOfProc;                   // 4 Bytes
    struct in_addr IP;               // 4 Bytes
    struct in_addr netMask;          // 4 Bytes
    unsigned char macAddr[6];        // 6 Bytes
    int CRC;                         // 4 Bytes
} metMsg;

/*** main ***/
int main(void)
{
    // struct decleration
    metMsg sendingStruct;
    char serverBuffer[maxBuff] = {0};
    // SOCKET
    uint32_t c = {0};
    uint8_t *pc = (uint8_t*)&c;
    int sockfd = 0, len = 0, n = 0;
    struct sockaddr_in servaddr;
    // CLIENT ID
    srand(time(0));

                  /*** socket stuff ***/
    /***** create sockfd (socket file descriptor) *****/
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // check if the socket creation was faild:
    if (sockfd == -1)
    {
        printf("*** error while creating socket ***");
        exit(EXIT_FAILURE);
    }
    
    // removing garbage from all adresses charcters by zeroing it:
    memset(&servaddr, 0, sizeof(servaddr));

    // server info:
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port); // little endian to big endian

    while(1)
    {
        // TIME
        time_t now;
        struct tm *info;
        time(&now);
        info = localtime(&now);
        // SYSTEMCALL for processes
        struct sysinfo si;
        sysinfo (&si);
        // IP ADDRESS AND DATA
        struct ifreq ifr;
        //char iface[] = "wlxd03745a55e30"; // my wifi usb card
        char iface[] = "eth0";

        printf("\n**********************************************************************\n");
        printf("\npress the \"S\" key to send message to the server or the \"Q\" key to quit\n");
        printf("\n");

        c = getKey();

        if ((c == 115) || (c == 83))
        {
            /*filling the struct*/
            sendingStruct.clientID = rand() % 64000;  // getting random number for the clientID, \
            limit to 32000
            sendingStruct.hh = info->tm_hour;
            sendingStruct.mm = info->tm_min;
            sendingStruct.YY = info->tm_year + 1900;
            sendingStruct.MM = info->tm_mon + 1;
            sendingStruct.DD = info->tm_mday;
            sendingStruct.numOfProc = si.procs;
            /***IP stuff***/
            ifr.ifr_addr.sa_family = AF_INET;
            strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);
            //get the ip address
            ioctl(sockfd, SIOCGIFADDR, &ifr);
            sendingStruct.IP = (((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr);
            //get the netmask
            ioctl(sockfd, SIOCGIFNETMASK, &ifr);
            sendingStruct.netMask = (((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr);
            //get the mac address
            ioctl(sockfd, SIOCGIFHWADDR, &ifr);
            memcpy(sendingStruct.macAddr, (unsigned char *)ifr.ifr_hwaddr.sa_data, 6);
            //sendingStruct.macAddr = (unsigned char *)ifr.ifr_hwaddr.sa_data;
            // CRC - simple simple checksum!!!
            int sum = 0;
            unsigned char *iter = (unsigned char*)&sendingStruct;
            for (int i = 0; i < sizeof(sendingStruct)-4; i++)
            {
                sum += iter[i];
            }
            sendingStruct.CRC = sum;
            
            // sending the struct to the server
            sendto(sockfd, (metMsg *)&sendingStruct, sizeof(sendingStruct), MSG_CONFIRM, \
            (const struct sockaddr *) &servaddr, sizeof(servaddr));
            printf("\n*** massage was sent to the server!!! ***\n\n");

            //recive and print the massage from the server:
            n = recvfrom(sockfd, (char *)serverBuffer, maxBuff, MSG_WAITALL, (struct sockaddr *) \
            &servaddr, &len);
            serverBuffer[n] = '\n';
            printf("\n\nThe server replied:\n\n%s\n", serverBuffer);
            fflush(stdout);
            continue;
        }
        else if ((c == 113) || (c == 81))
        {
            printf("\nTHANK YOU for using our UDP services, see you soon\n");
            break;
        }
        else if(c != 0)
        {
            printf("\n\n**ERROR! wrong key selection, please try again**\n\n\n\n");
        }
        fflush(stdout);
    }

    close(sockfd);
    return 0;
}

// FROM THE LESSON WITH IVGENY
unsigned int getKey(void)
{
    struct termios oldt, newt;
    uint32_t ch = {0};
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VTIME] = 255; // time to wait = 25.5sec
    newt.c_cc[VMIN] = 0; // minimum bytes to wait for
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    read(STDIN_FILENO, &ch, 4);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}