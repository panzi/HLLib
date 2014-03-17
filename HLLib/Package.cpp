/*
 * HLLib
 * Copyright (C) 2006-2010 Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include "HLLib.h"
#include "Wrapper.h"
#include "Package.h"
#include "Packages.h"
#include "Mappings.h"
#include "Streams.h"

using namespace HLLib;

CPackage::CPackage() : bDeleteStream(hlFalse), bDeleteMapping(hlFalse), pStream(0), pMapping(0), pRoot(0), pStreams(0)
{

}

CPackage::~CPackage()
{
	assert(this->pStream == 0);
	assert(this->pMapping == 0);
	assert(this->pRoot == 0);
	assert(this->pStreams == 0);
}

hlBool CPackage::GetOpened() const
{
	return this->pMapping != 0;
}

hlBool CPackage::Open(Streams::IStream &Stream, hlUInt uiMode)
{
	return this->Open(&Stream, uiMode, hlFalse);
}

hlBool CPackage::Open(Mapping::CMapping &Mapping, hlUInt uiMode)
{
	return this->Open(&Mapping, uiMode, hlFalse);
}

hlBool CPackage::Open(const hlChar *lpFileName, hlUInt uiMode)
{
	if(uiMode & HL_MODE_NO_FILEMAPPING)
	{
		return this->Open(new Streams::CFileStream(lpFileName), uiMode, hlTrue);
	}
	else
	{
		return this->Open(new Mapping::CFileMapping(lpFileName), uiMode, hlTrue);
	}
}

hlBool CPackage::Open(hlVoid *lpData, hlUInt uiBufferSize, hlUInt uiMode)
{
	return this->Open(new Mapping::CMemoryMapping(lpData, uiBufferSize), uiMode, hlTrue);
}

hlBool CPackage::Open(hlVoid *pUserData, hlUInt uiMode)
{
	return this->Open(new Streams::CProcStream(pUserData), uiMode, hlTrue);
}

hlBool CPackage::Open(Streams::IStream *pStream, hlUInt uiMode, hlBool bDeleteStream)
{
	this->Close();

	this->pStream = pStream;
	this->bDeleteStream = bDeleteStream;

	this->bDeleteMapping = hlTrue;
	this->pMapping = new Mapping::CStreamMapping(*this->pStream);

	if(!this->pMapping->Open(uiMode))
	{
		this->Close();
		return hlFalse;
	}

	if(!this->MapDataStructures())
	{
		this->UnmapDataStructures();
		this->Close();
		return hlFalse;
	}

	this->pStreams = new CStreamList();

	return hlTrue;
}

hlBool CPackage::Open(Mapping::CMapping *pMapping, hlUInt uiMode, hlBool bDeleteMapping)
{
	this->Close();

	this->bDeleteMapping = bDeleteMapping;
	this->pMapping = pMapping;

	if(!this->pMapping->Open(uiMode))
	{
		this->Close();
		return hlFalse;
	}

	if(!this->MapDataStructures())
	{
		this->UnmapDataStructures();
		this->Close();
		return hlFalse;
	}

	this->pStreams = new CStreamList();

	return hlTrue;
}

hlVoid CPackage::Close()
{
	if(this->pStreams != 0)
	{
		for(CStreamList::iterator i = this->pStreams->begin(); i != this->pStreams->end(); ++i)
		{
			Streams::IStream *pStream = *i;

			pStream->Close();
			this->ReleaseStreamInternal(*pStream);
			delete pStream;
		}

		delete this->pStreams;
		this->pStreams = 0;
	}

	if(this->pMapping != 0)
	{
		this->UnmapDataStructures();

		this->pMapping->Close();
	}

	if(this->pRoot != 0)
	{
		this->ReleaseRoot();
		delete this->pRoot;
		this->pRoot = 0;
	}

	if(this->bDeleteMapping)
	{
		delete this->pMapping;
		this->bDeleteMapping = hlFalse;
	}
	this->pMapping = 0;

	if(this->bDeleteStream)
	{
		delete this->pStream;
		this->bDeleteStream = hlFalse;
	}
	this->pStream = 0;
}

hlBool CPackage::Defragment()
{
	if(!this->GetOpened())
	{
		LastError.SetErrorMessage("Package not opened.");
		return hlFalse;
	}

	if(!(this->GetMapping()->GetMode() & HL_MODE_WRITE))
	{
		LastError.SetErrorMessage("Package does not have write privileges, please enable them.");
		return hlFalse;
	}

	if(this->GetMapping()->GetMode() & HL_MODE_VOLATILE)
	{
		LastError.SetErrorMessage("Package has volatile access enabled, please disable it.");
		return hlFalse;
	}

	return this->DefragmentInternal();
}

hlBool CPackage::DefragmentInternal()
{
	return hlTrue;
}

const Mapping::CMapping* CPackage::GetMapping() const
{
	return this->pMapping;
}

CDirectoryFolder *CPackage::GetRoot()
{
	if(!this->GetOpened())
	{
		return 0;
	}

	if(this->pRoot == 0)
	{
		this->pRoot = this->CreateRoot();
		this->pRoot->Sort();
	}

	return this->pRoot;
}

const CDirectoryFolder *CPackage::GetRoot() const
{
	if(!this->GetOpened())
	{
		return 0;
	}

	return this->pRoot;
}

hlVoid CPackage::ReleaseRoot()
{

}

hlUInt CPackage::GetAttributeCount() const
{
	if(!this->GetOpened())
	{
		return 0;
	}

	return this->GetAttributeCountInternal();
}

const hlChar *CPackage::GetAttributeName(HLPackageAttribute eAttribute) const
{
	if(!this->GetOpened())
	{
		return 0;
	}

	return this->GetAttributeNameInternal(eAttribute);
}

hlBool CPackage::GetAttribute(HLPackageAttribute eAttribute, HLAttribute &Attribute) const
{
	Attribute.eAttributeType = HL_ATTRIBUTE_INVALID;

	if(!this->GetOpened())
	{
		//LastError.SetErrorMessage("Package not open.");
		return hlFalse;
	}

	return this->GetAttributeInternal(eAttribute, Attribute);
}

hlUInt CPackage::GetAttributeCountInternal() const
{
	return 0;
}

const hlChar *CPackage::GetAttributeNameInternal(HLPackageAttribute eAttribute) const
{
	(void)eAttribute;
	return 0;
}

hlBool CPackage::GetAttributeInternal(HLPackageAttribute eAttribute, HLAttribute &Attribute) const
{
	(void)eAttribute;
	(void)Attribute;
	return hlFalse;
}

hlUInt CPackage::GetItemAttributeCount() const
{
	if(!this->GetOpened())
	{
		return 0;
	}

	return this->GetItemAttributeCountInternal();
}

const hlChar *CPackage::GetItemAttributeName(HLPackageAttribute eAttribute) const
{
	if(!this->GetOpened())
	{
		return 0;
	}

	return this->GetItemAttributeNameInternal(eAttribute);
}

hlBool CPackage::GetItemAttribute(const CDirectoryItem *pItem, HLPackageAttribute eAttribute, HLAttribute &Attribute) const
{
	Attribute.eAttributeType = HL_ATTRIBUTE_INVALID;

	if(!this->GetOpened() || pItem == 0 || pItem->GetPackage() != this)
	{
		LastError.SetErrorMessage("Item does not belong to package.");
		return hlFalse;
	}

	return this->GetItemAttributeInternal(pItem, eAttribute, Attribute);
}

hlUInt CPackage::GetItemAttributeCountInternal() const
{
	return 0;
}

const hlChar *CPackage::GetItemAttributeNameInternal(HLPackageAttribute eAttribute) const
{
	(void)eAttribute;
	return 0;
}

hlBool CPackage::GetItemAttributeInternal(const CDirectoryItem *pItem, HLPackageAttribute eAttribute, HLAttribute &Attribute) const
{
	(void)pItem;
	(void)eAttribute;
	(void)Attribute;
	return hlFalse;
}

hlBool CPackage::GetFileExtractable(const CDirectoryFile *pFile, hlBool &bExtractable) const
{
	bExtractable = hlFalse;

	if(!this->GetOpened() || pFile == 0 || pFile->GetPackage() != this)
	{
		LastError.SetErrorMessage("File does not belong to package.");
		return hlFalse;
	}

	return this->GetFileExtractableInternal(pFile, bExtractable);
}

hlBool CPackage::GetFileExtractableInternal(const CDirectoryFile *pFile, hlBool &bExtractable) const
{
	(void)pFile;
	bExtractable = hlTrue;
	return hlTrue;
}

hlBool CPackage::GetFileValidation(const CDirectoryFile *pFile, HLValidation &eValidation) const
{
	eValidation = HL_VALIDATES_ASSUMED_OK;

	if(!this->GetOpened() || pFile == 0 || pFile->GetPackage() != this)
	{
		LastError.SetErrorMessage("File does not belong to package.");
		return hlFalse;
	}

	return this->GetFileValidationInternal(pFile, eValidation);
}

hlBool CPackage::GetFileValidationInternal(const CDirectoryFile *pFile, HLValidation &eValidation) const
{
	(void)pFile;
	eValidation = HL_VALIDATES_ASSUMED_OK;
	return hlTrue;
}

hlBool CPackage::GetFileSize(const CDirectoryFile *pFile, hlUInt &uiSize) const
{
	uiSize = 0;

	if(!this->GetOpened() || pFile == 0 || pFile->GetPackage() != this)
	{
		LastError.SetErrorMessage("File does not belong to package.");
		return hlFalse;
	}

	return this->GetFileSizeInternal(pFile, uiSize);
}

hlBool CPackage::GetFileSizeOnDisk(const CDirectoryFile *pFile, hlUInt &uiSize) const
{
	uiSize = 0;

	if(!this->GetOpened() || pFile == 0 || pFile->GetPackage() != this)
	{
		LastError.SetErrorMessage("File does not belong to package.");
		return hlFalse;
	}

	return this->GetFileSizeOnDiskInternal(pFile, uiSize);
}

hlBool CPackage::CreateStream(const CDirectoryFile *pFile, Streams::IStream *&pStream) const
{
	pStream = 0;

	if(!this->GetOpened() || pFile == 0 || pFile->GetPackage() != this)
	{
		LastError.SetErrorMessage("File does not belong to package.");
		return hlFalse;
	}

	if(!this->CreateStreamInternal(pFile, pStream))
	{
		return hlFalse;
	}

	this->pStreams->push_back(pStream);
	return hlTrue;
}

hlVoid CPackage::ReleaseStream(Streams::IStream *pStream) const
{
	if(!this->GetOpened())
	{
		return;
	}

	for(CStreamList::iterator i = this->pStreams->begin(); i != this->pStreams->end(); ++i)
	{
		if(*i == pStream)
		{
			pStream->Close();
			this->ReleaseStreamInternal(*pStream);
			delete pStream;

			this->pStreams->erase(i);
			break;
		}
	}
}

hlVoid CPackage::ReleaseStreamInternal(Streams::IStream &Stream) const
{
	(void)Stream;
}

static CPackage *PackageFromType(HLPackageType ePackageType) {
	switch (ePackageType) {
	case HL_PACKAGE_NONE:
		break;

	case HL_PACKAGE_BSP:
		return new CBSPFile();

	case HL_PACKAGE_GCF:
		return new CGCFFile();

	case HL_PACKAGE_PAK:
		return new CPAKFile();

	case HL_PACKAGE_VBSP:
		return new CVBSPFile();

	case HL_PACKAGE_WAD:
		return new CWADFile();

	case HL_PACKAGE_XZP:
		return new CXZPFile();

	case HL_PACKAGE_ZIP:
		return new CZIPFile();

	case HL_PACKAGE_NCF:
		return new CNCFFile();

	case HL_PACKAGE_VPK:
		return new CVPKFile();

	case HL_PACKAGE_SGA:
		return new CSGAFile();
	}

	return 0;
}

static CPackage *PackageFromStream(Streams::IStream *lpStream) {
	return PackageFromType(hlGetPackageTypeFromStream(lpStream));
}

static CPackage *PackageFromMemory(const hlVoid *lpBuffer, hlUInt uiBufferSize) {
	return PackageFromType(hlGetPackageTypeFromMemory(lpBuffer, uiBufferSize));
}

CPackage *CPackage::AutoOpen(Streams::IStream &Stream, hlUInt uiMode) {
	CPackage *lpPackage = PackageFromStream(&Stream);

	if (lpPackage != 0) {
		if (!lpPackage->Open(Stream, uiMode)) {
			delete lpPackage;
			lpPackage = 0;
		}
	}

	return lpPackage;
}

CPackage *CPackage::AutoOpen(Mapping::CMapping &Mapping, hlUInt uiMode) {
	Mapping::CView *lpView = 0;
	if (!Mapping.Map(lpView, 0, 512 > Mapping.GetMappingSize() ? Mapping.GetMappingSize() : 512)) {
		return 0;
	}
	CPackage *lpPackage = PackageFromMemory(lpView->GetView(), lpView->GetLength());
	Mapping.Unmap(lpView);

	if (lpPackage != 0) {
		if (!lpPackage->Open(Mapping, uiMode)) {
			delete lpPackage;
			lpPackage = 0;
		}
	}

	return lpPackage;
}

CPackage *CPackage::AutoOpen(const hlChar *lpFileName, hlUInt uiMode) {
	Streams::CFileStream *lpStream = new Streams::CFileStream(lpFileName);
	CPackage *lpPackage = PackageFromStream(lpStream);

	if (lpPackage != 0) {
		if (!lpPackage->Open(lpStream, uiMode, hlTrue)) {
			delete lpPackage;
			lpPackage = 0;
		}
	}
	else {
		delete lpStream;
	}

	return lpPackage;
}

CPackage *CPackage::AutoOpen(hlVoid *lpData, hlUInt uiBufferSize, hlUInt uiMode) {
	CPackage *lpPackage = PackageFromMemory(lpData, uiBufferSize);

	if (lpPackage != 0) {
		if (!lpPackage->Open(lpData, uiBufferSize, uiMode)) {
			delete lpPackage;
			lpPackage = 0;
		}
	}

	return lpPackage;
}

CPackage *CPackage::AutoOpen(hlVoid *pUserData, hlUInt uiMode) {
	Streams::CProcStream *lpStream = new Streams::CProcStream(pUserData);
	CPackage *lpPackage = PackageFromStream(lpStream);

	if (lpPackage != 0) {
		if (!lpPackage->Open(lpStream, uiMode, hlTrue)) {
			delete lpPackage;
			lpPackage = 0;
		}
	}
	else {
		delete lpStream;
	}

	return lpPackage;
}

CPackage *CPackage::Open(Streams::IStream &Stream, hlUInt uiMode, HLPackageType ePackageType) {
	CPackage *lpPackage = PackageFromType(ePackageType);

	if (lpPackage != 0) {
		if (!lpPackage->Open(Stream, uiMode)) {
			delete lpPackage;
			lpPackage = 0;
		}
	}

	return lpPackage;
}

CPackage *CPackage::Open(Mapping::CMapping &Mapping, hlUInt uiMode, HLPackageType ePackageType) {
	CPackage *lpPackage = PackageFromType(ePackageType);

	if (lpPackage != 0) {
		if (!lpPackage->Open(Mapping, uiMode)) {
			delete lpPackage;
			lpPackage = 0;
		}
	}

	return lpPackage;
}

CPackage *CPackage::Open(const hlChar *lpFileName, hlUInt uiMode, HLPackageType ePackageType) {
	CPackage *lpPackage = PackageFromType(ePackageType);

	if (lpPackage != 0) {
		if (!lpPackage->Open(lpFileName, uiMode)) {
			delete lpPackage;
			lpPackage = 0;
		}
	}

	return lpPackage;
}

CPackage *CPackage::Open(hlVoid *lpData, hlUInt uiBufferSize, hlUInt uiMode, HLPackageType ePackageType) {
	CPackage *lpPackage = PackageFromType(ePackageType);

	if (lpPackage != 0) {
		if (!lpPackage->Open(lpData, uiBufferSize, uiMode)) {
			delete lpPackage;
			lpPackage = 0;
		}
	}

	return lpPackage;
}

CPackage *CPackage::Open(hlVoid *pUserData, hlUInt uiMode, HLPackageType ePackageType) {
	CPackage *lpPackage = PackageFromType(ePackageType);

	if (lpPackage != 0) {
		if (!lpPackage->Open(pUserData, uiMode)) {
			delete lpPackage;
			lpPackage = 0;
		}
	}

	return lpPackage;
}
