/*  NAME:
        E3IOFileFormat.c

    DESCRIPTION:
        Implementation of Quesa Abstract FileFormat object.
        
    COPYRIGHT:
        Quesa Copyright � 1999-2001, Quesa Developers.
        
        For the list of Quesa Developers, and contact details, see:
        
            Documentation/contributors.html

        For the current version of Quesa, see:

        	<http://www.quesa.org/>

		This library is free software; you can redistribute it and/or
		modify it under the terms of the GNU Lesser General Public
		License as published by the Free Software Foundation; either
		version 2 of the License, or (at your option) any later version.

		This library is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
		Lesser General Public License for more details.

		You should have received a copy of the GNU Lesser General Public
		License along with this library; if not, write to the Free Software
		Foundation Inc, 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
    ___________________________________________________________________________
*/
//=============================================================================
//      Include files
//-----------------------------------------------------------------------------
#include "E3Prefix.h"
#include "E3IO.h"
#include "E3IOFileFormat.h"
#include "E3FFR_3DMF.h"
#include "E3View.h"





//=============================================================================
//      Public functions
//-----------------------------------------------------------------------------
//      E3FileFormat_RegisterClasses : Register the classes.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_RegisterClass()
{	TQ3Status		qd3dStatus;



	// Register the class
	qd3dStatus = E3ClassTree_RegisterClass(kQ3ObjectTypeShared,
											kQ3ObjectTypeFileFormat,
											kQ3ClassNameFileFormat,
											NULL,
											0);


	// Register the built in readers
	if(qd3dStatus == kQ3Success)
		qd3dStatus = E3ClassTree_RegisterClass(kQ3ObjectTypeFileFormat,
											kQ3FileFormatTypeReader,
											kQ3ClassNameFileFormatReader,
											NULL,
											0);



	if(qd3dStatus == kQ3Success)
		qd3dStatus = E3FFormat_3DMF_Reader_RegisterClass();




	// Register the built in writers
	if(qd3dStatus == kQ3Success)
		qd3dStatus = E3ClassTree_RegisterClass(kQ3ObjectTypeFileFormat,
											kQ3FileFormatTypeWriter,
											kQ3ClassNameFileFormatWriter,
											NULL,
											0);

	if(qd3dStatus == kQ3Success)
		qd3dStatus = E3FFW_3DMF_Register();

	return(qd3dStatus);
}





//=============================================================================
//      E3FileFormat_UnregisterClass : Unregister the classes.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_UnregisterClass()
{	TQ3Status		qd3dStatus;



	// Unregister the classes
	qd3dStatus = E3FFormat_3DMF_Reader_UnregisterClass();
	qd3dStatus = E3ClassTree_UnregisterClass(kQ3FFormatReaderType3DMFBin, kQ3True);
	qd3dStatus = E3ClassTree_UnregisterClass(kQ3FFormatReaderType3DMFBinSwapped, kQ3True);
	
	qd3dStatus = E3ClassTree_UnregisterClass(kQ3FileFormatTypeReader, kQ3True);

	qd3dStatus = E3FFW_3DMF_Unregister();
	qd3dStatus = E3ClassTree_UnregisterClass(kQ3FileFormatTypeWriter, kQ3True);

	qd3dStatus = E3ClassTree_UnregisterClass(kQ3ObjectTypeFileFormat, kQ3True);

	return(qd3dStatus);
}





//=============================================================================
//      E3FileFormat_Init : Initializes the format.
//-----------------------------------------------------------------------------
TQ3Status 
E3FileFormat_Init(TQ3FileFormatObject theFileFormat, TQ3StorageObject storage)
{
	TQ3FFormatBaseData		*instanceData = (TQ3FFormatBaseData *) theFileFormat->instanceData;
	E3ClassInfoPtr storageClass;
	E3ClassInfoPtr fformatClass;
	
	fformatClass = theFileFormat->theClass;

	E3Shared_Replace(&instanceData->storage, storage);

	if( instanceData->storage != NULL)
	{
	storageClass = storage->theClass;
	
	instanceData->currentStoragePosition = 0;
	instanceData->readInGroup = kQ3True;

	
	if(Q3Storage_GetSize(storage, &instanceData->logicalEOF) == kQ3Failure)
		return kQ3Failure;
	}
	
	return(kQ3Success);
}





