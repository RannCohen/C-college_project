/*************************************************************/
         /* project made by Ran Cohen - 300907953 */
/* the client sending struct to the server using UDP protocol*/
/*************************************************************/

#include <stdio.h> 
#include <stdlib.h>  // for "EXIT_FAILURE"
#include <unistd.h> 
#include <string.h> // for strings, memset
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> // server struct
#include <netinet/in.h> 

#define port 1307 // port number is my birthday date :) 
#define maxBuff 1024 // buffer maximum size (1024 bytes)

// format for the DB file
const char *Last_Client_Format = "\
The last values that the server recived from the client were:\n\
client id:                        %d\n\
time of massage:                  %u:%u %u\\%u\\%u\n\
processes running on the system:  %d\n\
Client IP:                        %s\n\
Client MAC adderss:               %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n";
const char *Last_Client_NETMASK = "\
Client NETMASK:                   %s\n";

// prototypes:
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
    unsigned char macAddr[6];        // 8 Bytes but Should be 6 Bytes
    int CRC;                         // 4 Bytes
} metMsg;


int main(void)
{
    int sockfd = 0, bindReturn = 0, len = 0, n = 0; 
    char *confirmation = "**The struct was recived and parsed by the server!**";
    struct sockaddr_in servaddr, cliaddr;
    
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
    memset(&cliaddr, 0, sizeof(cliaddr));

    // server info:
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port); // little endian to big endian

    /***** bind the socket to the server address *****/
    bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    
    // check if binding failed:
    if (bindReturn == -1)
    {
        printf("*** error while binding ***");
        exit(EXIT_FAILURE);
    }
    printf("\n***THE SERVER IS NOW RUNNING ***\n");
    while(1)
    {
        // allocate space for incoming struct
        metMsg * temp = malloc(sizeof(metMsg));

        // recive and print the message from the client:
        len = sizeof(cliaddr);
        n = recvfrom(sockfd, temp, sizeof(*temp), MSG_WAITALL, (struct sockaddr *) &cliaddr, \
        &len);
        
        int sum = 0;
        unsigned char *iter = (unsigned char*)temp;
        for (int i = 0; i < sizeof(metMsg)-4; i++)
        {
            sum += iter[i];
        }
        if (temp->CRC == sum) {
            printf("\n\n             *the message recived from the client is:*\n\n");
            printf("*client ID is:                                       %d\n", temp->clientID);
            printf("*the massage sent at:                                %u:%u %u\\%u\\%u\n", \
            temp->hh, temp->mm, temp->DD, temp->MM, temp->YY);
            printf("*the number of processes running on the system are:  %d\n", temp->numOfProc);
            printf("*client IP number is:                                %s\n", inet_ntoa(temp->IP));
            printf("*client netmask is:                                  %s\n", inet_ntoa(temp->netMask));
            printf("*client MAC address is:                              %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n" \
            ,temp->macAddr[0], temp->macAddr[1], temp->macAddr[2], temp->macAddr[3], temp->macAddr[4], \
            temp->macAddr[5]);
            printf("*CRC is:                                             %d\n", temp->CRC);

            // sendind back message to the client:
            sendto(sockfd, (const char *)confirmation, strlen(confirmation), MSG_CONFIRM, \
            (const struct sockaddr *) &cliaddr, len);

            printf("\n            **this massage was seaved to lastClient.DB!**\n");
            printf("\n                 *massage back was sent to client!*\n");

            // saving last values to "lastClient.DB" file
            FILE *fp = fopen("lastClient.DB", "w");
            if(!fp)
            {
                perror("\nFile opening failed\n");
                return EXIT_FAILURE;
            }
            fseek(fp, 0, SEEK_SET);
            fprintf(fp, Last_Client_Format, temp->clientID, temp->hh, \
            temp->mm, temp->DD, temp->MM, \
            temp->YY, temp->numOfProc, inet_ntoa(temp->IP), \
            temp->macAddr[0], \
            temp->macAddr[1], temp->macAddr[2], temp->macAddr[3], temp->macAddr[4], \
            temp->macAddr[5]);
            fprintf(fp, Last_Client_NETMASK, inet_ntoa(temp->netMask)); 
            fflush(fp);
            fclose(fp);

            fflush(stdout);
            free(temp);
        }else
        {
            printf("***ERROR - invalid CRC!!!***\n");
        }
        fflush(stdout);
    }
    return 0;
}