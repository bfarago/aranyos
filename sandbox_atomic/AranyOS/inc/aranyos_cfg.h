/*
 * aranyos_cfg.h
 * AranyOS config header
 *  Created on: Oct 7, 2018
 *      Author: Barna
 */

#ifndef ARANYOS_CFG_H_
#define ARANYOS_CFG_H_

//max possible tasks at a time, allocation max.
#define ARANYOS_MAX_TASKS 4

//DET trace Module ID
#define OS_DET_MODULE_ID  0x0123u

//DET trace Instance ID (probably core Id would be better)
#define OS_DET_INSTANCE_ID  0x00u

//OS use this macro to call DET report.
#define Os_Det_ReportError(aid, eid) Det_ReportError(OS_DET_MODULE_ID, OS_DET_INSTANCE_ID, aid, eid)


#endif /* ARANYOS_CFG_H_ */