//=============================================================================
//      E3File_GetFileFormat : Get the file format for a file.
//-----------------------------------------------------------------------------
#pragma mark -
TQ3FileFormatObject 
E3File_GetFileFormat(TQ3FileObject theFile)
{
	TE3FileData		*instanceData = (TE3FileData *) theFile->instanceData;

	
	return(instanceData->format);
}




//=============================================================================
//      E3FileFormat_NewFromType : Create a new fileFormat object.
//-----------------------------------------------------------------------------
TQ3FileFormatObject
E3FileFormat_NewFromType(TQ3ObjectType fformatObjectType)
{
	TQ3Object		theObject;

	// Create the object
	theObject = E3ClassTree_CreateInstance(fformatObjectType, kQ3False, NULL);
	

	return(theObject);
}





//=============================================================================
//      E3FileFormat_GenericReadBinary_8 : Reads 8 bits from stream.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericReadBinary_8(TQ3FileFormatObject format, TQ3Int8* data)
{
	return Q3FileFormat_GenericReadBinary_Raw (format, (unsigned char*)data, 1);
}





//=============================================================================
//      E3FileFormat_GenericReadBinary_16 : Reads 16 bits from stream.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericReadBinary_16(TQ3FileFormatObject format, TQ3Int16* data)
{
	return Q3FileFormat_GenericReadBinary_Raw (format, (unsigned char*)data, 2);
}





//=============================================================================
//      E3FileFormat_GenericReadBinary_32 : Reads 32 bits from stream.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericReadBinary_32(TQ3FileFormatObject format, TQ3Int32* data)
{
	return Q3FileFormat_GenericReadBinary_Raw (format, (unsigned char*)data, 4);
}





//=============================================================================
//      E3FileFormat_GenericReadBinary_64 : Reads 64 bits from stream.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericReadBinary_64(TQ3FileFormatObject format, TQ3Int64* data)
{
	return Q3FileFormat_GenericReadBinary_Raw (format, (unsigned char*)data, 8);
}





//=============================================================================
//      E3FileFormat_GenericReadBinary_String : Reads a zero terminated padded
//												string from stream.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericReadBinary_String(TQ3FileFormatObject format, char* data, TQ3Uns32 *length)
{
	TQ3Uns32 					sizeRead = 0;
	TQ3Status 					result = kQ3Failure;
	TQ3FFormatBaseData			*instanceData = (TQ3FFormatBaseData *) format->instanceData;
	TQ3XStorageReadDataMethod	dataRead;
	
	char* 						dataPtr = data;
	char 						lastChar;

	dataRead = (TQ3XStorageReadDataMethod)
					E3ClassTree_GetMethod(instanceData->storage->theClass, kQ3XMethodTypeStorageReadData);

	length = 0;
	if( dataRead != NULL)
	
		do{
		result = dataRead(instanceData->storage,
							instanceData->currentStoragePosition,
							1, (unsigned char *)&lastChar, &sizeRead);
							
		instanceData->currentStoragePosition++;
		length++;
		*dataPtr = lastChar;
		dataPtr++;
		
		} 
		while ((lastChar != 0) && (result == kQ3Success));
		
		length--;// don't count trailing zero


	return result;							 
}





//=============================================================================
//      E3FileFormat_GenericReadBinary_Raw : Reads length bytes from storage.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericReadBinary_Raw(TQ3FileFormatObject format, unsigned char* data, TQ3Uns32 length)
{
	TQ3Uns32 sizeRead = 0;
	TQ3Status result = kQ3Failure;
	TQ3FFormatBaseData		*instanceData = (TQ3FFormatBaseData *) format->instanceData;
	TQ3XStorageReadDataMethod	dataRead;

	dataRead = (TQ3XStorageReadDataMethod)
					E3ClassTree_GetMethod(instanceData->storage->theClass, kQ3XMethodTypeStorageReadData);

	if( dataRead != NULL)
		result = dataRead(instanceData->storage,
							instanceData->currentStoragePosition,
							length, data, &sizeRead);

	Q3_ASSERT(sizeRead == length);
	instanceData->currentStoragePosition += length;

	return result;							 
}





