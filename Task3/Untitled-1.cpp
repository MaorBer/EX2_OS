#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void extract_last_tcpc(const char *input, char *output) {
    const char *tcpc_start = NULL;
    const char *temp = input;

    // Find the last occurrence of "TCPC" in the input string
    while ((temp = strstr(temp, "TCPC")) != NULL) {
        tcpc_start = temp;
        temp += 4; // Move past "TCPC"
    }

    if (tcpc_start == NULL) {
        strcpy(output, ""); // If "TCPC" not found, return an empty string
        return;
    }

    // Find the end of the TCPC string (ending with a space or end of string)
    const char *tcpc_end = strchr(tcpc_start, ' ');
    if (tcpc_end == NULL) {
        tcpc_end = input + strlen(input); // End of string
    }

    // Copy the TCPC string into the output
    strncpy(output, tcpc_start, tcpc_end - tcpc_start);
    output[tcpc_end - tcpc_start] = '\0'; // Null-terminate the output string
}


void parse_tcpc_string(const char *str, char *ip, int *port) {
    // Find the position of the comma
    const char *comma_pos = strchr(str, ',');
    if (!comma_pos) {
        // Comma not found, unable to parse
        *port = -1;
        return;
    }
    
    // Calculate the length of the IP address or hostname
    int ip_length = comma_pos - (str + 4); // Length of "TCPC"
    if (ip_length <= 0) {
        // Invalid IP address or hostname
        *port = -1;
        return;
    }

    // Copy the IP address or hostname
    strncpy(ip, str + 4, ip_length);
    ip[ip_length] = '\0'; // Null-terminate the string

    // Extract the port number and convert it to an integer
    *port = atoi(comma_pos + 1);
}

int main(int argc, char **argv) {


    const char *str1 = "TCPC127.127.127.127,4111";
    const char *str2 = "TCPC127.127.127.116,4568";
    char ip1[128]; // Allocate memory for the IP address
    int port1 = 0;
    char ip2[128]; // Allocate memory for the IP address
    int port2 = 0;
    parse_tcpc_string(str1, ip1, &port1);
   parse_tcpc_string(str2, ip2, &port2);
 
 
    if (port1 != -1) {
        printf("IP: %s\nPort: %d\n", ip1, port1);
    } else {
        printf("Invalid format or port not found in string: %s\n", str1);
    }

        if (port2 != -1) {
        printf("IP: %s\nPort: %d\n", ip2, port2);
    } else {
        printf("Invalid format or port not found in string: %s\n", str2);
    }


    return 0;


}
