/* 
   Unix SMB/CIFS implementation.
   SMB parameters and setup, plus a whole lot more.
   
   Copyright (C) Andrew Tridgell              2001
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _WERROR_H_
#define _WERROR_H_

#include <stdint.h>

/* the following rather strange looking definitions of NTSTATUS and WERROR
   and there in order to catch common coding errors where different error types
   are mixed up. This is especially important as we slowly convert Samba
   from using bool for internal functions 
*/

#if defined(HAVE_IMMEDIATE_STRUCTURES)
typedef struct {uint32_t v;} WERROR;
#define W_ERROR(x) ((WERROR) { x })
#define W_ERROR_V(x) ((x).v)
#else
typedef uint32_t WERROR;
#define W_ERROR(x) (x)
#define W_ERROR_V(x) (x)
#endif

#define W_ERROR_IS_OK(x) (W_ERROR_V(x) == 0)
#define W_ERROR_EQUAL(x,y) (W_ERROR_V(x) == W_ERROR_V(y))

#define W_ERROR_HAVE_NO_MEMORY(x) do { \
	if (!(x)) {\
		return WERR_NOMEM;\
	}\
} while (0)

#define W_ERROR_IS_OK_RETURN(x) do { \
	if (W_ERROR_IS_OK(x)) {\
		return x;\
	}\
} while (0)

#define W_ERROR_NOT_OK_RETURN(x) do { \
	if (!W_ERROR_IS_OK(x)) {\
		return x;\
	}\
} while (0)

#define W_ERROR_NOT_OK_GOTO_DONE(x) do { \
	if (!W_ERROR_IS_OK(x)) {\
		goto done;\
	}\
} while (0)

#define W_ERROR_NOT_OK_GOTO(x, y) do {\
	if (!W_ERROR_IS_OK(x)) {\
		goto y;\
	}\
} while(0)

/* these are win32 error codes. There are only a few places where
   these matter for Samba, primarily in the NT printing code */
