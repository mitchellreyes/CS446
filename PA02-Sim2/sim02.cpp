#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <pthread.h>
#include <cstdlib>
#include <time.h>
#include <iomanip>

#define LTB 1 //log to both is set as 1
#define LTF 2 //log to file is set as 2
#define LTM 3 //log to monitor is set as 3
#define START 0
#define END 1

using namespace std;

//struct declarations to store data//////////
typedef struct process
{
	char pName;
	int burstTime;
	char state[30];
	float cycleTime;
	string stateName;
}process;
//------------------------------------------//
typedef struct config
{
	float phase;
	string filePath;
	string schedCode;
	int processorCycleTime;
	int monitorDisplayTime;
	int hardDriveCycleTime;
	int printerCycleTime;
	int keyboardCycleTime;
	string logTo;
	string logFilePath;
}config;
///////////////////////////////////////////

//clock variables//////////////////////////
clock_t runtime;
float duration = 0.000000;
//////////////////////////////////////////

//function prototypes////////////////////////////////////////////////////
int getData( config* cnfData, process* data, int &numProcesses);
int getConfigData(const char* fileName, config* data);
void calcCycleTime(config* cnfData, process* mdfData, int numProcesses);
int createThread(process* mdfData, int numProcesses, config* cnfData);
void* runIO(process* data);
void convertProcessName(process* mdfData, int numProcesses);
void writeInfo(char SAIPO, int timeLine, int whereTo, process* mdfData, int index, ofstream &out, int &pNum);
void SJF_sort(process* mdfData, config* cnfData, int numProcesses);
void structAssign(process &left, process &right);
void FCFS_sort(process* mdfData, config* cnfData, int numProcesses);
void SRTFN_sort(process* mdfData, config* cnfData, int numProcesses);



/////////////////////////////////////////////////////////////////////////


//start of main function////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	//creating a config struct to store the config data
	config* configData = new config[1];
	//creating a process struct to hold all the mdf data
	process* PCB = new process[500];
	//able to count the number of processes in the process array
	int numProcesses = 0;

	//read in data
	if(getConfigData(argv[1], configData) == 1)
	{
		return 1;
	}
	else
	{
		getData(configData, PCB, numProcesses);
	}
	//calculate the cycle times for each process
	calcCycleTime(configData, PCB, numProcesses);
	//set the process name for each MDF name
	convertProcessName(PCB, numProcesses);
	//calling create thread for each I/O letter
	if(configData->schedCode == "SJF")
	{
		SJF_sort(PCB, configData, numProcesses);
		createThread(PCB, numProcesses, configData);
	}
	if(configData->schedCode == "FCFS")
	{
		FCFS_sort(PCB, configData, numProcesses);
	}
	if(configData->schedCode == "SRTF-N")
	{
		SRTFN_sort(PCB, configData, numProcesses);
		createThread(PCB, numProcesses, configData);
	}

	return 0;
}
///////////////////////////////////////////////////////////////////////


//@function: reading in the config data/////////////////////////////
/*checks if the file is okay, if not, then print an error, else, read in the data from a file into the struct components*/
int getConfigData(const char* fileName, config* data)
{
	ifstream fin;
	fin.open(fileName, ifstream::in);
	//return an error if you cant read in the file
	if(!fin.good())
	{
		cout << "Unable to open config file" << endl;
		return 1;
	}
	else
	{
		//read in all the data to the different config struct components
		fin.ignore(512, ':');
		fin >> data->phase;
		fin.ignore(512, ':');
		fin >> data->filePath;
		//2.0/////added to obtain the scheduling type/////////////
		fin.ignore(512, ':');
		fin >> data->schedCode;
		/////////////////////////////////////////////////////////
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
		fin.ignore(512, ' ');
		getline(fin, data->logTo);
		fin.ignore(512, ':');
		fin >> data->logFilePath;
		fin.close();
		return 0;
	}
}
///////////////////////////////////////////////////////////////////



