/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                           Ken Stevens, Steve McClure
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  ---
 *
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  service.c: Windows services support
 * 
 *  Known contributors to this file:
 *     Ron Koenderink, 2004
 */

#ifdef _WIN32
#include <windows.h>

#include "prototypes.h"
#include "service.h"
#include "../gen/getopt.h"
#include "optlist.h"

int
install_service(char *program_name, char *service_name, int datadir_set, char *config_file)
{
    char strDir[1024];
    HANDLE schSCManager,schService;
    LPCTSTR lpszBinaryPathName;
    SERVICE_DESCRIPTION sdBuf;

    if (strrchr(program_name,'\\') == NULL) {
	GetCurrentDirectory(sizeof(strDir), strDir);
	strcat(strDir, "\\");
	strcat(strDir, program_name);
    } else
	strcpy(strDir, program_name);

    if (datadir_set) {
	strcat(strDir, " -D ");
	strcat(strDir, datadir);
    }
    if (config_file != NULL) {
	strcat(strDir, " -e ");
	strcat(strDir, config_file);
    }

    if (service_name == NULL)
	service_name = DEFAULT_SERVICE_NAME;
    else if (service_name[0] == '\0')
	service_name = DEFAULT_SERVICE_NAME;

    schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

    if (schSCManager == NULL) {
        logerror("install_service failed to open Service Control Manager"); 
	return EXIT_FAILURE;
    }

    lpszBinaryPathName = strDir;

    schService = CreateService(schSCManager,
    	service_name,
    	service_name,			/* service name to display */
        SERVICE_ALL_ACCESS,		/* desired access */
        SERVICE_WIN32_OWN_PROCESS,	/* service type */
        SERVICE_AUTO_START,		/* start type */
        SERVICE_ERROR_NORMAL,		/* error control type */
        lpszBinaryPathName,		/* service's binary */
        NULL,				/* no load ordering group */
        NULL,				/* no tag identifier */
        NULL,				/* database service dependency */
        NULL,				/* LocalSystem account */
        NULL);				/* no password */
 
    if (schService == NULL) {
	logerror("install_service failed to create service %s", service_name);
        return EXIT_FAILURE;
    }
    sdBuf.lpDescription = "Server for Empire game";

    if(!ChangeServiceConfig2(
          schService,                 /* handle to service */
          SERVICE_CONFIG_DESCRIPTION, /* change: description */
          &sdBuf)) {                  /* value: new description */
        logerror("install_service failed to set the description");
    }

    printf("Service %s installed.\n", service_name);
    CloseServiceHandle(schService);
    return EXIT_SUCCESS;
}

int
remove_service(char *service_name)
{
    HANDLE schSCManager;
    SC_HANDLE hService;

    if (service_name == NULL)
	service_name = DEFAULT_SERVICE_NAME;
    else if (service_name[0] == '\0')
	service_name = DEFAULT_SERVICE_NAME;

    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (schSCManager == NULL) {
        logerror("remove_service failed to open Service Control Manager"); 
	return EXIT_FAILURE;
    }

    hService = OpenService(schSCManager, service_name, SERVICE_ALL_ACCESS);

    if (hService == NULL) {
        logerror("remove_service failed to open service %s", service_name);
	return EXIT_FAILURE;
    }

    if (DeleteService(hService) == 0) {
        logerror("remove_service failed to remove service %s", service_name);
	return EXIT_FAILURE;
    }

    if (CloseServiceHandle(hService) == 0) {
        logerror("remove_service failed to close service %s", service_name); 
        return EXIT_FAILURE;
    } else {
        printf("Service %s removed.\n", service_name); 
        return EXIT_SUCCESS;
    }
}

static SERVICE_STATUS          service_status; 
static SERVICE_STATUS_HANDLE   service_status_handle; 

void WINAPI
service_ctrl_handler(DWORD Opcode) 
{ 
    DWORD status; 
 
    switch(Opcode) 
    { 
        case SERVICE_CONTROL_PAUSE: 
            service_status.dwCurrentState = SERVICE_PAUSED;
	    logerror("Pausing the service not supported");
            break; 
 
        case SERVICE_CONTROL_CONTINUE: 
	    logerror("Continuing the service not supported");
            service_status.dwCurrentState = SERVICE_RUNNING; 
            break; 
 
        case SERVICE_CONTROL_STOP: 
            service_status.dwWin32ExitCode = 0; 
            service_status.dwCurrentState  = SERVICE_STOPPED; 
            service_status.dwCheckPoint    = 0; 
            service_status.dwWaitHint      = 0; 
 
            if (!SetServiceStatus (service_status_handle, 
                &service_status)) { 
                status = GetLastError(); 
                logerror("Error while stopping service SetServiceStatus"
		    " error %ld", status); 
            } 
 
            logerror("Service stopped"); 
            return; 
 
        case SERVICE_CONTROL_INTERROGATE: 
        /* Fall through to send current status.  */
            break; 
 
        default: 
            logerror("Unrecognized opcode %ld in ServiceCtrlHandler", 
                Opcode); 
    } 
 
    /* Send current status. */
    if (!SetServiceStatus (service_status_handle,  &service_status)) { 
        status = GetLastError(); 
        logerror("SetServiceStatus error %ld",status); 
    } 
    return; 
} 

void WINAPI
service_main(DWORD argc, LPTSTR *argv)
{ 
    int op;
    s_char tbuf[256];
    DWORD status;
    char *config_file = NULL;

    optind = 1;
    opterr = 1;
    while ((op = getopt(argc, argv, "D:e:")) != EOF) {
	switch (op) {
	case 'D':
		datadir = optarg;
		break;
	case 'e':
		config_file = optarg;
		break;
	}
    }  

    if (config_file == NULL) {
	sprintf(tbuf, "%s/econfig", datadir);
	config_file = tbuf;
    }
    emp_config(config_file);

    service_status.dwServiceType        = SERVICE_WIN32; 
    service_status.dwCurrentState       = SERVICE_START_PENDING; 
    service_status.dwControlsAccepted   = SERVICE_ACCEPT_STOP; 
    service_status.dwWin32ExitCode      = 0; 
    service_status.dwServiceSpecificExitCode = 0; 
    service_status.dwCheckPoint         = 0; 
    service_status.dwWaitHint           = 0; 
 
    service_status_handle = RegisterServiceCtrlHandler(
        DEFAULT_SERVICE_NAME, service_ctrl_handler);
 
    if (service_status_handle == (SERVICE_STATUS_HANDLE)0) { 
        logerror("RegisterServiceCtrlHandler failed %d\n", GetLastError()); 
        return; 
    }
 
    /* Initialization code goes here. */
    init_server(0);
    start_server(0);
 
    /* Initialization complete - report running status. */
    service_status.dwCurrentState       = SERVICE_RUNNING; 
    service_status.dwCheckPoint         = 0; 
    service_status.dwWaitHint           = 0; 
 
    if (!SetServiceStatus (service_status_handle, &service_status)) { 
        status = GetLastError();
        logerror("SetServiceStatus error %ld\n",status);
    }

    empth_exit();

/* We should never get here.  But, just in case... */
    close_files();

    loc_NTTerm();

    // This is where the service does its work.
    logerror("Returning the Main Thread \n",0);
    return;
}

int
service_stopped(void)
{
    if (service_status.dwCurrentState == SERVICE_STOPPED)
	return 1;
    else
	return 0;
}

#endif /* _WIN32 */
