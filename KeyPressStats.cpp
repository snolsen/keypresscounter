#include "KeyPressStats.h"

#include <fstream>
#include <strsafe.h>

KeyPressStats::KeyPressStats(LPCWSTR _fileName)
{
	fileName = _fileName;
	loadStats();
}

KeyPressStats::KeyPressStats()
{
	fileName = L"default.dat";
	loadStats();
}

KeyPressStats::~KeyPressStats()
{
	saveStats();
	printReport();
	getTotalKeys();
	getLast();
}

void KeyPressStats::regKeypress(USHORT vKey)
{	
	++keyStats[vKey];		// Increase count in keyStats map (if nonexisting, keyStats[vKey] is zero-initialized and the incremented as per std::map spec
	lastKey = vKey;
}

void KeyPressStats::printReport()
{
	std::wofstream oFile(L"report.txt");

	if (!oFile.is_open())
		OutputDebugString(L"printReport() could not open file for writing!");
	else
	{
		WCHAR szOutput[1024];
		for (const auto &v : keyStats)
		{
			StringCchPrintf(szOutput, sizeof szOutput, L"VKey: %04d \t Count: %9d\n", v.first, v.second);
#ifdef _DEBUG
			OutputDebugString(szOutput);
#endif // _DEBUG

			if (oFile.is_open())
				oFile << szOutput;
		}		
		StringCchPrintf(szOutput, sizeof szOutput, L"\n\nTotal no. of keystrokes: %d\n", getTotalKeys());
		oFile << szOutput;
		StringCchPrintf(szOutput, sizeof szOutput, L"Mouse clicks: %d (left) %d (right)\n", getTotalLMB(), getTotalRMB());
		oFile << szOutput;
		StringCchPrintf(szOutput, sizeof szOutput, L"VKey of last keystroke: %#04x (%03d)\n", getLast(), getLast());
		oFile << szOutput;
		oFile.close();
	}
}

void KeyPressStats::saveStats()
{
	std::ofstream oFile(fileName.c_str(), std::ios::out | std::ios::binary);
	if (!oFile.is_open())
		OutputDebugString(L"saveStats() could not open file for writing!");
	else {
		for (const auto &v : keyStats)
		{
			oFile.write((char*)&v.first, sizeof v.first);
			oFile.write((char*)&v.second, sizeof v.second);
		}
		oFile.close();
	}
}

void KeyPressStats::loadStats()
{
	USHORT vKey;
	ULONG count;

	std::ifstream iFile(fileName.c_str(), std::ios::in | std::ios::binary);
	if (!iFile.is_open())
		OutputDebugString(L"loadStats() could not open file for reading!");
	else {
		while (iFile.good())
		{
			iFile.read((char*)&vKey, sizeof vKey);
			iFile.read((char*)&count, sizeof count);
			if (iFile.good())
				keyStats[vKey] = count;			// Load data from file into keyStats object in stack.
		}
		iFile.close();
	}
}

ULONG KeyPressStats::getTotalKeys()
{	
	ULONG sum = 0;
	for (const auto &it : keyStats)
	{ 
		if((it.first != VK_LBUTTON) && (it.first != VK_RBUTTON) )
			sum += it.second;
	}
#ifdef _DEBUG
	WCHAR szOutput[1024];
	StringCchPrintf(szOutput, sizeof szOutput, L"Total no. of keystrokes: %d\n", sum);
	OutputDebugString(szOutput);
#endif // _DEBUG

	
	return sum;
}

ULONG KeyPressStats::getTotalLMB()
{
	return keyStats[VK_LBUTTON];
}

ULONG KeyPressStats::getTotalRMB()
{
	return keyStats[VK_RBUTTON];
}

USHORT KeyPressStats::getLast()
{
#ifdef _DEBUG
	WCHAR szOutput[1024];
	StringCchPrintf(szOutput, sizeof szOutput, L"VKey of last keystroke: %04d\n", lastKey);
	OutputDebugString(szOutput);
#endif // _DEBUG
	
	return lastKey;
}
