#include "classes.h"

metaData::metaData()
{

}

metaData::~metaData()
{

}

void metaData::setLetter(char a)
{
	SAIPO_letter = a;
}

void metaData::setState(string a)
{
	state = a;
}

void metaData::setInitialTime(int a)
{
	initialTime = a;
}

void metaData::setCycleTime(float a)
{
	cycleTime = a;
}

void metaData::setProcessNum(int a)
{
	processNum = a;
}

char metaData::getLetter()
{
	return SAIPO_letter;
}

string metaData::getState()
{
	return state;
}

int metaData::getInitialTime()
{
	return initialTime;
}

float metaData::getCycleTime()
{
	return cycleTime;
}

int metaData::getProcessNum()
{
	return processNum;
}

 void metaData::readData(ifstream &fin)
 {
	fin >> SAIPO_letter;
	fin.ignore(512, leftOpenParenthesis);
	getline(fin, state, rightClosedParenthesis);
	if(state == "run")
	{
		state = "processing";
	}
	fin >> initialTime;
	fin.ignore(512, ';');
 }

void metaData::printData(float duration, int startOrEnd, int whereTo, ofstream &out, int &pNum)
{
	cout << fixed;
	cout << setprecision(6);
	if(SAIPO_letter == 'S')
	{
		if(whereTo == LTF || whereTo == LTB)
		{
			out << duration << " - Simulator program ending" << endl;
		}
		if(whereTo == LTM || whereTo == LTB)
		{
			cout << duration << " - Simulator program ending" << endl;
		}
		exit(0);
	}
	if(SAIPO_letter == 'A' && state == "start")
	{
		pNum++;
		if(whereTo == LTF || whereTo == LTB)
		{
			out << duration << " - OS: starting process " << pNum << endl;
		}
		if(whereTo == LTM || whereTo == LTB)
		{
			cout << duration << " - OS: starting process " << pNum << endl;
		}
	}
	if(SAIPO_letter == 'I')
	{
		if(startOrEnd == 0)
		{
			cout << duration << " - Process " << pNum << ": start " << state << " input" << endl;
		}
		else
		{
			cout << duration << " - Process " << pNum << ": end " << state << " input" << endl;
		}
	}
	if(SAIPO_letter == 'P')
	{
		if(startOrEnd == 0)
		{
			cout << duration << " - Process " << pNum << ": start " << state << " action" << endl;
		}
		else
		{
			cout << duration << " - Process " << pNum << ": end " << state << " action" << endl;
		}
	}
	if(SAIPO_letter == 'O')
	{
		if(startOrEnd == 0)
		{
			cout << duration << " - Process " << pNum << ": start " << state << " output" << endl;
		}
		else
		{
			cout << duration << " - Process " << pNum << ": end " << state << " output" << endl;
		}
	}
}

process::process()
{
	processID = -1;
}

process::~process()
{
	actionQueue.clear();
	processID = -1;
}

void process::setProcessID(int num)
{
	processID = num;
}

int process::getProcessID()
{
	return processID;
}

void process::copyActionQueue(deque<metaData> &rhs)
{
	metaData* tmp;
	unsigned int index = 0;
	while(index != rhs.size()-1)
	{
		tmp = new metaData();
		tmp->setLetter(rhs[index].getLetter());
		tmp->setState(rhs[index].getState());
		tmp->setInitialTime(rhs[index].getInitialTime());
		tmp->setCycleTime(rhs[index].getCycleTime());
		tmp->setProcessNum(rhs[index].getProcessNum());
		actionQueue.push_back(*tmp);
		delete tmp;
		index++;
	}
}


int configData::whereToLog()
{
	if(logTo == "Log to Both")
	{
		return LTB;
	}
	else if(logTo == "Log to Monitor")
	{
		return LTM;
	}
	else if(logTo == "Log to File")
	{
		return LTF;
	}
	else
	{
		return 0;
	}
}
