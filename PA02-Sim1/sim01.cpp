#include <stdio.h>
#include <string.h>
#include <queue>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <pthread.h>
#include <cstdlib>
#include <time.h>

using namespace std;

typedef struct process
{
	char pName;
	int burstTime;
	char state[30];
	float cycleTime;
}process;

typedef struct config
{
	float phase;
	string filePath;
	int processorCycleTime;
	int monitorDisplayTime;
	int hardDriveCycleTime;
	int printerCycleTime;
	int keyboardCycleTime;
	string logTo;
	string logFilePath;
}config;

clock_t runtime;
float duration = 0.000000;

int getData( config* cnfData, process* data);
int getConfigData(const char* fileName, config* data);
void calcCycleTime(config* cnfData, process* mdfData);
int createThread(process* mdfData);
void* runIO(process* data);


int main(int argc, char *argv[])
{
	
	config* configData = new config[10];
	process* PCB = new process[500];

	//getData will change dependant on getConfigData->logFile
	if(getConfigData(argv[1], configData) == 1)
	{
		return 1;
	}
	else
	{
		getData(configData, PCB);
	}
	calcCycleTime(configData, PCB);
	createThread(PCB);

	//createThread Loop
	return 0;
}

int getConfigData(const char* fileName, config* data)
{
	ifstream fin;
	fin.open(fileName, ifstream::in);
	string endCheck;
	if(!fin.good())
	{
		cout << "Unable to open config file" << endl;
		return 1;
	}
	else
	{
		fin.ignore(512, ':');
		fin >> data->phase;
		fin.ignore(512, ':');
		fin >> data->filePath;
		fin.ignore(512, ':');
		fin >> data->processorCycleTime;
		fin.ignore(512, ':');
		fin >> data->monitorDisplayTime;
		fin.ignore(512, ':');
		fin >> data->hardDriveCycleTime;
		fin.ignore(512, ':');
		fin >> data->printerCycleTime;
		fin.ignore(512, ':');
		fin >> data->keyboardCycleTime;
		fin.ignore(512, ':');
		fin >> data->logTo;
		fin.ignore(512, ':');
		fin >> data->logFilePath;
		fin.close();
		return 0;
	}
}

void calcCycleTime(config* cnfData, process* mdfData)
{	
	//for every instance in the struct array of the mdf file
	for(int i = 0; i < 10; i++)
	{
		if(strcmp(mdfData[i].state, "(harddrive)") == 0)
		{
			mdfData[i].cycleTime = (mdfData[i].burstTime * cnfData->hardDriveCycleTime) / 1000.0;
		}
		else if(strcmp(mdfData[i].state, "(keyboard)") == 0)
		{
			mdfData[i].cycleTime = (mdfData[i].burstTime * cnfData->keyboardCycleTime) / 1000.0;
		}
		else if(strcmp(mdfData[i].state, "(printer)") == 0)
		{
			mdfData[i].cycleTime = (mdfData[i].burstTime * cnfData->printerCycleTime) / 1000.0;
		}
		else if(strcmp(mdfData[i].state, "(run)") == 0)
		{
			mdfData[i].cycleTime = (mdfData[i].burstTime * cnfData->processorCycleTime) / 1000.0;
		}
		else if(strcmp(mdfData[i].state, "(monitor)") == 0)
		{
			mdfData[i].cycleTime = (mdfData[i].burstTime * cnfData->monitorDisplayTime) / 1000.0;
		}
		else
		{
			mdfData[i].cycleTime = 0;
		}
	}
}

/*Start Program Meta-Data Code:
S(start)0; A(start)0; I(keyboard)6; O(printer)7; P(run)5; 
P(run)10; I(keyboard)9; O(hard drive)10; I(hard drive)9; 
O(hard drive)12; P(run)8; I(hard drive)9; P(run)15; O(hard drive)10; 
I(keyboard)13; O(hard drive)10; P(run)14; A(end)0; S(end)0.
End Program Meta-Data Code.*/

int getData( config* cnfData, process* data)
{
	//converting the string to a char array
	int size_of_filePath = (cnfData->filePath).size();
	int index;
	char fileName[50];
	for(index = 0; index < size_of_filePath; index++)
	{
		fileName[index] = cnfData->filePath[index];
	}
	fileName[index] = '\0';
    /*file pointer and open it*/
    ifstream fin;
    fin.open(fileName, ifstream::in);

    /*if you can open/find the file*/
    if(!fin.good())
    {
        cout << "Unable to open file path" << endl;
        return 1;
    }
    /*storing the data into the array*/
    else
    {
    	char stateCheck;
    	index = 0;
    	fin.ignore(512, ':');
    	while(fin.good())
    	{
    		fin >> data->pName;
    		fin >> stateCheck;
    		while(stateCheck != ')')
    		{
    			data->state[index] = stateCheck;
    			index++;
    			fin >> stateCheck;

    		}
    		data->state[index] = ')';
			index++;
			//terminate the string with a null char
			data->state[index] = '\0';
			fin >> data->burstTime;
			fin >> stateCheck;
			if(stateCheck == ';')
			{
				data++;
				index = 0;
			}
			else if(stateCheck == '.')
			{
				fin.close();
				return 0;
			}
    	}
    }

    /*Close the file after succesful reading*/
    fin.close();
    return 0;
}

void* runIO(void* mdfData)
{
	//recast our data back to process data from void*
	process* myData = (process*)mdfData;
	cout << "this is my process in the thread creation: " << myData->pName << endl;
	//do the math for the clock to wait
	float threadTime = duration;
	printf("%.6f - duration before wait time\n", duration);
	//waiting until it hits the time duration
	//while((duration - threadTime) < myData->cycleTime);
	//printf("%.6f - Simulator program starting\n", duration);
	//duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
	//cout << "duration in the thread process: " << duration << endl;
}

int createThread(process* mdfData)
{
	pthread_t t1;
	process* needle;
	runtime = clock();
	duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
	printf("%.6f - Simulator program starting\n", duration);

	for(int i = 0; i < 10; i++)
	{	
		duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
		//thread processes
		if(mdfData[i].pName == 'I' || mdfData[i].pName == 'O')
		{
			*needle = mdfData[i];
			cout << "Going into thread creation at loop: " << duration << endl;
			//cout << "Look I found an " << mdfData[i].pName << endl;
			pthread_create(&t1, NULL, runIO, (void*)needle);
			pthread_join(t1, NULL);
		}
		//printf("%.6f - loop %d duration\n", duration, i); //should print out start/end of the cycle
		duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
	}
}