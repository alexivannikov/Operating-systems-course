#include "pch.h"
#include <iostream>
#include <cmath>
#include <windows.h>
#include <process.h>
#include <mutex>

#define READERS 10
#define WRITERS 3

using namespace std;

HANDLE canRead;
HANDLE canWrite;
HANDLE MUTEX;

LONG val = 0;
LONG maxVal = 10; 
LONG stop = 0;

LONG readers = 0; 
 
LONG queueReaders = 0; 
LONG queueWriters = 0; 

BOOL writeLock = false;

void startRead(int);
void stopRead();
unsigned __stdcall read(PVOID);

void startWrite(int);
void stopWrite();
unsigned __stdcall write(PVOID); 

LONG Show(LONG&);

int main() {
	canRead = CreateEvent(NULL, TRUE, FALSE, NULL);
	canWrite = CreateEvent(NULL, FALSE, TRUE, NULL);
	MUTEX = CreateMutex(NULL, FALSE, NULL);

	for (int j = 0; j < READERS; j++) {
		_beginthreadex(NULL, 0, read, (PVOID)j, 0, NULL);
	}

	for (int i = 0; i < WRITERS; i++) {
		_beginthreadex(NULL, 0, write, (PVOID)i, 0, NULL);
	}

	getchar();

	return 0;
}


unsigned __stdcall read(PVOID Id) {
	int id = (int)Id;
	
	while (stop!=1) {
		startRead(id);
		Sleep(2000);
		printf("%d\n\n", Show(val));
		stopRead();
	}

	return 0;
}

void startRead(int id) {
	InterlockedIncrement(&queueReaders);

	if (writeLock == true || queueWriters > 0) {
		ResetEvent(canRead);
	}

	WaitForSingleObject(canRead, INFINITE);
	InterlockedIncrement(&readers);
	InterlockedDecrement(&queueReaders);
	SetEvent(canRead);
}

void stopRead() {
	InterlockedDecrement(&readers);

	if (readers == 0) {
		SetEvent(canWrite);
	}
}

unsigned __stdcall write(PVOID Id) {
	int id = (int)Id;

	while (stop!=1) { 
		startWrite(id);

		if (val < maxVal) {
			++val;
			cout << "Writer " << id << "\n\nWritten value: " << Show(val) << "\n" << endl;
			cout << "Readers are reading...\n" << endl;
		}
		else { 
			WaitForSingleObject(MUTEX, INFINITE);
			stop = 1;
			ReleaseMutex(MUTEX);
		}

		stopWrite();
	}

	return 0;
}

void startWrite(int id) {
	InterlockedIncrement(&queueWriters);

	if (readers > 0 || writeLock == true) { 
		ResetEvent(canWrite);
	}

	WaitForSingleObject(canWrite, INFINITE);
	writeLock = true;
	InterlockedDecrement(&queueWriters); 
}

void stopWrite() {
	writeLock = false;

	if (queueReaders > 0)
		SetEvent(canRead);
	else
		SetEvent(canWrite);
}


LONG Show(LONG& val) {

	return val;
}

