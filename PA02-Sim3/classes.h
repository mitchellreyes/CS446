#include <iostream>
#include <stdlib.h>
#include <iomanip>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <deque>

#define LTB 100
#define LTM 200
#define LTF 300
#define leftOpenParenthesis '('
#define rightClosedParenthesis ')'

using namespace std;

class metaData{
    public:
        metaData();
        ~metaData();

        void setLetter(char);
        void setState(string);
        void setInitialTime(int);
        void setCycleTime(float);
        void setProcessNum(int);

        char getLetter();
        string getState();
        int getInitialTime();
        float getCycleTime();
        int getProcessNum();


        void readData(ifstream&);
        void printData(float, int, int, ofstream&, int&);

    private:
        char SAIPO_letter;
        string state;
        int initialTime;
        float cycleTime;
        int processNum;
};

class process
{
    private:
        int processID;
    public:
        process();
        ~process();
        void copyActionQueue(deque<metaData>&);
        void setProcessID(int);
        int getProcessID();
        deque<metaData> actionQueue;

};

class configData{
    public:
        int whereToLog();
        float phase;
        string filePath;
        string schedCode;
        float quantumTime;
        int processorCycleTime;
        int monitorDisplayTime;
        int hardDriveCycleTime;
        int printerCycleTime;
        int keyboardCycleTime;
        string logTo;
        string logFilePath;
};