//@function: finding out how long each cycle time should be for, depending on the config file//////////
/*takes in our config data, our mdf data, and the number of processes we have, then calculates the cycletime for each
state we have*/
void calcCycleTime(config* cnfData, process* mdfData, int numProcesses)
{	
	int index;
	//for every instance in the struct array of the mdf file
	for(index = 0; index < numProcesses; index++)
	{
		//check the name in the cnf
		if(strcmp(mdfData[index].state, "(harddrive)") == 0)
		{
			//take the number given for that name multiply it by the number given in the mdf then divide it by 1000 to get seconds
			mdfData[index].cycleTime = (mdfData[index].burstTime * cnfData->hardDriveCycleTime) / 1000.0;
		}
		else if(strcmp(mdfData[index].state, "(keyboard)") == 0)
		{
			mdfData[index].cycleTime = (mdfData[index].burstTime * cnfData->keyboardCycleTime) / 1000.0;
		}
		else if(strcmp(mdfData[index].state, "(printer)") == 0)
		{
			mdfData[index].cycleTime = (mdfData[index].burstTime * cnfData->printerCycleTime) / 1000.0;
		}
		else if(strcmp(mdfData[index].state, "(run)") == 0)
		{
			mdfData[index].cycleTime = (mdfData[index].burstTime * cnfData->processorCycleTime) / 1000.0;
		}
		else if(strcmp(mdfData[index].state, "(monitor)") == 0)
		{
			mdfData[index].cycleTime = (mdfData[index].burstTime * cnfData->monitorDisplayTime) / 1000.0;
		}
		else
		{
			//if there is some other name, set the wait time to 0
			mdfData[index].cycleTime = 0;
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////



//@function: getting the data from the mdf file that is read from the cnf file/////////////////////////////
int getData( config* cnfData, process* data, int &numProcesses)
{
    /*file pointer and open it*/
    ifstream fin;
    fin.open((cnfData->filePath).c_str(), ifstream::in);

    /*if you can open/find the file*/
    if(!fin.good())
    {
        cout << "Unable to open file path" << endl;
        return 1;
    }
    else
    {
    	//read in the data to each component of the process struct
    	char stateCheck;
    	int index = 0;
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
				numProcesses++;
				index = 0;
			}
			else if(stateCheck == '.')
			{
				fin.close();
				return 0;
			}
    	}
    }
    fin.close();
    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//@function: what each 'I' or 'O' will do once the thread is created///////////////////
void* runIO(void* mdfData)
{
	//recast our data back to process data from void*
	process* myData = (process*)mdfData;
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
	//hold the end duration for printing
	duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
	return NULL;
}
//////////////////////////////////////////////////////////////////////////////////////




//@function: where to thread and where to print/////////////////////////////////////////
int createThread(process* mdfData, int numProcesses, config* cnfData)
{

	ofstream out;
	int index;
	int whereTo = 0;
	int blockNum = 0;
	//if we need to write our info to a file

	if(cnfData->logTo == "Log to Both")
	{
		whereTo = LTB;
		out.open((cnfData->logFilePath).c_str());
	}
	else if(cnfData->logTo == "Log to File")
	{
		whereTo = LTF;
		out.open((cnfData->logFilePath).c_str());
	}
	else
	{
		whereTo = LTM;
	}

	//creating our threads
	pthread_t t1;

	//pointing to a process, so that the thread process knows which process it is still
	process* needle;

	//starting our clock variable
	runtime = clock();

	//making it print out 6 decimals to the screen and file///////
	cout << fixed;
	cout << setprecision(6);

	out << fixed;
	out << setprecision(6);
	//////////////////////////////////////////////////////////////

	//start the clock
	duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
	writeInfo('G', START, whereTo, NULL, -1, out, blockNum);
	for(index = 0; index < numProcesses; index++)
	{	
		//cout << "process at " << index << ":" << mdfData[index].pName << endl;
		//if were at the start of the program
		if(mdfData[index].pName == 'S' && (strcmp(mdfData[index].state, "(start)") == 0))
		{
			//get the time it took to get here
			duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
			writeInfo('S', START, whereTo, NULL, 0, out, blockNum);
		}
		else if(mdfData[index].pName == 'A' && (strcmp(mdfData[index].state, "(start)") == 0))
		{
			//get the time it took to get here
			duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
			//checking where to log the info
			//say starting process 1
			if(index < numProcesses)
			{
				blockNum++;
				writeInfo('X', START, whereTo, mdfData, blockNum, out, blockNum);
				duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
				writeInfo('A', START, whereTo, mdfData, blockNum, out, blockNum);
			}
		}
		//if were at the end of the program
		else if(mdfData[index].pName == 'A' && (strcmp(mdfData[index].state, "(end)") == 0))
		{
			//get the time it took to get here
			duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
			//say remove process 1
			//if were not at the end of the process list
			if(index < numProcesses)
			{
				//blockNum++;
				writeInfo('A', END, whereTo, mdfData, blockNum, out, blockNum);
			}
			//exit(0);
		}
		//for every other process in the middle
		else
		{
			//get the timestamp of how long it took for us to get here 
			duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
			//point to the current process we are at in case of threading for the 'I' and the 'O'
			needle = &(mdfData[index]);
			//if we are at an 'I'
			if(mdfData[index].pName == 'I')
			{
				//check where to log start info
				writeInfo('I', START, whereTo, mdfData, index, out, blockNum);
				//THREADING PROCESS///////////////////////////////////////////////////////////////////////////
				//thread the process
				pthread_create(&t1, NULL, runIO, (void*)needle);
				//wait for it to die and join the threads
				pthread_join(t1, NULL);
				//////////////////////////////////////////////////////////////////////////////////////////////
				writeInfo('I', END, whereTo, mdfData, index, out, blockNum);
			}
			//if we are at an 'O'
			else if(mdfData[index].pName == 'O')
			{
				//check where to log the data
				writeInfo('O', START, whereTo, mdfData, index, out, blockNum);
				//THREADING PROCESS/////////////////////////////////////////////////////////////////////////
				pthread_create(&t1, NULL, runIO, (void*)needle);
				pthread_join(t1, NULL);
				///////////////////////////////////////////////////////////////////////////////////////////
				writeInfo('O', END, whereTo, mdfData, index, out, blockNum);

			}
			else if(mdfData[index].pName == 'P')
			{
				writeInfo('P', START, whereTo, mdfData, index, out, blockNum);
				pthread_create(&t1, NULL, runIO, (void*)needle);
				pthread_join(t1, NULL);
				writeInfo('P', END, whereTo, mdfData, index, out, blockNum);
			}
		}
	}

	//say the end of program and how long it took for us to get here
	duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
	writeInfo('S', END, whereTo, NULL, 0, out, blockNum);
	out.close();
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//@function writing out the info where it needs to be//
//var SAIPO is the letter of the process we are on
//var timeline is if we are at the start(0) or end(1) of the process (could be 1 or 0)
//var whereTo will tell you if it is log to both or log to file// 
void writeInfo(char SAIPO, int timeLine, int whereTo, process* mdfData, int index, ofstream &out, int &pNum)
{
	switch(SAIPO)
	{
		case 'G': //G for go (or beginning of the scheduling)
			if(timeLine == START)
			{
				if(whereTo == LTB || whereTo == LTF)
				{
					out << duration << " - Simulator program starting" << endl;
				}
				if(whereTo == LTB || whereTo == LTM)
				{
					cout << duration << " - Simulator program starting" << endl;
				}
			}
			break;
		case 'X':
			if(whereTo == LTB || whereTo == LTF)
			{
				out << duration << " - OS: Selecting next process" << endl;
			}
			if(whereTo == LTB || whereTo == LTM)
			{
				cout << duration << " - OS: Selecting next process" << endl;
			}
			break;
		////////////////////////////////////////////////////////////////////////
		case 'S':
			//if its at the start of the process
			if(timeLine == START)
			{
				if(whereTo == LTB || whereTo == LTF)
				{
					out << duration << " - OS: preparing all processes" << endl;
				}
				if(whereTo == LTB || whereTo == LTM)				
				{
					cout << duration << " - OS: preparing all processes" << endl;
				}
			}
			else
			{
				if(whereTo == LTB || whereTo == LTF)
				{
					out << duration << " - Simulator program ending" << endl;
				}
				if(whereTo == LTB || whereTo == LTM)				
				{
					cout << duration << " - Simulator program ending" << endl;	
				}
			}
			break;
		/////////////////////////////////////////////////////////////////////////
		case 'A':
			if(timeLine == START)
			{
				if(whereTo == LTB || whereTo == LTF)
				{
					out << duration << " - OS: starting process " << index << endl;
				}
				if(whereTo == LTB || whereTo == LTM)
				{
					cout << duration << " - OS: starting process " << index << endl;
				}
			}
			else
			{
				if(whereTo == LTB || whereTo == LTF)
				{
					out << duration << " - OS: removing process " << index << endl;
				}
				if(whereTo == LTB || whereTo == LTM)
				{
					cout << duration << " - OS: removing process " << index << endl;
				}
			}
			break;
		////////////////////////////////////////////////////////////////////////////////////////////////
		case 'I':
			if(timeLine == START)
			{
				if(whereTo == LTB || whereTo == LTF)
				{
					out << duration << " - Process " << pNum << ": start " << mdfData[index].stateName << " input" << endl;
				}
				if(whereTo == LTB || whereTo == LTM)
				{
					cout << duration << " - Process " << pNum << ": start " << mdfData[index].stateName << " input" << endl;
				}
			}
			else
			{
				if(whereTo == LTB || whereTo == LTF)
				{
					out << duration << " - Process " << pNum << ": end " << mdfData[index].stateName << " input" << endl;
				}
				if(whereTo == LTB || whereTo == LTM)
				{
					cout << duration << " - Process " << pNum << ": end " << mdfData[index].stateName << " input" << endl;
				}
			}
			break;
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		case 'P':
			if(timeLine == START)
			{
				if(whereTo == LTB || whereTo == LTF)
				{
					out << duration << " - Process " << pNum << ": start " << mdfData[index].stateName << " action" << endl;
				}
				if(whereTo == LTB || whereTo == LTM)
				{
					cout << duration << " - Process " << pNum << ": start " << mdfData[index].stateName << " action" << endl;
				}
			}
			else
			{
				if(whereTo == LTB || whereTo == LTF)
				{
					out << duration << " - Process " << pNum << ": end " << mdfData[index].stateName << " action" << endl;
				}
				if(whereTo == LTB || whereTo == LTM)
				{
					cout << duration << " - Process " << pNum << ": end " << mdfData[index].stateName << " action" << endl;
				}
			}
			break;
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		case 'O':
			if(timeLine == START)
			{
				if(whereTo == LTB || whereTo == LTF)
				{
					out << duration << " - Process " << pNum << ": start " << mdfData[index].stateName << " output" << endl;
				}
				if(whereTo == LTB || whereTo == LTM)
				{
					cout << duration << " - Process " << pNum << ": start " << mdfData[index].stateName << " output" << endl;
				}
			}
			else
			{
				if(whereTo == LTB || whereTo == LTF)
				{
					out << duration << " - Process " << pNum << ": end " << mdfData[index].stateName << " output" << endl;
				}
				if(whereTo == LTB || whereTo == LTM)
				{
					cout << duration << " - Process " << pNum << ": end " << mdfData[index].stateName << " output" << endl;
				}
			}
			break;
		////////////////////////////////////////////////////////////////////////////////////////////////////////////
		default:
			break;
	}
}

//sort for FCFS, SJF, and maybe SRTF-N?
void SJF_sort(process* mdfData, config* cnfData, int numProcesses)
{
	int index;
	process temp;
	for(index = 0; index < numProcesses; index++)
	{
		if((mdfData[index].burstTime > mdfData[index+1].burstTime) && (mdfData[index+1].burstTime != 0))
		{
			structAssign(temp, mdfData[index]);
			structAssign(mdfData[index], mdfData[index + 1]);
			structAssign(mdfData[index + 1], temp);
		}
	}
}

void FCFS_sort(process* mdfData, config* cnfData, int numProcesses)
{
	createThread(mdfData, numProcesses, cnfData);
}

void SRTFN_sort(process* mdfData, config* cnfData, int numProcesses)
{
	int index;
	process temp;
	for(index = 0; index < numProcesses; index++)
	{
		if((mdfData[index].burstTime > mdfData[index+1].burstTime) && (mdfData[index+1].burstTime != 0))
		{
			structAssign(temp, mdfData[index]);
			structAssign(mdfData[index], mdfData[index + 1]);
			structAssign(mdfData[index + 1], temp);
		}
	}
}

//left is the "Empty struct"
//right has the data we need
	/*char pName;
	int burstTime;
	char state[30];
	float cycleTime;
	string stateName;*/
void structAssign(process &left, process &right)
{
	int index = 0;
	left.pName = right.pName;
	left.burstTime = right.burstTime;
	/*if(left.state[index] != '\0')
	{
		delete left.state;
	}*/
	while(right.state[index] != '\0')
	{
		left.state[index] = right.state[index];
		index++;
	}
	left.state[index] = '\0';
	left.cycleTime = right.cycleTime;
	left.stateName = right.stateName;

}


//giving each process their actual name////////////////////////////////////////////
void convertProcessName(process* mdfData, int numProcesses)
{
	int index;
	//for every process
	for(index = 0; index < numProcesses; index++)
	{
		//check the name
		if(strcmp(mdfData[index].state, "(harddrive)") == 0)
		{
			//set a name to it
			mdfData[index].stateName = "hard drive";
		}
		else if(strcmp(mdfData[index].state, "(keyboard)") == 0)
		{
			mdfData[index].stateName = "keyboard";
		}
		else if(strcmp(mdfData[index].state, "(printer)") == 0)
		{
			mdfData[index].stateName = "printer";
		}
		else if(strcmp(mdfData[index].state, "(run)") == 0)
		{
			mdfData[index].stateName = "processing";
		}
		else if(strcmp(mdfData[index].state, "(monitor)") == 0)
		{
			mdfData[index].stateName = "monitor";
		}
		else
		{
			mdfData[index].stateName = "EMPTY";
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////