//=============================================================================
//      E3FileFormat_GenericReadBinSwap_16 : Reads 16 bits from storage,
//											 swapping the byte order.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericReadBinSwap_16(TQ3FileFormatObject format, TQ3Int16* data)
{
	TQ3Status result;
	result = Q3FileFormat_GenericReadBinary_16 (format, data);
	if(result == kQ3Success)
		*data = E3EndianSwap16(*data);
	
	return result;
}





//=============================================================================
//      E3FileFormat_GenericReadBinSwap_32 : Reads 32 bits from storage,
//											 swapping the byte order.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericReadBinSwap_32(TQ3FileFormatObject format, TQ3Int32* data)
{
	TQ3Status result;
	result = Q3FileFormat_GenericReadBinary_32 (format, data);
	if(result == kQ3Success)
		*data = E3EndianSwap32(*data);
	
	return result;
}





//=============================================================================
//      E3FileFormat_GenericReadBinSwap_64 : Reads 64 bits from storage,
//											 swapping the byte order.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericReadBinSwap_64(TQ3FileFormatObject format, TQ3Int64* data)
{
	TQ3Status result;
	result = Q3FileFormat_GenericReadBinary_64 (format, data);
	if(result == kQ3Success){
		TQ3Int64 temp;
		temp.lo = E3EndianSwap32(data->hi);
		temp.hi = E3EndianSwap32(data->lo);
		*data = temp;
		}
	
	return result;
}





//=============================================================================
//      E3FileFormat_GenericReadText_SkipBlanks : positions the currentPosition
//							pointing to the first non blank char.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericReadText_SkipBlanks(TQ3FileFormatObject format)
{
	char buffer;

	TQ3Uns32 sizeRead = 0;
	TQ3Status result = kQ3Failure;
	TQ3FFormatBaseData		*instanceData = (TQ3FFormatBaseData *) format->instanceData;
	TQ3XStorageReadDataMethod	dataRead;

	dataRead = (TQ3XStorageReadDataMethod)
					E3ClassTree_GetMethod(instanceData->storage->theClass, kQ3XMethodTypeStorageReadData);

	if( dataRead != NULL)
		do{
			result = dataRead(instanceData->storage,
							instanceData->currentStoragePosition,
							1, (TQ3Uns8 *)&buffer, &sizeRead);
			
			if( (buffer <= 0x20) ||
				(buffer == 0x7F) 
				)
				instanceData->currentStoragePosition++;
			else
				break;
			
			}
		while((result == kQ3Success) && (instanceData->currentStoragePosition <= instanceData->logicalEOF));


	return result;							 
}





//=============================================================================
//      E3FileFormat_GenericReadText_ReadUntilChars : reads a string until:
//						-one of the numChars first chars in *chars is reached.
//							in this case the found char index is returned in the foundChar parameter
//						-maxLen of chars has been read without founding any of the chars in *chars
//							in this case foundChar parameter returns -1
//					charsRead returns the number of chars read not counting the flag char if found
//					currentStoragePosition is set one past the flag char
//					if the blanks flag is set, all chars< 0x20 are considered as stop chars
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericReadText_ReadUntilChars(TQ3FileFormatObject format,char* buffer,
											char* chars, TQ3Uns32 numChars, TQ3Boolean blanks,
											TQ3Int32* foundChar,TQ3Uns32 maxLen,
											TQ3Uns32* charsRead)
{
	TQ3Boolean found;

	TQ3Uns32 sizeRead = 0;
	TQ3Uns32 index = 0;
	TQ3Uns32 i;
	TQ3Status result = kQ3Failure;
	TQ3FFormatBaseData		*instanceData = (TQ3FFormatBaseData *) format->instanceData;
	TQ3XStorageReadDataMethod	dataRead;

	if(foundChar)
		*foundChar = -1;

	dataRead = (TQ3XStorageReadDataMethod)
					E3ClassTree_GetMethod(instanceData->storage->theClass, kQ3XMethodTypeStorageReadData);

	if( dataRead != NULL)
			found = kQ3False;
			result = dataRead(instanceData->storage,
							instanceData->currentStoragePosition,
							maxLen, (TQ3Uns8*)buffer, &sizeRead); // read all the data at once
			
		while((result == kQ3Success)
				&& (instanceData->currentStoragePosition < instanceData->logicalEOF) 
				&& (index < sizeRead) 
				 ){
			instanceData->currentStoragePosition ++;
			
			for(i = 0; i< numChars; i++)
				if((chars[i] == buffer[index]) || ((blanks == kQ3True) && (buffer[index] <= 0x20))
					|| ((chars[i] == 0x0D) && (buffer[index] == 0x0A)))// unix file
					{
					if(foundChar)
						*foundChar = buffer[index];
					if((chars[i] == 0x0D) && (buffer[index+1] == 0x0A))// windows file
						instanceData->currentStoragePosition ++;
					buffer[index] = 0;
					found = kQ3True;
					break;
					}
			
			if(found == kQ3True)
				break;
			else
				{
				index++;
				}
			
			};

	if(charsRead)
		*charsRead = index;
	
	return result;							 
}



