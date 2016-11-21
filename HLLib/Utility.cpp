/*
 * HLLib
 * Copyright (C) 2006-2012 Ryan Gregg
 * Copyright (C) 2014 Mathias Panzenböck
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include "Utility.h"

hlBool HLLib::GetFileExists(const hlChar *lpPath)
{
#ifdef _WIN32
	WIN32_FIND_DATA FindData;
	HANDLE Handle = FindFirstFile(lpPath, &FindData);

	if(Handle == INVALID_HANDLE_VALUE)
	{
		return hlFalse;
	}
	else
	{
		FindClose(Handle);

		return (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
	}
#else
	struct stat Stat;

	if(stat(lpPath, &Stat) < 0)
	{
		return hlFalse;
	}

	return S_ISREG(Stat.st_mode) != 0;
#endif
}

hlBool HLLib::GetFolderExists(const hlChar *lpPath)
{
#ifdef _WIN32
	WIN32_FIND_DATA FindData;
	HANDLE Handle = FindFirstFile(lpPath, &FindData);

	if(Handle == INVALID_HANDLE_VALUE)
	{
		return hlFalse;
	}
	else
	{
		FindClose(Handle);

		return (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
#else
	struct stat Stat;

	if(stat(lpPath, &Stat) < 0)
	{
		return hlFalse;
	}

	return S_ISDIR(Stat.st_mode) != 0;
#endif
}

hlBool HLLib::GetFileSize(const hlChar *lpPath, hlUInt &uiFileSize)
{
	uiFileSize = 0;

#ifdef _WIN32
	WIN32_FIND_DATA FindData;
	HANDLE Handle = FindFirstFile(lpPath, &FindData);

	if(Handle != INVALID_HANDLE_VALUE)
	{
		FindClose(Handle);

		if((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			uiFileSize = (hlUInt)FindData.nFileSizeLow;
			return hlTrue;
		}
	}
#else
	struct stat Stat;

	if(stat(lpPath, &Stat) >= 0 && S_ISREG(Stat.st_mode) != 0 && Stat.st_size >= 0)
	{
		// FIXME: this can overflow!
		// st_size is signed 64bit value on 64bit Linux and also on 32bit Linux if
		// compiled with -D_FILE_OFFSET_BITS=64 (needed to support files > 2 GB)
		uiFileSize = (hlUInt)Stat.st_size;
		return hlTrue;
	}
#endif

	return hlFalse;
}

hlBool HLLib::CreateFolder(const hlChar *lpPath)
{
#ifdef _WIN32
	return CreateDirectory(lpPath, 0) != FALSE || GetLastError() == ERROR_ALREADY_EXISTS;
#else
	return mkdir(lpPath, 0755) >= 0 || errno == EEXIST;
#endif
}

hlVoid HLLib::FixupIllegalCharacters(hlChar *lpName)
{
	while(*lpName)
	{
#ifdef _WIN32
		if(*lpName == '/')
		{
			*lpName = '\\';
		}
#else
		if(*lpName == '\\')
		{
			*lpName = '/';
		}
#endif
		lpName++;
	}
}

hlVoid HLLib::RemoveIllegalCharacters(hlChar *lpName)
{
	hlUInt uiLength = (hlUInt)strlen(lpName);
	hlChar lpIllegalCharacters[] = { '/', '\\', '?', '<', '>', ':', '*', '|', '"', '\0' };

	for(hlUInt i = 0; i < uiLength;)
	{
		hlBool bFound = hlFalse;

		for(hlUInt j = 0; lpIllegalCharacters[j]; j++)
		{
			if(lpName[i] == lpIllegalCharacters[j])
			{
				// Remove the illegal character.
				for(hlUInt k = i; k < uiLength; k++)
				{
					lpName[k] = lpName[k + 1];
				}

				uiLength--;
				bFound = hlTrue;
				break;
			}
		}

		if(!bFound)
		{
			i++;
		}
	}
}

hlChar HLLib::NibbleToChar(hlByte uiNibble)
{
	uiNibble &= 0x0F;
	return uiNibble <= 9 ? '0' + static_cast<hlChar>(uiNibble) : 'A' + static_cast<hlChar>(uiNibble) - ('9' - '0' + 1);
}

hlUInt HLLib::BufferToHexString(const hlByte *lpBuffer, hlUInt uiBufferSize, hlChar* lpString, hlUInt uiStringSize)
{
	hlUInt uiCharsWritten = (uiStringSize + 1) / 2;
	for(hlUInt i = 0; i < uiBufferSize && uiStringSize > 2; i++)
	{
		*lpString++ = NibbleToChar(lpBuffer[i] >> 4);
		uiStringSize--;

		*lpString++ = NibbleToChar(lpBuffer[i]);
		uiStringSize--;
	}
	if(uiStringSize > 0)
	{
		*lpString = '\0';
	}
	return uiCharsWritten;
}

hlUInt HLLib::WStringToString(const hlWChar *lpSource, hlChar* lpDest, hlUInt uiDestSize)
{
#ifdef _WIN32
	int iResult = WideCharToMultiByte(CP_ACP, 0, lpSource, -1, lpDest, static_cast<int>(uiDestSize), NULL, NULL);
	if(iResult > 0)
	{
		return static_cast<hlUInt>(iResult);
	}
	else if(uiDestSize > 0)
	{
		*lpDest = '\0';
		return 1;
	}
	return 0;
#else
	hlUInt uiCharsWritten = 0;
	while(*lpSource != L'\0' && uiDestSize > 1)
	{
		if(*lpSource >= L' ' && *lpSource <= L'~')
		{
			*lpDest++ = static_cast<hlChar>(*lpSource++);
			uiDestSize--;
			uiCharsWritten++;
		}
	}
	if(uiDestSize > 0)
	{
		*lpDest = '\0';
		uiCharsWritten++;
	}
	return uiCharsWritten;
#endif
}

hlChar *HLLib::StringCopy(const hlChar *lpSource) {
	size_t size = strlen(lpSource) + 1;
	hlChar *lpCopy = new hlChar[size];
	strlcpy(lpCopy, lpSource, size);
	return lpCopy;
}

hlChar *HLLib::StringJoin(const hlChar *lpFirst, ...) {
	hlChar *lpJoined = 0;

	if (lpFirst != 0) {
		va_list ap;
		size_t size = strlen(lpFirst) + 1;

		// sum up buffer size
		va_start(ap, lpFirst);
		for (;;) {
			const hlChar *lpStr = va_arg(ap, const hlChar *);

			if (lpStr == 0)
				break;

			size += strlen(lpStr);
		}
		va_end(ap);

		// create buffer and copy strings to it
		lpJoined = new hlChar[size];
		strlcpy(lpJoined, lpFirst, size);

		va_start(ap, lpFirst);
		for (;;) {
			const hlChar *lpStr = va_arg(ap, const hlChar *);

			if (lpStr == 0)
				break;

			strlcat(lpJoined, lpStr, size);
		}
		va_end(ap);
	}
	else {
		lpJoined = new hlChar[1];
		lpJoined[0] = 0;
	}

	return lpJoined;
}
