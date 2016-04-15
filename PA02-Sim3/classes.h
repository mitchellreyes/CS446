#include <iostream>
#include <stdlib.h>
#include <iomanip>
#include <fstream>
#include <stdio.h>
#include <string.h>

#define LTB 100
#define LTM 200
#define LTF 300
#define leftOpenParenthesis '('
#define rightClosedParenthesis ')'

using namespace std;

class metaData{
    public:
        void setLetter(char);
        void setState(string);
        void setInitialTime(int);
        void setCycleTime(float);
        void setProcessNum(int);

        char getLetter();
        string getState();
        int getInitialTime();
        float getCycleTime();

        void readData(ifstream&);
        void printData(float, int, int, ofstream&);

    private:
        char SAIPO_letter;
        string state;
        int initialTime;
        float cycleTime;
        int processNum;
};

class configData{
    public:
        int whereToLog();
        float phase;
        string filePath;
        string schedCode;
        int quantumTime;
        int processorCycleTime;
        int monitorDisplayTime;
        int hardDriveCycleTime;
        int printerCycleTime;
        int keyboardCycleTime;
        string logTo;
        string logFilePath;
};