//=============================================================================
//      E3FileFormat_GenericWriteBinary_8 : Writes 8 bits to a stream.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericWriteBinary_8(TQ3FileFormatObject format, TQ3Int8 data)
{
	return Q3FileFormat_GenericWriteBinary_Raw (format, (const unsigned char*)&data, 1);
}





//=============================================================================
//      E3FileFormat_GenericWriteBinary_16 : Writes 16 bits to a stream.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericWriteBinary_16(TQ3FileFormatObject format, TQ3Int16 data)
{
	return Q3FileFormat_GenericWriteBinary_Raw (format, (const unsigned char*)&data, 2);
}





//=============================================================================
//      E3FileFormat_GenericWriteBinary_32 : Writes 32 bits to a stream.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericWriteBinary_32(TQ3FileFormatObject format, TQ3Int32 data)
{
	return Q3FileFormat_GenericWriteBinary_Raw (format, (const unsigned char*)&data, 4);
}





//=============================================================================
//      E3FileFormat_GenericWriteBinary_64 : Writes 64 bits to a stream.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericWriteBinary_64(TQ3FileFormatObject format, TQ3Int64 data)
{
	return Q3FileFormat_GenericWriteBinary_Raw (format, (const unsigned char*)&data, 8);
}





//=============================================================================
//      E3FileFormat_GenericWriteBinary_String : Writes a zero terminated padded
//												string to a stream.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericWriteBinary_String(TQ3FileFormatObject format, const char* data, TQ3Uns32 *length)
{


	// To be implemented...
	return(kQ3Failure);
}





//=============================================================================
//      E3FileFormat_GenericWriteBinary_Raw : Writes length bytes to a storage.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericWriteBinary_Raw(TQ3FileFormatObject format,const unsigned char* data, TQ3Uns32 length)
{
	TQ3Uns32 sizeWrite = 0;
	TQ3Status result = kQ3Failure;
	TQ3FFormatBaseData		*instanceData = (TQ3FFormatBaseData *) format->instanceData;
	TQ3XStorageWriteDataMethod	dataWrite;

	dataWrite = (TQ3XStorageWriteDataMethod)
					E3ClassTree_GetMethod(instanceData->storage->theClass, kQ3XMethodTypeStorageWriteData);

	if( dataWrite != NULL)
		result = dataWrite(instanceData->storage,
							instanceData->currentStoragePosition,
							length, data, &sizeWrite);

	Q3_ASSERT(sizeWrite == length);
	instanceData->currentStoragePosition += length;

	return result;							 
}





//=============================================================================
//      E3FileFormat_GenericWriteBinSwap_16 : Writes 16 bits to a storage,
//											 swapping the byte order.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericWriteBinSwap_16(TQ3FileFormatObject format, TQ3Int16 data)
{
	TQ3Status result;
	data = E3EndianSwap16(data);
	result = Q3FileFormat_GenericWriteBinary_16 (format, data);
	
	return result;
}





