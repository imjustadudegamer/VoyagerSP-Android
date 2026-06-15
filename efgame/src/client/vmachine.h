// vmachine.h -- virtual machine header for client
#ifndef __VMACHINE_H__
#define __VMACHINE_H__

/*
==================================================================

functions exported to the main executable

==================================================================
*/

typedef enum {
	CG_INIT,
	CG_SHUTDOWN,
	CG_CONSOLE_COMMAND,
	CG_DRAW_ACTIVE_FRAME,
	CG_CROSSHAIR_PLAYER,
	CG_CAMERA_POS,
} cgameExport_t;

/*
==============================================================

VIRTUAL MACHINE

==============================================================
*/
struct vm_s {
	int			(*entryPoint)( int callNum, ... );
};

typedef struct vm_s vm_t;

extern	vm_t	cgvm;	// interface to cgame dll or vm
extern int	VM_Call( int callnum, ... );
extern int VM_DllSyscall( int arg, ... );
extern void CL_ShutdownCGame(void);

/*
================
VM_Create

it will attempt to load as a system dll
================
*/
extern void *Sys_LoadCgame( int (**entryPoint)(int, ...), int (*systemcalls)(int, ...) );

inline void *VM_Create() 
{
	void *res;
	// try to load as a system dll
	res = Sys_LoadCgame( &cgvm.entryPoint, VM_DllSyscall );
	if ( !res) 
	{
		//Com_Printf( "Failed.\n" );
		return 0;
	}
	return res;
}

#endif //__VMACHINE_H__