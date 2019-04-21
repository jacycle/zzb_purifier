#ifndef __CONFIG_CHECK__
#define __CONFIG_CHECK__

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Exported types ------------------------------------------------------------*/
#define MAX_CGI_PARAMETERS  10

/* Exported types ------------------------------------------------------------*/
//typedef unsigned char tBoolean;
typedef enum
{
  false = 0,
  true = 1,
}tBoolean;

/* Exported types ------------------------------------------------------------*/
int
extract_uri_parameters(char *params[], char *param_vals[], char *uri);
long
ConfigGetCGIParamHex(const char *pcName, char *pcParams[], char *pcValue[],
                  int iNumParams, tBoolean *pbError);
long
ConfigGetCGIParamDec(const char *pcName, char *pcParams[], char *pcValue[],
                  int iNumParams, tBoolean *pbError);
int
ConfigFindCGIParameter(const char *pcToFind, char *pcParam[], int iNumParams);

int
extract_json_parameters(char *params[], char *param_vals[], char *uri);

#endif