//=============================================================================
//      E3FileFormat_GenericWriteBinSwap_32 : Writes 32 bits to a storage,
//											 swapping the byte order.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericWriteBinSwap_32(TQ3FileFormatObject format, TQ3Int32 data)
{
	TQ3Status result;
	data = E3EndianSwap32(data);
	result = Q3FileFormat_GenericWriteBinary_32 (format, data);
	
	return result;
}





//=============================================================================
//      E3FileFormat_GenericWriteBinSwap_64 : Writes 64 bits to a storage,
//											 swapping the byte order.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GenericWriteBinSwap_64(TQ3FileFormatObject format, TQ3Int64 data)
{
	TQ3Status result;
	TQ3Int64 temp;
	temp.lo = E3EndianSwap32(data.hi);
	temp.hi = E3EndianSwap32(data.lo);
	
	result = Q3FileFormat_GenericWriteBinary_64 (format, temp);
	
	return result;
}



//=============================================================================
//      E3FileFormat_GetType : Return the type of a fileFormat.
//-----------------------------------------------------------------------------
TQ3ObjectType
E3FileFormat_GetType(TQ3FileFormatObject theFormat)
{


	// Return the type
	return(E3ClassTree_GetObjectType(theFormat, kQ3ObjectTypeFileFormat));
}




//=============================================================================
//      E3FileFormat_HasModalConfigure : Does a fileFormat have a configure dialog?
//-----------------------------------------------------------------------------
TQ3Boolean
E3FileFormat_HasModalConfigure(TQ3FileFormatObject theFormat)
{


	// Return as the method is defined or not
	return((TQ3Boolean) (E3ClassTree_GetMethod(theFormat->theClass,
										 		kQ3XMethodTypeRendererModalConfigure) != NULL));
}





//=============================================================================
//      E3FileFormat_ModalConfigure : Invoke a fileFormat's modal configure dialog.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_ModalConfigure(TQ3FileFormatObject theFormat, TQ3DialogAnchor dialogAnchor, TQ3Boolean *canceled)
{	TQ3XRendererModalConfigureMethod	modalConfigure;
	TQ3Status							qd3dStatus;



	// Find the method
	modalConfigure = (TQ3XRendererModalConfigureMethod)
						E3ClassTree_GetMethod(theFormat->theClass,
											  kQ3XMethodTypeRendererModalConfigure);
	if (modalConfigure == NULL)
		return(kQ3Failure);



	// Call the method
	qd3dStatus = modalConfigure(theFormat, dialogAnchor, canceled, theFormat->instanceData);

	return(qd3dStatus);
}





//=============================================================================
//      E3FileFormatClass_GetFormatNameString : Get the fileFormat nick name.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormatClass_GetFormatNameString(TQ3ObjectType fileFormatClassType, TQ3ObjectClassNameString fileFormatClassString)
{	TQ3XRendererGetNickNameStringMethod		nickNameMethod;
	E3ClassInfoPtr							fileFormatClass;
	TQ3Status								qd3dStatus;
	TQ3Uns32								numWritten;
	


	// Initialise a return value
	fileFormatClassString[0] = 0x00;



	// Find the fileFormat class, and get the method
	fileFormatClass = E3ClassTree_GetClassByType(fileFormatClassType);
	if (fileFormatClass == NULL)
		return(kQ3Failure);

	nickNameMethod = (TQ3XRendererGetNickNameStringMethod)
								E3ClassTree_GetMethod(fileFormatClass,
													  kQ3XMethodTypeRendererGetNickNameString);
	if (nickNameMethod == NULL)
		return(kQ3Failure);



	// Call the method
	qd3dStatus = nickNameMethod((unsigned char *) fileFormatClassString,
									sizeof(TQ3ObjectClassNameString),
									&numWritten);

	return(qd3dStatus);
}





