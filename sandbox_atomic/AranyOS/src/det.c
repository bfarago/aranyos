/*
 * det.c
 * Developer Error Trace Implementation
 *  Created on: Oct 7, 2018
 *      Author: Barna Farago
 */
#include "det.h"

// Globals
 uint16_t Det_Last_ModuleId;
 uint8_t Det_Last_InstanceId;
 uint8_t Det_Last_ApiId;
 uint8_t Det_Last_ErrorId;
 
 /* Developer Error Trace Report Function
  */
 uint8_t Det_ReportError(
  uint16_t ModuleId,
  uint8_t InstanceId,
  uint8_t ApiId,
  uint8_t ErrorId
 )
{
	Det_Last_ModuleId= ModuleId;
	Det_Last_InstanceId=InstanceId;
	Det_Last_ApiId=ApiId;
	Det_Last_ErrorId= ErrorId;
	return 0;
}
