/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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

#include <config.h>

#ifdef _WIN32
#include <windows.h>

#include "prototypes.h"
#include "service.h"
#include "optlist.h"

int
install_service(char *program_name, char *service_name, char *config_file)
{
    HANDLE schSCManager,schService;
    SERVICE_DESCRIPTION sdBuf;

    if (config_file != NULL)
        _snprintf(&program_name[strlen(program_name)], _MAX_PATH-strlen(program_name), " -e %s",
	    config_file);

    if (service_name == NULL)
	service_name = DEFAULT_SERVICE_NAME;
    else if (service_name[0] == '\0')
	service_name = DEFAULT_SERVICE_NAME;

    schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

    if (schSCManager == NULL) {
        fprintf(stderr, "install_service failed to open Service Control Manager\n"); 
	return EXIT_FAILURE;
    }

    schService = CreateService(schSCManager,
    	service_name,
    	service_name,			/* service name to display */
        SERVICE_ALL_ACCESS,		/* desired access */
        SERVICE_WIN32_OWN_PROCESS,	/* service type */
        SERVICE_AUTO_START,		/* start type */
        SERVICE_ERROR_NORMAL,		/* error control type */
        program_name,			/* service's binary */
        NULL,				/* no load ordering group */
        NULL,				/* no tag identifier */
        NULL,				/* database service dependency */
        NULL,				/* LocalSystem account */
        NULL);				/* no password */
 
    if (schService == NULL) {
	fprintf(stderr, "install_service failed to create service %s\n", service_name);
        return EXIT_FAILURE;
    }
    sdBuf.lpDescription = "Server for Empire game";

    if(!ChangeServiceConfig2(
          schService,                 /* handle to service */
          SERVICE_CONFIG_DESCRIPTION, /* change: description */
          &sdBuf)) {                  /* value: new description */
        fprintf(stderr, "install_service failed to set the description\n");
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
        fprintf(stderr, "remove_service failed to open Service Control Manager\n"); 
	return EXIT_FAILURE;
    }

    hService = OpenService(schSCManager, service_name, SERVICE_ALL_ACCESS);

    if (hService == NULL) {
        fprintf(stderr, "remove_service failed to open service %s\n", service_name);
	return EXIT_FAILURE;
    }

    if (DeleteService(hService) == 0) {
        fprintf(stderr, "remove_service failed to remove service %s\n", service_name);
	return EXIT_FAILURE;
    }

    if (CloseServiceHandle(hService) == 0) {
        fprintf(stderr, "remove_service failed to close service %s\n", service_name); 
        return EXIT_FAILURE;
    } else {
        printf("Service %s removed.\n", service_name); 
        return EXIT_SUCCESS;
    }
}

static SERVICE_STATUS		service_status; 
static SERVICE_STATUS_HANDLE	service_status_handle;

static void WINAPI
service_ctrl_handler(DWORD Opcode) 
{ 
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
	    logerror("Service stopping");
	    empth_request_shutdown();
            return; 
 
        case SERVICE_CONTROL_INTERROGATE: 
        /* Fall through to send current status.  */
            break; 
 
        default: 
            logerror("Unrecognized opcode %ld in ServiceCtrlHandler", 
                Opcode); 
    } 
 
    /* Send current status. */
    if (!SetServiceStatus (service_status_handle,  &service_status))
        logerror("SetServiceStatus error %ld",GetLastError()); 
    return; 
} 

void WINAPI
service_main(DWORD argc, LPTSTR *argv)
{
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
        logerror("RegisterServiceCtrlHandler failed %lu\n", GetLastError());
	finish_server();
        return;
    }
 
    start_server(0);
 
    /* Initialization complete - report running status. */
    service_status.dwCurrentState       = SERVICE_RUNNING; 
    service_status.dwCheckPoint         = 0; 
    service_status.dwWaitHint           = 0; 
 
    if (!SetServiceStatus (service_status_handle, &service_status)) { 
        logerror("SetServiceStatus error %ld\n", GetLastError());
    }

    empth_exit();

    CANT_HAPPEN("main thread terminated");
    finish_server();
}

void
stop_service(void)
{
    logerror("Service stopped");
    service_status.dwWin32ExitCode = 0; 
    service_status.dwCurrentState  = SERVICE_STOPPED; 
    service_status.dwCheckPoint    = 0; 
    service_status.dwWaitHint      = 0; 

    if (!SetServiceStatus (service_status_handle, &service_status)) 
	logerror("Error while stopping service SetServiceStatus"
	    " error %ld", GetLastError()); 
}

#endif /* _WIN32 */
