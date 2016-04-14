#include <iostream>
using namespace std;

class metaData{
    public:
        void setLetter(char);
        void setState(string);
        void setInitialTime(int);
        float setCycleTime(float);
    private:
        char SAIPO_letter;
        string state;
        int initialTime;
        float cycleTime;
};

class configData{
    public:
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
