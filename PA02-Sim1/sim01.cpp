#include <stdio.h>
#include <string.h>
#include <queue>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <pthread.h>
#include <cstdlib>
#include <time.h>
#include <iomanip>

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
	createThread(PCB, numProcesses, configData);
	return 0;
}

//@function: reading in the config data/////////////////////////////
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
	//if we need to write our info to a file
	if((cnfData->logTo == "Log to Both") || (cnfData->logTo == "Log to File"))
	{
		//open the filepath for writing
		out.open((cnfData->logFilePath).c_str());
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

	//checking where to output the text
	if(cnfData->logTo == "Log to Both")
	{
		out << duration << " - Simulator program starting" << endl;
		cout << duration << " - Simulator program starting" << endl;
	}
	else if(cnfData->logTo == "Log to File")
	{
		out << duration << " - Simulator program starting" << endl;
	}
	else
	{
		cout << duration << " - Simulator program starting" << endl;
	}

	for(index = 0; index < numProcesses; index++)
	{	
		//if were at the start of the program
		if(mdfData[index].pName == 'S' && (strcmp(mdfData[index].state, "(start)") == 0))
		{
			//get the time it took to get here
			duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
			//checking where to log the info
			if(cnfData->logTo == "Log to Both")
			{
				out << duration << " - OS: preparing process 1" << endl;
				cout << duration << " - OS: preparing process 1" << endl;
			}
			else if(cnfData->logTo == "Log to File")
			{
				out << duration << " - OS: preparing process 1" << endl;
			}
			else
			{
				cout << duration << " - OS: preparing process 1" << endl;
			}
		}
		//if were at the start of the program
		else if(mdfData[index].pName == 'A' && (strcmp(mdfData[index].state, "(start)") == 0))
		{
			//get the time it took to get here
			duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
			//checking where to log the info
			if(cnfData->logTo == "Log to Both")
			{
				out << duration << " - OS: starting process 1" << endl;
				cout << duration << " - OS: starting process 1" << endl;
			}
			else if(cnfData->logTo == "Log to File")
			{
				out << duration << " - OS: starting process 1" << endl;
			}
			else
			{
				cout << duration << " - OS: starting process 1" << endl;
			}
		}
		//if were at the end of the program
		else if(mdfData[index].pName == 'A' && (strcmp(mdfData[index].state, "(end)") == 0))
		{
			//get the time it took to get here
			duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
			//checking where to log the info
			if(cnfData->logTo == "Log to Both")
			{
				out << duration << " - OS: removing process 1" << endl;
				out << duration << " - Simulator program ending" << endl;
				cout << duration << " - OS: removing process 1" << endl;
				cout << duration << " - Simulator program ending" << endl;

			}
			else if(cnfData->logTo == "Log to File")
			{
				out << duration << " - OS: removing process 1" << endl;
				out << duration << " - Simulator program ending" << endl;

			}
			else
			{
				cout << duration << " - OS: removing process 1" << endl;
				cout << duration << " - Simulator program ending" << endl;

			}
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
				if(cnfData->logTo == "Log to Both")
				{
					out << duration << " - Process 1: start " << mdfData[index].stateName << " input" << endl;
					cout << duration << " - Process 1: start " << mdfData[index].stateName << " input" << endl;
				}
				else if(cnfData->logTo == "Log to File")
				{
					out << duration << " - Process 1: start " << mdfData[index].stateName << " input" << endl;
				}
				else
				{
					cout << duration << " - Process 1: start " << mdfData[index].stateName << " input" << endl;
				}

				//THREADING PROCESS///////////////////////////////////////////////////////////////////////////
				//thread the process
				pthread_create(&t1, NULL, runIO, (void*)needle);
				//wait for it to die and join the threads
				pthread_join(t1, NULL);
				//////////////////////////////////////////////////////////////////////////////////////////////

				//log the end of the process data
				if(cnfData->logTo == "Log to Both")
				{
					out << duration << " - Process 1: end " << mdfData[index].stateName << " input" << endl;
					cout << duration << " - Process 1: end " << mdfData[index].stateName << " input" << endl;
				}
				else if(cnfData->logTo == "Log to File")
				{
					out << duration << " - Process 1: end " << mdfData[index].stateName << " input" << endl;
				}
				else
				{
					cout << duration << " - Process 1: end " << mdfData[index].stateName << " input" << endl;
				}

			}
			//if we are at an 'O'
			else if(mdfData[index].pName == 'O')
			{
				//check where to log the data
				if(cnfData->logTo == "Log to Both")
				{
					out << duration << " - Process 1: start " << mdfData[index].stateName << " output" << endl;
					cout << duration << " - Process 1: start " << mdfData[index].stateName << " output" << endl;
				}
				else if(cnfData->logTo == "Log to File")
				{
					out << duration << " - Process 1: start " << mdfData[index].stateName << " output" << endl;
				}
				else
				{
					cout << duration << " - Process 1: start " << mdfData[index].stateName << " output" << endl;
				}

				//THREADING PROCESS/////////////////////////////////////////////////////////////////////////
				pthread_create(&t1, NULL, runIO, (void*)needle);
				pthread_join(t1, NULL);
				///////////////////////////////////////////////////////////////////////////////////////////

				//log the end of the threading process
				if(cnfData->logTo == "Log to Both")
				{
					out << duration << " - Process 1: end " << mdfData[index].stateName << " output" << endl;
					cout << duration << " - Process 1: end " << mdfData[index].stateName << " output" << endl;
				}
				else if(cnfData->logTo == "Log to File")
				{
					out << duration << " - Process 1: end " << mdfData[index].stateName << " output" << endl;
				}
				else
				{
					cout << duration << " - Process 1: end " << mdfData[index].stateName << " output" << endl;
				}
			}
			else //if(mdfData[i].pName == 'P')
			{
				if(cnfData->logTo == "Log to Both")
				{
					out << duration << " - Process 1: start " << mdfData[index].stateName << " action" << endl;
					cout << duration << " - Process 1: start " << mdfData[index].stateName << " action" << endl;
				}
				else if(cnfData->logTo == "Log to File")
				{
					out << duration << " - Process 1: start " << mdfData[index].stateName << " action" << endl;
				}
				else
				{
					cout << duration << " - Process 1: start " << mdfData[index].stateName << " action" << endl;
				}

				//timestamp the amount of time it took us to get here
				duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;

				//log the end of the process
				if(cnfData->logTo == "Log to Both")
				{
					out << duration << " - Process 1: end " << mdfData[index].stateName << " action" << endl;
					cout << duration << " - Process 1: end " << mdfData[index].stateName << " action" << endl;
				}
				else if(cnfData->logTo == "Log to File")
				{
					out << duration << " - Process 1: end " << mdfData[index].stateName << " action" << endl;
				}
				else
				{
					cout << duration << " - Process 1: end " << mdfData[index].stateName << " action" << endl;
				}
			}
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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