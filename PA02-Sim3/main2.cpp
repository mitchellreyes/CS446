#include <iostream>
#include <queue>
#include <deque>
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
void runProcesses(deque<process> &PCB_queue, ofstream &out, configData cnfData, int numberOfActions);
void printNode(metaData mdfData);
float calcCycleTime(int initialTime, string state, configData cnfData);
void get_mdfData(configData cnfData, deque<process> &PCB_queue, int &numberOfActions);
void getConfigData(const char* fileName, configData &cnfData);
void runQuantum(float quantum);
int findParentProcess(metaData child);
deque<process> copyProcess(deque<process> rhs);
metaData copyAction(metaData mdfData);

deque<metaData> threadTimeFinishedQueue;
deque<process> blockedQueue;
bool quantumRunning = false;

int main(int argc, char *argv[])
{
	deque<process> PCB_queue;
	configData cnfData;
	int whereToLog;
	ofstream out;
	int numberOfActions = 0;
	// float quantum;

	getConfigData(argv[1], cnfData);
	whereToLog = cnfData.whereToLog();
	// quantum = cnfData.getQuantum();

	if(whereToLog == LTB || whereToLog == LTF)
	{
		out.open((cnfData.logFilePath).c_str());
	}
	get_mdfData(cnfData, PCB_queue, numberOfActions);
	runProcesses(PCB_queue, out, cnfData, numberOfActions);
	return 0;
}

void* runIO(void* mdfData)
{
	metaData* sendOut = new metaData();
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
	*sendOut = copyAction(*myData);
	threadTimeFinishedQueue.push_back(*sendOut);
	delete sendOut;
	return NULL;
}

void runQuantum(float quantum)
{
	//timestamp the duration it came in with
	float timestamp = duration;
	//stamp the duration again
	quantumRunning = true;
	duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
	float stopTime = quantum;
	while((duration - timestamp) < stopTime)
	{
		duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
	}
	//hold the duration for printing
	duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
	quantumRunning = false;
}

process copyProcess(process rhs)
{
	process* copiedPCB = new process();
	copiedPCB->actionQueue = rhs.actionQueue;
	copiedPCB->setProcessID(rhs.getProcessID());
	return *copiedPCB;
}

metaData copyAction(metaData mdfData)
{
	metaData copy;
	copy.setLetter(mdfData.getLetter());
	copy.setState(mdfData.getState());
	copy.setInitialTime(mdfData.getInitialTime());
	copy.setCycleTime(mdfData.getCycleTime());
	copy.setProcessNum(mdfData.getProcessNum());

	return copy;
}

int findParentProcess(metaData child)
{
	unsigned int index;
	for(index = 0; index < blockedQueue.size(); index++)
	{
		if(blockedQueue[index].getProcessID() == child.getProcessNum())
		{
			return index;
		}
	}
	return -1;
}



