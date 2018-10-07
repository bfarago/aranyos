/*
 * det.h
 * Developer Error Trace
 *  Created on: Oct 7, 2018
 *      Author: Barna Farago
 */

#ifndef DET_H_
#define DET_H_
#include "typedefs.h"

// Report an error
uint8_t Det_ReportError(
 uint16_t ModuleId,
 uint8_t InstanceId,
 uint8_t ApiId,
 uint8_t ErrorId
);

#endif /* DET_H_ */
