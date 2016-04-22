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
deque<process> copyProcess(deque<process> &rhs);
metaData copyAction(metaData &mdfData);

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
	pthread_exit(NULL);
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

process copyProcess(process &rhs)
{
	process copy;
	copy.setProcessID(rhs.getProcessID());
	copy.copyActionQueue(rhs.actionQueue);
	return copy;

}

metaData copyAction(metaData &mdfData)
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
		if((blockedQueue[index]).getProcessID() == child.getProcessNum())
		{
			return index;
		}
	}
	return -1;
}



void runProcesses(deque<process> &PCB_queue, ofstream &out, configData cnfData, int numberOfActions)
{
	pthread_t threads[5000];
	int threadCount = 0;
	metaData* needle;

	deque<metaData>* peekActionQ;
	metaData* peekAction;

	process* tempProcess;

	cout << setprecision(6) << fixed;
	runtime = clock();
	duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;


	//while PCB_queue is not empty OR blockedQueue is not empty
	while(!PCB_queue.empty() || !blockedQueue.empty())
	{
		//get the queue (A) from PCB_queue.front()
		peekActionQ = &(PCB_queue.front().actionQueue);
		//while A is not empty
		while(!peekActionQ->empty())
		{
			//get the action from A (A.front())
			peekAction = &(peekActionQ->front());
					printNode(*peekAction);

			//record how long it took for us to get here
			duration = ((float)(clock() - runtime)) / CLOCKS_PER_SEC;

			//if 'S' and "start"
			if(peekAction->getLetter() == 'S')
			{
				if(peekAction->getState() == "start")
				{
					//print stuff
					cout << duration << " - OS: preparing all processes" << endl;
					//pop from A
					PCB_queue.pop_front();
				}
				else //if(peekAction.getLetter() == 'S' && peekAction.getState() == "end")
				{
					//check all the queues and quantum to see if the program is done
					if(blockedQueue.empty() && ((PCB_queue.size()-1) <= 0))
					{
						cout << duration << " - Simulator program ending" << endl;
						exit(0);
					}
					else
					{
						//if not, push the end process block back on to the PCB_queue
						tempProcess = new process();
						*tempProcess = copyProcess(PCB_queue.front());
						PCB_queue.push_back(*tempProcess);
						PCB_queue.pop_front();
						delete tempProcess;
					}
				}
			}
			else if(peekAction->getLetter() == 'A')
			{
				if(peekAction->getState() == "start")
				{
					//print stuff
					cout << duration << " - OS: starting process " << PCB_queue.front().getProcessID() << endl;
					peekActionQ->pop_front();
				}
				else //if 'A' and "end"
				{
					//print stuff
					cout << duration << " - OS: removing process " << PCB_queue.front().getProcessID() << endl;
					PCB_queue.pop_front();
				}
				//pop from A

			}
			else if(peekAction->getLetter() == 'P')
			{
				//run quantum
				runQuantum(cnfData.quantumTime);
				//check the cycle time < quantum time
				if(peekAction->getCycleTime() > cnfData.quantumTime)
				{
					//if yes
					cout << duration << " - OS: quantum time out" << endl;
					peekAction->setCycleTime((peekAction->getCycleTime()) - (cnfData.quantumTime));
				}
				else
				{
					peekActionQ->pop_front();
				}
				
				//copy PCB_queue.front()
				//PCB_queue.push_back(copy)
				tempProcess = new process();
				*tempProcess = copyProcess(PCB_queue.front());
				PCB_queue.push_back(*tempProcess);
				PCB_queue.pop_front();
				delete tempProcess;
			}
			else if(peekAction->getLetter() == 'I' || peekAction->getLetter() == 'O')
			{
				//create thread for the action
				needle = &(*peekAction);
				pthread_create(&threads[threadCount], NULL, runIO, (void*)needle);
				threadCount++;
				//pop from A
				peekActionQ->pop_front();
				//copy PCB_queue.front()
				tempProcess = new process();
				*tempProcess = copyProcess(PCB_queue.front());
				//blockedQueue.push_back(copy)
				blockedQueue.push_back(*tempProcess);
				PCB_queue.pop_front();
				delete tempProcess;
			}

			// //check the thread timings if anything is done
			// while(!threadTimeFinishedQueue.empty())
			// {
			// 	//if something is done
			// 	//find the parent in the blocked queue
			// 	int parentIndex = findParentProcess(threadTimeFinishedQueue.front());
			// 	if(parentIndex == -1)
			// 	{
			// 		cout << "An error has occured with finding a thread's process" << endl;
			// 		exit(1);
			// 	}
			// 	tempProcess = new process();
			// 	*tempProcess = copyProcess(blockedQueue[parentIndex]);
			// 	//remove it from the blocked queue
			// 	blockedQueue.erase(blockedQueue.begin() + parentIndex);
			// 	//return the process back into the PCB_queue
			// 	PCB_queue.push_back(*tempProcess);
			// 	PCB_queue.pop_front();
			// 	delete tempProcess;
			// }
		}//end of inner while loop

		//pop from PCB_queue;
		//PCB_queue.pop_front();

		//if PCB_queue is empty && blockedQueue is NOT empty
		// if(PCB_queue.empty() && !blockedQueue.empty())
		// {
		// 	//print IDLE CPU
		// 	cout << "IDLE CPU" << endl;
		// 	//wait until something is in thread timings
		// 	//while(threadTimeFinishedQueue.empty());
		// 	while(!threadTimeFinishedQueue.empty())
		// 	{
		// 		//if something is done
		// 		//find the parent in the blocked queue
		// 		int parentIndex = findParentProcess(threadTimeFinishedQueue.front());
		// 		if(parentIndex == -1)
		// 		{
		// 			cout << "An error has occured with finding a thread's process" << endl;
		// 			exit(1);
		// 		}
		// 		tempProcess = new process();
		// 		*tempProcess = copyProcess(blockedQueue[parentIndex]);
		// 		//remove it from the blocked queue
		// 		blockedQueue.erase(blockedQueue.begin() + parentIndex);
		// 		//return the process back into the PCB_queue
		// 		PCB_queue.push_back(*tempProcess);
		// 		delete tempProcess;
		// 	}
		// }

	}//end of outer while loop
}//end of function

void printNode(metaData mdfData)
{
	cout << "SAIPO_letter: " << mdfData.getLetter() << endl;
	cout << "\tState: " << mdfData.getState() << endl;
	cout << "\tInitial Time: " << mdfData.getInitialTime() << endl;
	cout << "\tCycle Time: " << mdfData.getCycleTime() << endl; 
	cout << "\tProcess Number: " << mdfData.getProcessNum() << endl << endl;
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
	