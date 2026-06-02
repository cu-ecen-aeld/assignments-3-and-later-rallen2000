#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{	
	
	openlog("System Log", LOG_PID, LOG_USER);
	
	if (argc != 3)
	{
		syslog(LOG_ERR, "Error: Incorrect number of arguments\n");
		return 1;
	}
	
	syslog(LOG_DEBUG, "Writing %s to %s.\n", argv[2], argv[1]);
	FILE * filePtr = fopen(argv[1], "w");
	
	if (filePtr == NULL)
	{
		syslog(LOG_ERR, "Error: Unable to create file\n");
		return 1;
	}
	
	fprintf(filePtr, "%s", argv[2]);
	
	fclose(filePtr);
	closelog();
	
	
	return 0;
}
