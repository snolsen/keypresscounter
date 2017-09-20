#pragma once

#include <Windows.h>
#include <string>
#include <map>

class KeyPressStats
{
public:
	KeyPressStats();
	KeyPressStats(LPCWSTR _fileName);
	~KeyPressStats();

	void regKeypress(USHORT vKey);
	void printReport();
	void saveStats();
	void loadStats();
	ULONG getTotalKeys();
	ULONG getTotalLMB();
	ULONG getTotalRMB();
	USHORT getLast();
private:
	std::wstring fileName;
	std::map<USHORT, ULONG> keyStats;
	USHORT lastKey;
};
