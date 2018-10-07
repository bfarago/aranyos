/*
 * aranyos_cfg.h
 *
 *  Created on: Oct 7, 2018
 *      Author: Barna
 */

#ifndef ARANYOS_CFG_H_
#define ARANYOS_CFG_H_

#define ARANYOS_MAX_TASKS 4
#define OS_DET_MODULE_ID  0x0123u
#define OS_DET_INSTANCE_ID  0x00u

#define Os_Det_ReportError(aid, eid) Det_ReportError(OS_DET_MODULE_ID, OS_DET_INSTANCE_ID, aid, eid)


#endif /* ARANYOS_CFG_H_ */
