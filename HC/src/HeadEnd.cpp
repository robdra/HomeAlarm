#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <ctime>
#include <fstream>
#include <string>

#include "SerialPort.h"

#define POLL 500
//#define test

void failed();
std::string getDate();
void sysLog(std::string msg);
void fullTrig();
void shellTrig();
void failedAttempt();
void Send(std::string msg);
void PassCheck(int choice, std::string pass);

char portNo[] = "\\\\.\\COM11";
char *port_name = portNo;

char inData[MAX_DATA_LENGTH];
SerialPort arduino(port_name);

int main()
{
    //Checks if the arduino is connected
    if(arduino.isConnected())
    {
        std::cout << "Arduino is connected" << std::endl;
    }
    else
    {
        std::cout << "Arduino is not connected" << std::endl;
    }

    while(arduino.isConnected())
    {
        //Reads from serial and stores eventual data in an inData char array
        int read_result = arduino.readSerialPort(inData, MAX_DATA_LENGTH);
        if(read_result > 0)
        {
            #ifdef test
                std::cout << "Indata is: " << inData << std::endl;
            #endif

            Sleep(POLL);
            //Checks the first character of inData
            if(inData[0] == 'A')
            {
                //If the first char is A it removes the A and converts the rest 
                //of the array to a string ands send it to be checked against the passwords
                std::string str(inData);
                std::string pin = str.substr(1);
                PassCheck(0, pin);
            }
            if(inData[0] == 'B')
            {
                //If the first char is B it means that the shell sensors have been triggered
                shellTrig();
                char second = inData[1];
                //Also checks if the interior sensors have been triggered
                if(second == 'C')
                {
                    fullTrig();
                }
                memset(inData, 0, sizeof(inData));
            }
            if(inData[0] == 'C')
            {
                //If the first char is C it means that the interior sensors have been triggered
                fullTrig();
                char second = inData[1];
                //Also checks if the shell sensors have been triggered
                if(second == 'B')
                {
                    shellTrig();
                }
                memset(inData, 0, sizeof(inData));
            }
            if(inData[0] == 'D')
            {
                memset(inData, 0, sizeof(inData));
                //If the first char is D it means three tries have been made and failed to deactivate the alarm
                failed();
            }
            if(inData[0] == 'E')
            {
                //If the first char is E it means that someone wants to reactivate the alarm.
                //Again it checks the pin against the saved passwords but also sends a 1 to
                //get a different system log message
                std::string str(inData);
                std::string pin = str.substr(1);
                PassCheck(1, pin);
            }
        }

        Sleep(POLL);
    }

    return 0;
}
//Sends data to the arduino
void Send(std::string msg)
{
    char outData[msg.length()];
    std::size_t len = msg.copy(outData, msg.length(), 0);
    outData[len] = '\0';

    bool res = arduino.writeSerialPort(outData, sizeof(outData));
}

void PassCheck(int choice, std::string pass)
{
    #ifdef test
        std::cout << "In passcheck" << std::endl;
    #endif
    std::ifstream inFile;
    std::string id, pin, assaultPin, tag, status, reserved;
    bool noMatch = true;
    
    inFile.open("user.dat");
 
    if(inFile.is_open())
    {
        while(!inFile.eof())
        {   
            #ifdef test
                std::cout << "Open infile" << std::endl;
            #endif
            //Each loop it takes every value separeted by comma and store it in variables.
            //Then it checks if the stored password matches the pressed password
            getline(inFile, id, ';');
            getline(inFile, pin, ';');
            getline(inFile, assaultPin, ';');
            getline(inFile, tag, ';');
            getline(inFile, status, ';');
            getline(inFile, reserved, '\n');
            //If a match is found it closes the infile and calls the system logging
            //function and sets noMatch to false
            if(pass == pin)
            {
                #ifdef test
                    std::cout << "Pinmatch" << std::endl;
                #endif
                inFile.close();
                memset(inData, 0, sizeof(inData));
                if(choice == 0)
                {
                    sysLog("Deactivated alarm");
                }
                if (choice == 1)
                {
                    sysLog("Alarm reactivated");
                }
                noMatch = false;
                break;
            }
            //If the pin matches an assaultpin it again sets noMatch to false
            //but also log the event and prints to the terminal that an assaultpin was used
            else if(pass == assaultPin)
            {
                inFile.close();
                memset(inData, 0, sizeof(inData));
                std::cout << "Assault pin used" << std::endl;
                sysLog("The assaultpin was used");
                noMatch = false;
                break;
            }
        }
    }
    inFile.close();
    //If mo match could be found noMatch will be true and a 0 is sent back to the arduino
    if(noMatch)
    {
        memset(inData, 0, sizeof(inData));
        failedAttempt();
        Send("0");
    }
    //If a match is found noMatch will be set to false and a 1 will be sent back to the arduino
    Send("1");
}
//Sends a message to the system logging function.
//These are their own functions to make it easier to
//extend their functionalities
void failedAttempt()
{
    sysLog("Failed attempt to log in");
}
//Sends a message to the system logging function if interior sensors were triggered
void fullTrig()
{
    sysLog("Inside sensors triggered");
}
//Sends a message to the syslog if shell sensors were triggered
void shellTrig()
{
    sysLog("Shell sensors triggered");
}
//This function logs the system events in a textfile
void sysLog(std::string msg)
{
    std::ofstream outFile;
    std::ifstream inFile;
    std::string reserved, id;
    char c;
    int rows = 0;
    std::string date = getDate();
    
    inFile.open("system.log");

    if(inFile.is_open())
    {
        while(inFile.get(c))
        {   //Checks how many rows are already in the log file
            if(c == '\n')
            {
                rows++;
            }
        }
        inFile.close();
    }

    outFile.open("system.log", std::ofstream::app);
    outFile << rows+1 << ";" << date << ";" << id << ";" << reserved << ";" << msg << '\n';
    outFile.close();
}
//Gets the current date
std::string getDate()
{
    time_t now = time(0);
    struct tm tstruct;
    char       date[80];
    tstruct = *localtime(&now);

    strftime(date, sizeof(date), "%Y%m%d %X", &tstruct);

    return date;
}
//Sends a message to syslog if three failed attempts to deactivate alarm was made
void failed()
{
    sysLog("Failure to deactivate alarm. System locked");
}