#define WERR_OK W_ERROR(0)
#define WERR_BADFUNC W_ERROR(1)
#define WERR_BADFILE W_ERROR(2)
#define WERR_ACCESS_DENIED W_ERROR(5)
#define WERR_BADFID W_ERROR(6)
#define WERR_NOMEM W_ERROR(8)
#define WERR_GENERAL_FAILURE W_ERROR(31)
#define WERR_NOT_SUPPORTED W_ERROR(50)
#define WERR_DUP_NAME W_ERROR(52)
#define WERR_BAD_NETPATH W_ERROR(53)
#define WERR_BAD_NET_RESP W_ERROR(58)
#define WERR_UNEXP_NET_ERR W_ERROR(59)
#define WERR_DEVICE_NOT_EXIST W_ERROR(55)
#define WERR_PRINTQ_FULL W_ERROR(61)
#define WERR_NO_SPOOL_SPACE W_ERROR(62)
#define WERR_NO_SUCH_SHARE W_ERROR(67)
#define WERR_FILE_EXISTS W_ERROR(80)
#define WERR_BAD_PASSWORD W_ERROR(86)
#define WERR_INVALID_PARAM W_ERROR(87)
#define WERR_CALL_NOT_IMPLEMENTED W_ERROR(120)
#define WERR_SEM_TIMEOUT W_ERROR(121)
#define WERR_INSUFFICIENT_BUFFER W_ERROR(122)
#define WERR_INVALID_NAME W_ERROR(123)
#define WERR_UNKNOWN_LEVEL W_ERROR(124)
#define WERR_OBJECT_PATH_INVALID W_ERROR(161)
#define WERR_ALREADY_EXISTS W_ERROR(183)
#define WERR_NO_MORE_ITEMS W_ERROR(259)
#define WERR_MORE_DATA W_ERROR(234)
#define WERR_INVALID_OWNER W_ERROR(1307)
#define WERR_IO_PENDING W_ERROR(997)
#define WERR_CAN_NOT_COMPLETE W_ERROR(1003)
#define WERR_INVALID_FLAGS W_ERROR(1004)
#define WERR_REG_CORRUPT W_ERROR(1015)
#define WERR_REG_IO_FAILURE W_ERROR(1016)
#define WERR_REG_FILE_INVALID W_ERROR(1017)
#define WERR_NO_SUCH_SERVICE W_ERROR(1060)
#define WERR_INVALID_SERVICE_CONTROL W_ERROR(1052)
#define WERR_SERVICE_ALREADY_RUNNING W_ERROR(1056)
#define WERR_SERVICE_DISABLED W_ERROR(1058)
#define WERR_SERVICE_MARKED_FOR_DELETE W_ERROR(1072)
#define WERR_SERVICE_EXISTS W_ERROR(1073)
#define WERR_SERVICE_NEVER_STARTED W_ERROR(1077)
#define WERR_DUPLICATE_SERVICE_NAME W_ERROR(1078)
#define WERR_DEVICE_NOT_CONNECTED W_ERROR(1167)
#define WERR_NOT_FOUND W_ERROR(1168)
#define WERR_INVALID_COMPUTERNAME W_ERROR(1210)
#define WERR_INVALID_DOMAINNAME W_ERROR(1212)
#define WERR_NOT_AUTHENTICATED W_ERROR(1244)
#define WERR_UNKNOWN_REVISION W_ERROR(1305)
#define WERR_MACHINE_LOCKED W_ERROR(1271)
#define WERR_REVISION_MISMATCH W_ERROR(1306)
#define WERR_INVALID_OWNER W_ERROR(1307)
#define WERR_NO_LOGON_SERVERS W_ERROR(1311)
#define WERR_NO_SUCH_LOGON_SESSION W_ERROR(1312)
#define WERR_NO_SUCH_PRIVILEGE W_ERROR(1313)
#define WERR_PRIVILEGE_NOT_HELD W_ERROR(1314)
#define WERR_USER_ALREADY_EXISTS W_ERROR(1316)
#define WERR_NO_SUCH_USER W_ERROR(1317)
#define WERR_GROUP_EXISTS W_ERROR(1318)
#define WERR_MEMBER_IN_GROUP W_ERROR(1320)
#define WERR_USER_NOT_IN_GROUP W_ERROR(1321)
#define WERR_WRONG_PASSWORD W_ERROR(1323)
#define WERR_PASSWORD_RESTRICTION W_ERROR(1325)
#define WERR_LOGON_FAILURE W_ERROR(1326)
#define WERR_NO_SUCH_DOMAIN W_ERROR(1355)
#define WERR_NONE_MAPPED W_ERROR(1332)
#define WERR_INVALID_SECURITY_DESCRIPTOR W_ERROR(1338)
#define WERR_INVALID_DOMAIN_STATE W_ERROR(1353)
#define WERR_INVALID_DOMAIN_ROLE W_ERROR(1354)
#define WERR_NO_SUCH_DOMAIN W_ERROR(1355)
#define WERR_NO_SYSTEM_RESOURCES W_ERROR(1450)
#define WERR_SPECIAL_ACCOUNT W_ERROR(1371)
#define WERR_NO_SUCH_ALIAS W_ERROR(1376)
#define WERR_MEMBER_IN_ALIAS W_ERROR(1378)
#define WERR_ALIAS_EXISTS W_ERROR(1379)
#define WERR_TIME_SKEW W_ERROR(1398)
#define WERR_EVENTLOG_FILE_CORRUPT W_ERROR(1500)
#define WERR_SERVER_UNAVAILABLE W_ERROR(1722)
#define WERR_INVALID_USER_BUFFER W_ERROR(1784)
#define WERR_NO_TRUST_SAM_ACCOUNT W_ERROR(1787)
#define WERR_INVALID_FORM_NAME W_ERROR(1902)
#define WERR_INVALID_FORM_SIZE W_ERROR(1903)
#define WERR_PASSWORD_MUST_CHANGE W_ERROR(1907)
#define WERR_ACCOUNT_LOCKED_OUT W_ERROR(1909)
#define WERR_ALREADY_SHARED W_ERROR(2118)
#define WERR_NOT_CONNECTED W_ERROR(2250)
#define WERR_NAME_NOT_FOUND W_ERROR(2273)
#define WERR_SESSION_NOT_FOUND W_ERROR(2312)
#define WERR_FID_NOT_FOUND W_ERROR(2314)
#define WERR_DOMAIN_CONTROLLER_NOT_FOUND W_ERROR(2453)
#define WERR_TIME_DIFF_AT_DC W_ERROR(2457)