//=============================================================================
//      E3FileFormat_GetConfigurationData : Get a fileFormat's config data.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_GetConfigurationData(TQ3FileFormatObject theFormat, unsigned char *dataBuffer, TQ3Uns32 bufferSize, TQ3Uns32 *actualDataSize)
{	TQ3XRendererGetConfigurationDataMethod	getConfigData;
	TQ3Status								qd3dStatus;



	// Initialise a return value
	*actualDataSize = 0;



	// Find the method
	getConfigData = (TQ3XRendererGetConfigurationDataMethod)
						E3ClassTree_GetMethod(theFormat->theClass,
											  kQ3XMethodTypeRendererGetConfigurationData);
	if (getConfigData == NULL)
		return(kQ3Failure);



	// Call the method
	qd3dStatus = getConfigData(theFormat,
								dataBuffer,
								bufferSize,
								actualDataSize,
								theFormat->instanceData);

	return(qd3dStatus);
}





//=============================================================================
//      E3FileFormat_SetConfigurationData : Set a fileFormat's config data.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_SetConfigurationData(TQ3FileFormatObject theFormat, unsigned char *dataBuffer, TQ3Uns32 bufferSize)
{	TQ3XRendererSetConfigurationDataMethod	setConfigData;
	TQ3Status								qd3dStatus;



	// Find the method
	setConfigData = (TQ3XRendererSetConfigurationDataMethod)
						E3ClassTree_GetMethod(theFormat->theClass,
											  kQ3XMethodTypeRendererSetConfigurationData);
	if (setConfigData == NULL)
		return(kQ3Failure);



	// Call the method
	qd3dStatus = setConfigData(theFormat,
								dataBuffer,
								bufferSize,
								theFormat->instanceData);

	Q3Shared_Edited(theFormat);

	return(qd3dStatus);
}



//=============================================================================
//      E3FileFormat_Method_StartFile : Call the start frame method.
//-----------------------------------------------------------------------------
#pragma mark -
TQ3Status
E3FileFormat_Method_StartFile(TQ3ViewObject theView)
{	TQ3FileFormatObject				theFormat = E3View_AccessFileFormat(theView);
	TQ3XRendererStartFrameMethod	startFrame;
	TQ3Status						qd3dStatus;



	// No-op if no format set
	if (theFormat == NULL)
		return(kQ3Success);



	// Find the method
	startFrame = (TQ3XRendererStartFrameMethod)
					E3ClassTree_GetMethod(theFormat->theClass,
										  kQ3XMethodTypeRendererStartFrame);
	if (startFrame == NULL)
		return(kQ3Success);



	// Call the method
	qd3dStatus = startFrame(theView, theFormat->instanceData, NULL);

	return(qd3dStatus);
}





//=============================================================================
//      E3FileFormat_Method_StartPass : Call the start pass method.
//-----------------------------------------------------------------------------
TQ3Status
E3FileFormat_Method_StartPass(TQ3ViewObject theView)
{	TQ3FileFormatObject				theFormat = E3View_AccessFileFormat(theView);
	TQ3XRendererStartPassMethod		startPass;
	TQ3Status						qd3dStatus;



	// No-op if no file FormatSet set
	if (theFormat == NULL)
		return(kQ3Success);



	// Find the method
	startPass = (TQ3XRendererStartPassMethod)
					E3ClassTree_GetMethod(theFormat->theClass,
										  kQ3XMethodTypeRendererStartPass);
	if (startPass == NULL)
		return(kQ3Success);



	// Call the method
	qd3dStatus = startPass(theView, theFormat->instanceData, NULL, NULL);

	return(qd3dStatus);
}





//=============================================================================
//      E3FileFormat_Method_EndPass : Call the end pass method.
//-----------------------------------------------------------------------------
TQ3ViewStatus
E3FileFormat_Method_EndPass(TQ3ViewObject theView)
{	TQ3FileFormatObject				theFormat = E3View_AccessFileFormat(theView);
	TQ3ViewStatus					viewStatus;
	TQ3XRendererEndPassMethod		endPass;



	// No-op if no format set
	if (theFormat == NULL)
		return(kQ3ViewStatusDone);



	// Find the method
	endPass = (TQ3XRendererEndPassMethod)
					E3ClassTree_GetMethod(theFormat->theClass,
										  kQ3XMethodTypeRendererEndPass);
	if (endPass == NULL)
		return(kQ3ViewStatusDone);



	// Call the method
	viewStatus = endPass(theView, theFormat->instanceData);

	return(viewStatus);
}


