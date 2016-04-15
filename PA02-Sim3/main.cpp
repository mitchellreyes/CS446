#include <iostream>
#include <queue>
#include <fstream>
//#include <stdlib.h>
#include <time.h>
#include <iomanip>
#include <pthread.h>
#include "classes.h"

#define MSEC 1000.0
#define START 0
#define END 1
#define LTB 100
#define LTM 200
#define LTF 300

using namespace std;

clock_t runtime;
float duration = 0.000000;

void* runIO(void* mdfData);
void runProcesses(queue<metaData> &mdfData, queue< queue<metaData> > &PCB, int whereTo, ofstream &out);
void printNode(metaData mdfData);
float calcCycleTime(int initialTime, string state, configData cnfData);
void get_mdfData(configData cnfData, queue< queue<metaData> > &PCB);
void getConfigData(const char* fileName, configData &cnfData);
void printPCB(queue< queue<metaData> > &PCB, queue<metaData> &mdfData);
void clearQueue(queue<metaData> &mdfData);




int main(int argc, char *argv[])
{
	queue< queue<metaData> > PCB; 
	queue<metaData> mdfData;
	queue<metaData> blocked;
	configData cnfData;
	int whereToLog;
	ofstream out;

	getConfigData(argv[1], cnfData);
	whereToLog = cnfData.whereToLog();
	if(whereToLog == LTB || whereToLog == LTF)
	{
		out.open((cnfData.logFilePath).c_str());
	}
	get_mdfData(cnfData, PCB);
	//printPCB(PCB, mdfData);
	//runProcesses(mdfData, PCB, whereToLog, out);
	return 0;
}

void printPCB(queue< queue<metaData> > &PCB, queue<metaData> &mdfData)
{
	int a = 1;
	while(!PCB.empty())
	{
		cout << "BLOCK " << a << ": " << endl;
		while(!mdfData.empty())
		{
			printNode(mdfData.front());
			mdfData.pop();
		}
		PCB.pop();
		a++;
	}
}

void* runIO(void* mdfData)
{
	//recast our data back to process data from void*
	metaData* myData = (metaData*)mdfData;
	//timestamp the duration it came in with
	float threadTime = duration;
	//stamp the duration again
	duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
	//holding what time we have to stop at
	float stopTime = (myData->getCycleTime());
	//wait for the timer to hit the stop time
	while((duration - threadTime) < stopTime)
	{
		//keep updating the duration so that the timer will keep running
		duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
	}
	// //hold the end duration for printing
	duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
	return NULL;
}

void runProcesses(queue<metaData> &mdfData, queue< queue<metaData> > &PCB, int whereTo, ofstream &out)
{
	metaData* needle;
	pthread_t t1;
	runtime = clock();
	int numProcess = 1;
	duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;

	cout << fixed << setprecision(6) << duration << " - Simulator program starting" << endl;
	printNode(mdfData.front());
	mdfData.pop();
	PCB.pop();
	printNode(mdfData.front());

	while(!PCB.empty())
	{
		cout << "STARTING PROCESS " << numProcess << ": " << endl;
		while(!mdfData.empty())
		{
			/*if((mdfData.front()).SAIPO_letter == 'A')
			{

			}*/
			needle = &(mdfData.front());
			duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;

			(mdfData.front()).printData(duration, START, whereTo, out);
			pthread_create(&t1, NULL, runIO, (void*)needle);
			pthread_join(t1, NULL);
			(mdfData.front()).printData(duration, END, whereTo, out);
			mdfData.pop();
		}
		cout << "EMPTY INNER MDFQUEUEU" << endl;
		PCB.pop();
		numProcess++;
	}//end of while statement
}//end of function

void printNode(metaData mdfData)
{
	cout << "SAIPO_letter: " << mdfData.getLetter() << endl;
	cout << "\tState: " << mdfData.getState() << endl;
	cout << "\tInitial Time: " << mdfData.getInitialTime() << endl;
	cout << "\tCycle Time: " << mdfData.getCycleTime() << endl << endl; 
}