void runProcesses(deque<process> &PCB_queue, ofstream &out, configData cnfData, int numberOfActions)
{
	pthread_t threads[numberOfActions];
	int threadCount = 0;
	metaData* needle;
	int parentIndex;
	process* placeInPCB_queue;
	metaData* current_action;
	process copied_current_process;
	cout << setprecision(6) << fixed;
	runtime = clock();
	duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;

	current_action = new metaData();
	*current_action = copyAction(PCB_queue.front().actionQueue.front());
	PCB_queue.front().actionQueue.pop_front();
	copied_current_process = copyProcess(PCB_queue.front());
	PCB_queue.pop_front();

	while(!PCB_queue.empty())
	{
		printNode(*current_action);

		if(current_action->getLetter() == 'S' && current_action->getState() == "start")
		{

			cout << duration << " - OS: preparing all processes" << endl;
			duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
			cout << duration << " - OS: selecting next process" << endl;

		}
		else if(current_action->getLetter() == 'S' && current_action->getState() == "end")
		{
			if(blockedQueue.empty() && threadTimeFinishedQueue.empty() && PCB_queue.empty() && (quantumRunning == false))
			{
				cout << duration << " - Simulator program ending" << endl;
				exit(0);
			}
			else
			{
				copied_current_process.actionQueue.push_back(*current_action);
				PCB_queue.push_back(copied_current_process);
				delete current_action;
			}
		}
		else if(current_action->getLetter() == 'A' && current_action->getState() == "start")
		{
			duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
			cout << duration << " - OS: starting process " << current_action->getProcessNum() << endl;
		} 
		//If P
		else if(current_action->getLetter() == 'P')
		{
			cout << "made it to P" << endl;
			if(current_action->getCycleTime() > cnfData.quantumTime)
			{
				//run quantum
				runQuantum(cnfData.quantumTime);
				//subtract runtime from quantum time
				current_action->setCycleTime(current_action->getCycleTime() - cnfData.quantumTime);
				//set new time as runtime
				//push action back on the queue (push_front)
				copied_current_process.actionQueue.push_front(*current_action);
				PCB_queue.push_back(copied_current_process);
				delete current_action;
			}
			else
			{
				//run quantum
				runQuantum(cnfData.quantumTime);
				//if process mdfQueue is NOT empty
				if(!copied_current_process.actionQueue.empty())
				{
					//push process back into the queue
					PCB_queue.push_back(copied_current_process);
				}
				delete current_action;
			}
			while(!threadTimeFinishedQueue.empty())
			{
				//print front()
				//find parent in blocked queue
				parentIndex = findParentProcess(threadTimeFinishedQueue.front());
				if(!blockedQueue[parentIndex].actionQueue.empty() 
					&& blockedQueue[parentIndex].actionQueue.front().getLetter() != 'A'
					&& blockedQueue[parentIndex].actionQueue.front().getState() != "end")
				{
					placeInPCB_queue = new process();
					*placeInPCB_queue = copyProcess(blockedQueue[parentIndex]);
					blockedQueue.erase(blockedQueue.begin()+(parentIndex));
					PCB_queue.push_back(*placeInPCB_queue);
					delete placeInPCB_queue;
				}
				else
				{
					blockedQueue.erase(blockedQueue.begin()+(parentIndex));
					//pop parent index from blocked queue
				}
				threadTimeFinishedQueue.pop_front();
				//pop action
			}
		}
		else if(current_action->getLetter() == 'I' || current_action->getLetter() == 'O')
		{
			cout << "made it to I/O" << endl;
			needle = &(*current_action);
			duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;
			//(readyQueue.front()).printData(duration, START, whereTo, out, numProcess);
			pthread_create(&threads[threadCount], NULL, runIO, (void*)needle);
			threadCount++;
			//push the process to a blocked queue (or array)
			blockedQueue.push_back(copied_current_process);
			delete current_action;
		}
		while(!threadTimeFinishedQueue.empty())
		{
			while(quantumRunning == true);
			//print front()
				//find parent in blocked queue
				parentIndex = findParentProcess(threadTimeFinishedQueue.front());
				if(!blockedQueue[parentIndex].actionQueue.empty() 
					&& blockedQueue[parentIndex].actionQueue.front().getLetter() != 'A'
					&& blockedQueue[parentIndex].actionQueue.front().getState() != "end")
				{
					placeInPCB_queue = new process();
					*placeInPCB_queue = copyProcess(blockedQueue[parentIndex]);
					blockedQueue.erase(blockedQueue.begin()+(parentIndex));
					PCB_queue.push_back(*placeInPCB_queue);
					delete placeInPCB_queue;
				}
				else
				{
					blockedQueue.erase(blockedQueue.begin()+(parentIndex));
					//pop parent index from blocked queue
				}
				threadTimeFinishedQueue.pop_front();
		}
		*current_action = copyAction(PCB_queue.front().actionQueue.front());
		PCB_queue.front().actionQueue.pop_front();
		copied_current_process = copyProcess(PCB_queue.front());
		PCB_queue.pop_front();
	}
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

void get_mdfData(configData cnfData, deque<process> &PCB_queue, int &numberOfActions)
{
    ifstream fin;
    fin.open((cnfData.filePath).c_str(), ifstream::in);
    int process_id = 0;

    if(!fin.good())
    {
		cout << "\tUNABLE TO OPEN: " << cnfData.filePath << endl << "\tEXITING PROGRAM" << endl;
        exit(1);
    }
    else
    {
		cout << "\tREADING DATA FROM: " << cnfData.filePath << endl;
		metaData* actionInfo;
		process* PCB;
		fin.ignore(512, ':');
		while(fin.good())
		{
			PCB = new process();
			do
			{
				actionInfo = new metaData();
				actionInfo->readData(fin);
				actionInfo->setCycleTime(calcCycleTime(actionInfo->getInitialTime(), actionInfo->getState(), cnfData));
				actionInfo->setProcessNum(process_id);
				PCB->actionQueue.push_back(*actionInfo);
				numberOfActions++;
			}while(actionInfo->getLetter() != 'S' && actionInfo->getState() != "end");
			PCB->setProcessID(process_id);
			PCB_queue.push_back(*PCB);
			process_id++;
		}
		cout << "\t\t SUCCESS" << endl << endl;
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
		cout << endl << "\tREADING DATA FROM: " << fileName << endl;
		//read in all the data to the different config struct components
		fin.ignore(512, ':');
		fin >> cnfData.phase;
		fin.ignore(512, ':');
		fin >> cnfData.filePath;
		fin.ignore(512, ':');
		fin >> cnfData.schedCode;
		fin.ignore(512, ':');
		fin >> cnfData.quantumTime;
		cnfData.quantumTime = cnfData.quantumTime / MSEC;
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
		cout << "\t\tSUCCESS" << endl << endl;

	}
}





//while PCB_queue is not empty OR blockedQueue is not empty

	//get the queue (A) from PCB_queue.front()

	//while A is not empty
		//get the action from A (A.front())

		//if 'S' and "start"
			//print stuff
		//if 'S' and "end"
			//check all the queues and stuff

		//if 'A' and "start"
			//print stuff
		//if 'A' and "end"
			//print stuff
			//pop from A

		//if 'P'
			//run quantum
			//check the cycle time < quantum time
				//if yes
					//subtract quantum time from cycle time 
				//if no
					//pop from A
			
			//copy PCB_queue.front()
			//PCB_queue.pop_front()
			//PCB_queue.push_back(copy)

		//if 'I/O'
			//create thread for the action
			//pop from A
			//copy PCB_queue.front()
			//PCB_queue.pop_front()
			//blockedQueue.push_back(copy)

		//check the thread timings if anything is done
	//end of inner while loop

	//if PCB_queue is empty && blockedQueue is NOT empty
		//print IDLE CPU
		//wait until something is in thread timings

//end of outer while loop
			
			
				
				
	