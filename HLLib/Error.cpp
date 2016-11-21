/*
 * HLLib
 * Copyright (C) 2006-2010 Ryan Gregg
 * Copyright (C) 2014 Mathias Panzenböck
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include "Error.h"

using namespace HLLib;

CError::CError()
{
	*this->lpError = '\0';
	this->uiSystemError = 0;
	*this->lpSystemError = '\0';
	*this->lpShortFormattedError = '\0';
	*this->lpLongFormattedError = '\0';
}

CError::~CError()
{

}

hlVoid CError::Clear()
{

}

const hlChar *CError::GetErrorMessage() const
{
	return this->lpError;
}

hlUInt CError::GetSystemError() const
{
	return this->uiSystemError;
}

const hlChar *CError::GetSystemErrorMessage() const
{
	return this->lpSystemError;
}

const hlChar *CError::GetShortFormattedErrorMessage()
{
	if(this->uiSystemError == 0)
	{
		if(*this->lpError)
		{
			snprintf(this->lpShortFormattedError, ERROR_STRING_SIZE, "Error: %s", this->lpError);
		}
		else
		{
			strlcpy(this->lpShortFormattedError, "<No error reported.>", ERROR_STRING_SIZE);
		}
	}
	else
	{
		snprintf(this->lpShortFormattedError, ERROR_STRING_SIZE, "Error (0x%.8x): %s %s", this->uiSystemError, this->lpError, this->lpSystemError);
	}

	return this->lpShortFormattedError;
}

const hlChar *CError::GetLongFormattedErrorMessage()
{
	if(this->uiSystemError == 0)
	{
		if(*this->lpError)
		{
			snprintf(this->lpLongFormattedError, ERROR_STRING_SIZE, "Error:\n%s", this->lpError);
		}
		else
		{
			strlcpy(this->lpLongFormattedError, "<No error reported.>", ERROR_STRING_SIZE);
		}
	}
	else
	{
		snprintf(this->lpLongFormattedError, ERROR_STRING_SIZE, "Error:\n%s\n\nSystem Error (0x%.8x):\n%s", this->lpError, this->uiSystemError, this->lpSystemError);
	}

	return this->lpLongFormattedError;
}

hlVoid CError::SetErrorMessage(const hlChar *lpError)
{
	this->SetErrorMessageFormated("%s", lpError);
}

hlVoid CError::SetErrorMessageFormated(const hlChar *lpFormat, ...)
{
	va_list ArgumentList;
	va_start(ArgumentList, lpFormat);
	vsnprintf(this->lpError, ERROR_STRING_SIZE, lpFormat, ArgumentList);
	va_end(ArgumentList);

	this->uiSystemError = 0;
	*this->lpSystemError = '\0';
}

hlVoid CError::SetSystemErrorMessage(const hlChar *lpError)
{
	this->SetSystemErrorMessageFormated("%s", lpError);
}

hlVoid CError::SetSystemErrorMessageFormated(const hlChar *lpFormat, ...)
{
	va_list ArgumentList;
	va_start(ArgumentList, lpFormat);
	vsnprintf(this->lpError, ERROR_STRING_SIZE, lpFormat, ArgumentList);
	va_end(ArgumentList);

#ifdef _WIN32
	this->uiSystemError = GetLastError();

	LPVOID lpMessage;

	if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, this->uiSystemError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMessage, 0, NULL))
	{
		strlcpy(this->lpSystemError, (hlChar *)lpMessage, ERROR_STRING_SIZE);

		LocalFree(lpMessage);
#else
	this->uiSystemError = (hlUInt)errno;

	hlChar *lpMessage = strerror(errno);

	if(lpMessage != 0)
	{
		strlcpy(this->lpSystemError, lpMessage, ERROR_STRING_SIZE);
#endif

		hlUInt uiLength = (hlUInt)strlen(this->lpSystemError);

		while(isspace(this->lpSystemError[uiLength - 1]))
		{
			uiLength--;
		}
		this->lpSystemError[uiLength] = '\0';
	}
	else
	{
		strlcpy(this->lpSystemError, "<Unable to retrieve system error message string.>", ERROR_STRING_SIZE);
	}
}
