#include "classes.h"

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

void metaData::printData(float duration, int startOrEnd, int whereTo, ofstream &out)
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
	if(SAIPO_letter == 'I')
	{
		if(startOrEnd == 0)
		{
			cout << duration << " - Process 1: start " << state << " input" << endl;
		}
		else
		{
			cout << duration << " - Process 1: end " << state << " input" << endl;
		}
	}
	if(SAIPO_letter == 'P')
	{
		if(startOrEnd == 0)
		{
			cout << duration << " - Process 1: start " << state << " action" << endl;
		}
		else
		{
			cout << duration << " - Process 1: end " << state << " action" << endl;
		}
	}
	if(SAIPO_letter == 'O')
	{
		if(startOrEnd == 0)
		{
			cout << duration << " - Process 1: start " << state << " output" << endl;
		}
		else
		{
			cout << duration << " - Process 1: end " << state << " output" << endl;
		}
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