#define WERR_DEVICE_NOT_AVAILABLE W_ERROR(4319)
#define WERR_STATUS_MORE_ENTRIES   W_ERROR(0x0105)

#define WERR_PRINTER_DRIVER_ALREADY_INSTALLED W_ERROR(ERRdriveralreadyinstalled)
#define WERR_UNKNOWN_PORT W_ERROR(ERRunknownprinterport)
#define WERR_UNKNOWN_PRINTER_DRIVER W_ERROR(ERRunknownprinterdriver)
#define WERR_UNKNOWN_PRINTPROCESSOR W_ERROR(ERRunknownprintprocessor)
#define WERR_INVALID_SEPARATOR_FILE W_ERROR(ERRinvalidseparatorfile)
#define WERR_INVALID_PRIORITY W_ERROR(ERRinvalidjobpriority)
#define WERR_INVALID_PRINTER_NAME W_ERROR(ERRinvalidprintername)
#define WERR_PRINTER_ALREADY_EXISTS W_ERROR(ERRprinteralreadyexists)
#define WERR_INVALID_PRINTER_COMMAND W_ERROR(ERRinvalidprintercommand)
#define WERR_INVALID_DATATYPE W_ERROR(ERRinvaliddatatype)
#define WERR_INVALID_ENVIRONMENT W_ERROR(ERRinvalidenvironment)

#define WERR_UNKNOWN_PRINT_MONITOR W_ERROR(ERRunknownprintmonitor)
#define WERR_PRINTER_DRIVER_IN_USE W_ERROR(ERRprinterdriverinuse)
#define WERR_SPOOL_FILE_NOT_FOUND W_ERROR(ERRspoolfilenotfound)
#define WERR_SPL_NO_STARTDOC W_ERROR(ERRnostartdoc)
#define WERR_SPL_NO_ADDJOB W_ERROR(ERRnoaddjob)
#define WERR_PRINT_PROCESSOR_ALREADY_INSTALLED W_ERROR(ERRprintprocessoralreadyinstalled)
#define WERR_PRINT_MONITOR_ALREADY_INSTALLED W_ERROR(ERRprintmonitoralreadyinstalled)
#define WERR_INVALID_PRINT_MONITOR W_ERROR(ERRinvalidprintmonitor)
#define WERR_PRINT_MONITOR_IN_USE W_ERROR(ERRprintmonitorinuse)
#define WERR_PRINTER_HAS_JOBS_QUEUED W_ERROR(ERRprinterhasjobsqueued)

#define WERR_CLASS_NOT_REGISTERED W_ERROR(0x40154)
#define WERR_NO_SHUTDOWN_IN_PROGRESS W_ERROR(0x45c)
#define WERR_SHUTDOWN_ALREADY_IN_PROGRESS W_ERROR(0x45b)
/* Configuration Manager Errors */
/* Basically Win32 errors meanings are specific to the \ntsvcs pipe */

#define WERR_CM_INVALID_POINTER W_ERROR(3)
#define WERR_CM_BUFFER_SMALL W_ERROR(26)
#define WERR_CM_NO_MORE_HW_PROFILES W_ERROR(35)
#define WERR_CM_NO_SUCH_VALUE W_ERROR(37)

#define WERR_DEVICE_NOT_SHARED		W_ERROR(NERR_BASE+211)

/* DFS errors */

#ifndef NERR_BASE
#define NERR_BASE (2100)
#endif

#ifndef MAX_NERR
#define MAX_NERR (NERR_BASE+899)
#endif