float calcCycleTime(int initialTime, string state, configData cnfData)
{	
	int timeMultiple;
	if(state == "hard drive")
	{
		//take the number given for that name multiply it by the number given in the mdf then divide it by 1000 to get seconds
		timeMultiple = cnfData.hardDriveCycleTime;
	}
	else if(state == "keyboard")
	{
		timeMultiple = cnfData.keyboardCycleTime;
	}
	else if(state == "printer")
	{
		timeMultiple = cnfData.printerCycleTime;
	}
	else if(state == "processing")
	{
		timeMultiple = cnfData.processorCycleTime;
	}
	else if(state == "monitor")
	{
		timeMultiple = cnfData.monitorDisplayTime;
	}
	else
	{
		//if there is some other name, set the wait time to 0
		timeMultiple = 0;
	}
	return (initialTime * timeMultiple) / MSEC;
}

void get_mdfData(configData cnfData, queue< queue<metaData> > &PCB)
{
    ifstream fin;
    fin.open((cnfData.filePath).c_str(), ifstream::in);
    queue<metaData> mdfData;
    //int processNum = 0;
    // char SAIPO;
    // string deviceName;
    // int cycles;

    if(!fin.good())
    {
		cout << "\tUNABLE TO OPEN: " << cnfData.filePath << endl << "\tEXITING PROGRAM" << endl;
        exit(1);
    }
    else
    {
		cout << "\tREADING DATA FROM: " << cnfData.filePath << endl << endl;
		metaData processInfo;
		fin.ignore(512, ':');
		while(fin.good())
		{
			do
			{
				processInfo.readData(fin);
				processInfo.setCycleTime(calcCycleTime(processInfo.getInitialTime(), processInfo.getState(), cnfData));
				mdfData.push(processInfo);
			}while(processInfo.getLetter() != 'S' && (processInfo.getLetter() != 'A' && processInfo.getState() != "end"));

			//if(processInfo.getLetter() == 'S' && (processInfo.getState() == "start" || processInfo.getState() == "end"))
			//{
				PCB.push(mdfData);
				clearQueue(mdfData);
			// } 
			// else if(processInfo.getLetter() == 'A' && processInfo.getState() == "end")
			// {
			// 	PCB.push(mdfData);
			// 	clearQueue(mdfData);
			// }
		}
		fin.close();
    }	
}

void clearQueue(queue<metaData> &mdfData)
{
	while(!mdfData.empty())
	{
		mdfData.pop();
	}
}

void getConfigData(const char* fileName, configData &cnfData)
{
	ifstream fin;
	fin.open(fileName, ifstream::in);
	if(!fin.good())
	{
		cout << endl << "\tUNABLE TO OPEN: " << fileName << endl << "\tEXITING PROGRAM" << endl;
		exit(1);
	}
	else
	{
		cout << endl << "\tREADING DATA FROM: " << fileName << endl << endl;
		//read in all the data to the different config struct components
		fin.ignore(512, ':');
		fin >> cnfData.phase;
		fin.ignore(512, ':');
		fin >> cnfData.filePath;
		fin.ignore(512, ':');
		fin >> cnfData.schedCode;
		fin.ignore(512, ':');
		fin >> cnfData.quantumTime;
		fin.ignore(512, ':');
		fin >> cnfData.processorCycleTime;
		fin.ignore(512, ':');
		fin >> cnfData.monitorDisplayTime;
		fin.ignore(512, ':');
		fin >> cnfData.hardDriveCycleTime;
		fin.ignore(512, ':');
		fin >> cnfData.printerCycleTime;
		fin.ignore(512, ':');
		fin >> cnfData.keyboardCycleTime;
		fin.ignore(512, ':');
		fin.ignore(512, ' ');
		getline(fin, cnfData.logTo);
		fin.ignore(512, ':');
		fin >> cnfData.logFilePath;
		fin.close();
	}
}

