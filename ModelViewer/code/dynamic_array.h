#pragma once
#include <stdint.h>

template<typename T>
struct dynamic_array
{
	uint32_t MaxEntriesCount;
	uint32_t EntriesCount;
	T* Entries;

    T &operator[](int32_t Index);

    dynamic_array()
    {
		InitializeDynamicArray(this);
    }

	dynamic_array(uint32_t InitialMaxEntriesCount)
	{
		InitializeDynamicArray(this, InitialMaxEntriesCount);
	}

    ~dynamic_array()
    {
        MaxEntriesCount = 0;
        EntriesCount = 0;
        if(Entries)
        {
            free(Entries);
            Entries = 0;
        }
    }
};

template<typename T>
T &dynamic_array<T>::operator[](int32_t Index)
{
    return(Entries[Index]);
}

template<typename T>
void InitializeDynamicArray(dynamic_array<T> *Array, uint32_t InitialMaxEntriesCount = 0)
{
    Array->MaxEntriesCount = InitialMaxEntriesCount;
    Array->EntriesCount = 0;
    Array->Entries = (Array->MaxEntriesCount > 0) ? (T *)malloc(Array->MaxEntriesCount*sizeof(T)) : 0;
}

template<typename T>
void ExpandDynamicArray(dynamic_array<T> *Array, uint32_t ExactMaxEntriesCount = 0)
{
    uint32_t NewMaxEntriesCount = 1;
    if(ExactMaxEntriesCount)
    {
        NewMaxEntriesCount = ExactMaxEntriesCount;
    }
    else if(Array->MaxEntriesCount)
    {
        NewMaxEntriesCount = 2*Array->MaxEntriesCount;
    }

    if(NewMaxEntriesCount > Array->MaxEntriesCount)
    {
        T *NewMemory = static_cast<T *>(malloc(NewMaxEntriesCount*sizeof(T)));
        for(uint32_t EntryIndex = 0;
            EntryIndex < Array->EntriesCount;
            EntryIndex++)
        {
            NewMemory[EntryIndex] = Array->Entries[EntryIndex];
        }

        free(Array->Entries);
        Array->MaxEntriesCount = NewMaxEntriesCount;
        Array->Entries = NewMemory;
    }
}

template<typename T>
void PushEntry(dynamic_array<T> *Array, T Entry)
{
    if(!(Array->EntriesCount < Array->MaxEntriesCount))
    {
        ExpandDynamicArray(Array);
    }

    Array->Entries[Array->EntriesCount++] = Entry;
}

template<typename T>
void ResizeDynamicArray(dynamic_array<T> *Array, uint32_t NewEntriesCount)
{
    if(NewEntriesCount > Array->EntriesCount)
    {
        while(NewEntriesCount > Array->EntriesCount)
        {
			T Entry = {};
            PushEntry(Array, Entry);
        }
    }
    else if(NewEntriesCount < Array->EntriesCount)
    {
        Array->EntriesCount = NewEntriesCount;
    }
}

template<typename T>
void ReserveDynamicArray(dynamic_array<T> *Array, uint32_t NewMaxEntriesCount)
{
    if(NewMaxEntriesCount > Array->MaxEntriesCount)
    {
        ExpandDynamicArray(Array, NewMaxEntriesCount);
    }
}