#define WERR_BUF_TOO_SMALL		W_ERROR(NERR_BASE+23)
#define WERR_JOB_NOT_FOUND		W_ERROR(NERR_BASE+51)
#define WERR_DEST_NOT_FOUND		W_ERROR(NERR_BASE+52)
#define WERR_GROUP_NOT_FOUND		W_ERROR(NERR_BASE+120)
#define WERR_USER_NOT_FOUND		W_ERROR(NERR_BASE+121)
#define WERR_USER_EXISTS		W_ERROR(NERR_BASE+124)
#define WERR_NET_NAME_NOT_FOUND		W_ERROR(NERR_BASE+210)
#define WERR_NOT_LOCAL_DOMAIN		W_ERROR(NERR_BASE+220)
#define WERR_DC_NOT_FOUND		W_ERROR(NERR_BASE+353)
#define WERR_DFS_NO_SUCH_VOL            W_ERROR(NERR_BASE+562)
#define WERR_DFS_NO_SUCH_SHARE          W_ERROR(NERR_BASE+565)
#define WERR_DFS_NO_SUCH_SERVER         W_ERROR(NERR_BASE+573)
#define WERR_DFS_INTERNAL_ERROR         W_ERROR(NERR_BASE+590)
#define WERR_DFS_CANT_CREATE_JUNCT      W_ERROR(NERR_BASE+569)
#define WERR_SETUP_ALREADY_JOINED	W_ERROR(NERR_BASE+591)
#define WERR_SETUP_NOT_JOINED		W_ERROR(NERR_BASE+592)
#define WERR_SETUP_DOMAIN_CONTROLLER	W_ERROR(NERR_BASE+593)
#define WERR_DEFAULT_JOIN_REQUIRED	W_ERROR(NERR_BASE+594)

/* DS errors */
#define WERR_DS_SERVICE_BUSY W_ERROR(0x0000200e)
#define WERR_DS_SERVICE_UNAVAILABLE W_ERROR(0x0000200f)
#define WERR_DS_NO_SUCH_OBJECT W_ERROR(0x00002030)
#define WERR_DS_OBJ_NOT_FOUND W_ERROR(0x0000208d)
#define WERR_DS_SCHEMA_NOT_LOADED W_ERROR(0x20de)
#define WERR_DS_SCHEMA_ALLOC_FAILED W_ERROR(0x20df)
#define WERR_DS_ATT_SCHEMA_REQ_SYNTAX W_ERROR(0x000020e0)
#define WERR_DS_DRA_SCHEMA_MISMATCH W_ERROR(0x000020e2)
#define WERR_DS_DRA_INVALID_PARAMETER W_ERROR(0x000020f5)
#define WERR_DS_DRA_BAD_DN W_ERROR(0x000020f7)
#define WERR_DS_DRA_BAD_NC W_ERROR(0x000020f8)
#define WERR_DS_DRA_INTERNAL_ERROR W_ERROR(0x000020fa)
#define WERR_DS_DRA_OUT_OF_MEM W_ERROR(0x000020fe)
#define WERR_DS_SINGLE_VALUE_CONSTRAINT W_ERROR(0x00002081)
#define WERR_DS_DRA_DB_ERROR W_ERROR(0x00002103)
#define WERR_DS_DRA_NO_REPLICA W_ERROR(0x00002104)
#define WERR_DS_DRA_ACCESS_DENIED W_ERROR(0x00002105)
#define WERR_DS_DRA_SOURCE_DISABLED W_ERROR(0x00002108)
#define WERR_DS_DNS_LOOKUP_FAILURE W_ERROR(0x0000214c)
#define WERR_DS_WRONG_LINKED_ATTRIBUTE_SYNTAX W_ERROR(0x00002150)
#define WERR_DS_NO_MSDS_INTID W_ERROR(0x00002194)
#define WERR_DS_DUP_MSDS_INTID W_ERROR(0x00002195)

/* FRS errors */
#define WERR_FRS_INSUFFICIENT_PRIV W_ERROR(FRS_ERR_BASE+7)
#define WERR_FRS_SYSVOL_IS_BUSY W_ERROR(FRS_ERR_BASE+15)
#define WERR_FRS_INVALID_SERVICE_PARAMETER W_ERROR(FRS_ERR_BASE+17)

/* RPC errors */
#define WERR_RPC_E_INVALID_HEADER	W_ERROR(0x80010111)
#define WERR_RPC_E_REMOTE_DISABLED	W_ERROR(0x8001011c)

/* SEC errors */
#define WERR_SEC_E_ENCRYPT_FAILURE	W_ERROR(0x80090329)
#define WERR_SEC_E_DECRYPT_FAILURE	W_ERROR(0x80090330)
#define WERR_SEC_E_ALGORITHM_MISMATCH	W_ERROR(0x80090331)

#define WERR_FOOBAR WERR_GENERAL_FAILURE

/*****************************************************************************
 returns a windows error message.  not amazingly helpful, but better than a number.
 *****************************************************************************/
const char *win_errstr(WERROR werror);

const char *get_friendly_werror_msg(WERROR werror);


#endif
