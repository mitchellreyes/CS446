#include <iostream>
#include <queue>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <iomanip>
#include <pthread.h>
#include "classes.h"

#define leftOpenParenthesis '('
#define rightClosedParenthesis ')'
#define MSEC 1000.0

using namespace std;

clock_t runtime;
float duration = 0.000000;

void* runIO(void* mdfData);
void runProcesses(queue<metaData> &mdfData);
void printNode(metaData mdfData);
float calcCycleTime(int initialTime, string state, configData cnfData);
void get_mdfData(configData cnfData, queue<metaData> &mdfData);
void getConfigData(const char* fileName, configData &cnfData);


int main(int argc, char *argv[])
{
	queue<metaData> mdfData;
	configData cnfData;

	getConfigData(argv[1], cnfData);
	get_mdfData(cnfData, mdfData);
	runProcesses(mdfData);
	return 0;
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
	float stopTime = (myData->cycleTime);
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

void runProcesses(queue<metaData> &mdfData)
{
	metaData* needle;

	ofstream out;
// 	int index;
// 	int whereTo = 0;
// 	int blockNum = 0;
// 	//if we need to write our info to a file

// 	if(cnfData->logTo == "Log to Both")
// 	{
// 		whereTo = LTB;
// 		out.open((cnfData->logFilePath).c_str());
// 	}
// 	else if(cnfData->logTo == "Log to File")
// 	{
// 		whereTo = LTF;
// 		out.open((cnfData->logFilePath).c_str());
// 	}
// 	else
// 	{
// 		whereTo = LTM;
// 	}

	pthread_t t1;
	//duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
//starting our clock variable
	cout << fixed;
	cout << setprecision(6);

	runtime = clock();
	duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
	cout << duration << " - Simulator program starting" << endl;
	mdfData.pop();
	while(!mdfData.empty())
	{
		if((mdfData.front()).SAIPO_letter == 'S')
		{
			duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
			cout << duration << " - Simulator program ending" << endl;
			exit(0);
		}
		/*if((mdfData.front()).SAIPO_letter == 'A')
		{

		}*/
		needle = &(mdfData.front());
		if((mdfData.front()).SAIPO_letter == 'I')
		{
			// check where to log start info
			// writeInfo('I', START, whereTo, mdfData, index, out, blockNum);
			cout << duration << " - Process 1: start " << (mdfData.front()).state << " input" << endl;
			pthread_create(&t1, NULL, runIO, (void*)needle);
			pthread_join(t1, NULL);
			cout << duration << " - Process 1: end " << (mdfData.front()).state << " input" << endl;
			// writeInfo('I', END, whereTo, mdfData, index, out, blockNum);
		}
		else if((mdfData.front()).SAIPO_letter == 'P')
		{
			cout << duration << " - Process 1: start " << (mdfData.front()).state << " action" << endl;
			pthread_create(&t1, NULL, runIO, (void*)needle);
			pthread_join(t1, NULL);
			cout << duration << " - Process 1: end " << (mdfData.front()).state << " action" << endl;
		}
		else if((mdfData.front()).SAIPO_letter == 'O')
		{
			cout << duration << " - Process 1: start " << (mdfData.front()).state << " output" << endl;
			pthread_create(&t1, NULL, runIO, (void*)needle);
			pthread_join(t1, NULL);
			cout << duration << " - Process 1: end " << (mdfData.front()).state << " output" << endl;
		}
		mdfData.pop();
	}//end of while statement
}//end of function

void printNode(metaData mdfData)
{
	cout << "SAIPO_letter: " << mdfData.SAIPO_letter << endl;
	cout << "\tState: " << mdfData.state << endl;
	cout << "\tInitial Time: " << mdfData.initialTime << endl;
	cout << "\tCycle Time: " << mdfData.cycleTime << endl << endl; 
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
	else if(state == "run")
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

void get_mdfData(configData cnfData, queue<metaData> &mdfData)
{
    ifstream fin;
    fin.open((cnfData.filePath).c_str(), ifstream::in);

    char SAIPO;
    string deviceName;
    int cycles;

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
			fin >> SAIPO;
			processInfo.setLetter(SAIPO);
			fin.ignore(512, leftOpenParenthesis);
			getline(fin, deviceName, rightClosedParenthesis);
			processInfo.setState(deviceName);
			fin >> cycles;
			processInfo.setInitialTime(cycles);
			processInfo.setCycleTime(calcCycleTime(cycles, deviceName, cnfData));
			fin.ignore(512, ';');
			//printNode(processInfo);
			mdfData.push(processInfo);
		}
		fin.close();
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

