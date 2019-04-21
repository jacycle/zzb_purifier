#include "configcheck.h"

//*****************************************************************************
//
//! \internal
//!
//! Searches the list of parameters passed to a CGI handler and returns the
//! index of a given parameter within that list.
//!
//! \param pcToFind is a pointer to a string containing the name of the
//! parameter that is to be found.
//! \param pcParam is an array of character pointers, each containing the name
//! of a single parameter as encoded in the URI requesting the CGI.
//! \param iNumParams is the number of elements in the pcParam array.
//!
//! This function searches an array of parameters to find the string passed in
//! \e pcToFind.  If the string is found, the index of that string within the
//! \e pcParam array is returned, otherwise -1 is returned.
//!
//! \return Returns the index of string \e pcToFind within array \e pcParam
//! or -1 if the string does not exist in the array.
//
//*****************************************************************************
int
ConfigFindCGIParameter(const char *pcToFind, char *pcParam[], int iNumParams)
{
    int iLoop;

    //
    // Scan through all the parameters in the array.
    //
    for(iLoop = 0; iLoop < iNumParams; iLoop++)
    {
        //
        // Does the parameter name match the provided string?
        //
        if(strcmp(pcToFind, pcParam[iLoop]) == 0)
        {
            //
            // We found a match - return the index.
            //
            return(iLoop);
        }
    }

    //
    // If we drop out, the parameter was not found.
    //
    return(-1);
}

//*****************************************************************************
//
//! \internal
//!
//! Ensures that a string passed represents a valid decimal number and,
//! if so, converts that number to a long.
//!
//! \param pcValue points to a null terminated string which should contain an
//! ASCII representation of a decimal number.
//! \param plValue points to storage which will receive the number represented
//! by pcValue assuming the string is a valid decimal number.
//!
//! This function determines whether or not a given string represents a valid
//! decimal number and, if it does, converts the string into a decimal number
//! which is returned to the caller.
//!
//! \return Returns \b true if the string is a valid representation of a
//! decimal number or \b false if not.

//*****************************************************************************
static tBoolean
ConfigCheckDecimalParam(const char *pcValue, long *plValue)
{
    unsigned long ulLoop;
    tBoolean bStarted;
    tBoolean bFinished;
    tBoolean bNeg;
    long lAccum;

    //
    // Check that the string is a valid decimal number.
    //
    bStarted = false;
    bFinished = false;
    bNeg = false;
    ulLoop = 0;
    lAccum = 0;

    while(pcValue[ulLoop])
    {
        //
        // Ignore whitespace before the string.
        //
        if(!bStarted)
        {
            if((pcValue[ulLoop] == ' ') || (pcValue[ulLoop] == '\t'))
            {
                ulLoop++;
                continue;
            }

            //
            // Ignore a + or - character as long as we have not started.
            //
            if((pcValue[ulLoop] == '+') || (pcValue[ulLoop] == '-'))
            {
                //
                // If the string starts with a '-', remember to negate the
                // result.
                //
                bNeg = (pcValue[ulLoop] == '-') ? true : false;
                bStarted = true;
                ulLoop++;
            }
            else
            {
                //
                // We found something other than whitespace or a sign character
                // so we start looking for numerals now.
                //
                bStarted = true;
            }
        }

        if(bStarted)
        {
            if(!bFinished)
            {
                //
                // We expect to see nothing other than valid digit characters
                // here.
                //
                if((pcValue[ulLoop] >= '0') && (pcValue[ulLoop] <= '9'))
                {
                    lAccum = (lAccum * 10) + (pcValue[ulLoop] - '0');
                }
                else
                {
                    //
                    // Have we hit whitespace?  If so, check for no more
                    // characters until the end of the string.
                    //
                    if((pcValue[ulLoop] == ' ') || (pcValue[ulLoop] == '\t'))
                    {
                        bFinished = true;
                    }
                    else
                    {
                        //
                        // We got something other than a digit or whitespace so
                        // this makes the string invalid as a decimal number.
                        //
                        return(false);
                    }
                }
            }
            else
            {
                //
                // We are scanning for whitespace until the end of the string.
                //
                if((pcValue[ulLoop] != ' ') && (pcValue[ulLoop] != '\t'))
                {
                    //
                    // We found something other than whitespace so the string
                    // is not valid.
                    //
                    return(false);
                }
            }

            //
            // Move on to the next character in the string.
            //
            ulLoop++;
        }
    }

    //
    // If we drop out of the loop, the string must be valid.  All we need to do
    // now is negate the accumulated value if the string started with a '-'.
    //
    *plValue = bNeg ? -lAccum : lAccum;
    return(true);
}

static tBoolean
ConfigCheckHexadecimalParam(const char *pcValue, long *plValue)
{
    unsigned long ulLoop;
    tBoolean bStarted;
    tBoolean bFinished;
    tBoolean bNeg;
    long lAccum;

    //
    // Check that the string is a valid decimal number.
    //
    bStarted = false;
    bFinished = false;
    bNeg = false;
    ulLoop = 0;
    lAccum = 0;

    while(pcValue[ulLoop])
    {
        //
        // Ignore whitespace before the string.
        //
        if(!bStarted)
        {
            if((pcValue[ulLoop] == ' ') || (pcValue[ulLoop] == '\t'))
            {
                ulLoop++;
                continue;
            }

            //
            // Ignore a + or - character as long as we have not started.
            //
            if((pcValue[ulLoop] == '+') || (pcValue[ulLoop] == '-'))
            {
                //
                // If the string starts with a '-', remember to negate the
                // result.
                //
                bNeg = (pcValue[ulLoop] == '-') ? true : false;
                bStarted = true;
                ulLoop++;
            }
            else
            {
                //
                // We found something other than whitespace or a sign character
                // so we start looking for numerals now.
                //
                bStarted = true;
            }
        }

        if(bStarted)
        {
            if(!bFinished)
            {
                //
                // We expect to see nothing other than valid digit characters
                // here.
                //
                if((pcValue[ulLoop] >= '0') && (pcValue[ulLoop] <= '9'))
                {
                    lAccum = (lAccum * 16) + (pcValue[ulLoop] - '0');
                }
                else if((pcValue[ulLoop] >= 'a') && (pcValue[ulLoop] <= 'f'))
                {
                    lAccum = (lAccum * 16) + (pcValue[ulLoop] - 'a') + 10;
                }
                else if((pcValue[ulLoop] >= 'A') && (pcValue[ulLoop] <= 'F'))
                {
                    lAccum = (lAccum * 16) + (pcValue[ulLoop] - 'A') + 10;
                }
                else
                {
                    //
                    // Have we hit whitespace?  If so, check for no more
                    // characters until the end of the string.
                    //
                    if((pcValue[ulLoop] == ' ') || (pcValue[ulLoop] == '\t'))
                    {
                        bFinished = true;
                    }
                    else
                    {
                        //
                        // We got something other than a digit or whitespace so
                        // this makes the string invalid as a decimal number.
                        //
                        return(false);
                    }
                }
            }
            else
            {
                //
                // We are scanning for whitespace until the end of the string.
                //
                if((pcValue[ulLoop] != ' ') && (pcValue[ulLoop] != '\t'))
                {
                    //
                    // We found something other than whitespace so the string
                    // is not valid.
                    //
                    return(false);
                }
            }

            //
            // Move on to the next character in the string.
            //
            ulLoop++;
        }
    }

    //
    // If we drop out of the loop, the string must be valid.  All we need to do
    // now is negate the accumulated value if the string started with a '-'.
    //
    *plValue = bNeg ? -lAccum : lAccum;
    return(true);
}

//*****************************************************************************
//
//! \internal
//!
//! Searches the list of parameters passed to a CGI handler for a parameter
//! with the given name and, if found, reads the parameter value as a decimal
//! number.
//!
//! \param pcName is a pointer to a string containing the name of the
//! parameter that is to be found.
//! \param pcParam is an array of character pointers, each containing the name
//! of a single parameter as encoded in the URI requesting the CGI.
//! \param iNumParams is the number of elements in the pcParam array.
//! \param pcValues is an array of values associated with each parameter from
//! the pcParam array.
//! \param pbError is a pointer that will be written to \b true if there is any
//! error during the parameter parsing process (parameter not found, value is
//! not a valid decimal number).
//!
//! This function searches an array of parameters to find the string passed in
//! \e pcName.  If the string is found, the corresponding parameter value is
//! read from array pcValues and checked to make sure that it is a valid
//! decimal number.  If so, the number is returned.  If any error is detected,
//! parameter \e pbError is written to \b true.  Note that \e pbError is NOT
//! written if the parameter is successfully found and validated.  This is to
//! allow multiple parameters to be parsed without the caller needing to check
//! return codes after each individual call.
//!
//! \return Returns the value of the parameter or 0 if an error is detected (in
//! which case \e *pbError will be \b true).
//
//*****************************************************************************
long
ConfigGetCGIParamDec(const char *pcName, char *pcParams[], char *pcValue[],
                  int iNumParams, tBoolean *pbError)
{
    int iParam;
    long lValue;
    tBoolean bRetcode;

    //
    // Is the parameter we are looking for in the list?
    //
    lValue = 0;
    iParam = ConfigFindCGIParameter(pcName, pcParams, iNumParams);
    if(iParam != -1)
    {
        //
        // We found the parameter so now get its value.
        //
        bRetcode = ConfigCheckDecimalParam(pcValue[iParam], &lValue);
        if(bRetcode)
        {
            //
            // All is well - return the parameter value.
            //
            return(lValue);
        }
    }

    //
    // If we reach here, there was a problem so return 0 and set the error
    // flag.
    //
    *pbError = true;
    return(0);
}

long
ConfigGetCGIParamHex(const char *pcName, char *pcParams[], char *pcValue[],
                  int iNumParams, tBoolean *pbError)
{
    int iParam;
    long lValue;
    tBoolean bRetcode;

    //
    // Is the parameter we are looking for in the list?
    //
    lValue = 0;
    iParam = ConfigFindCGIParameter(pcName, pcParams, iNumParams);
    if(iParam != -1)
    {
        //
        // We found the parameter so now get its value.
        //
        bRetcode = ConfigCheckHexadecimalParam(pcValue[iParam], &lValue);
        if(bRetcode)
        {
            //
            // All is well - return the parameter value.
            //
            return(lValue);
        }
    }

    //
    // If we reach here, there was a problem so return 0 and set the error
    // flag.
    //
    *pbError = true;
    return(0);
}

int
extract_uri_parameters(char *params[], char *param_vals[], char *uri)
{
  char *pair;
  char *equals;
  int loop;

  /* If we have no parameters at all, return immediately. */
  if(!uri || (uri[0] == '\0')) {
      return(0);
  }

  /* Get a pointer to our first parameter */
  pair = uri;

  /*
   * Parse up to MAX_CGI_PARAMETERS from the passed string and ignore the
   * remainder (if any)
   */
  for(loop = 0; (loop < MAX_CGI_PARAMETERS) && pair; loop++) {

    /* Save the name of the parameter */
    params[loop] = pair;

    /* Remember the start of this name=value pair */
    equals = pair;

    /* Find the start of the next name=value pair and replace the delimiter
     * with a 0 to terminate the previous pair string.
     */
    pair = strchr(pair, '&');
    if(pair) {
      *pair = '\0';
      pair++;
    } else {
       /* We didn't find a new parameter so find the end of the URI and
        * replace the space with a '\0'
        */
        pair = strchr(equals, ' ');
        if(pair) {
            *pair = '\0';
        }

        /* Revert to NULL so that we exit the loop as expected. */
        pair = NULL;
    }

    /* Now find the '=' in the previous pair, replace it with '\0' and save
     * the parameter value string.
     */
    equals = strchr(equals, '=');
    if(equals) {
      *equals = '\0';
      param_vals[loop] = equals + 1;
    } else {
      param_vals[loop] = NULL;
    }
  }

  return(loop);
}

void str_trim(char* str, char ch)  
{  
    char *ptmp = str;  
    
    if (ptmp == NULL)
    {
        return;
    }
    while ((*str) != '\0')   
    {  
        if (*str != ch)  
        {  
            *ptmp++ = *str;  
        }  
        ++str;  
    }  
    (*ptmp) = '\0';  
}  

int
extract_json_parameters(char *params[], char *param_vals[], char *uri)
{
  char *pair;
  char *equals;
  int loop;

  /* If we have no parameters at all, return immediately. */
  if(!uri || (uri[0] == '\0')) {
      return(0);
  }

  /* Get a pointer to our first parameter */
  pair = (char *)strchr(uri, '{');
  if(!pair || (pair[0] == '\0')) {
      return(0);
  }
  pair++;
  str_trim(pair, '"');
  str_trim(pair, ' ');

  /*
   * Parse up to MAX_CGI_PARAMETERS from the passed string and ignore the
   * remainder (if any)
   */
  for(loop = 0; (loop < MAX_CGI_PARAMETERS) && pair; loop++) {

    /* Save the name of the parameter */
    params[loop] = pair;

    /* Remember the start of this name=value pair */
    equals = pair;

    /* Find the start of the next name=value pair and replace the delimiter
     * with a 0 to terminate the previous pair string.
     */
    pair = (char *)strchr(pair, ',');
    if(pair) {
      *pair = '\0';
      pair++;
    } else {
       /* We didn't find a new parameter so find the end of the URI and
        * replace the space with a '\0'
        */
      pair = (char *)strchr(equals, '}');
        if(pair) {
            *pair = '\0';
        }

        /* Revert to NULL so that we exit the loop as expected. */
        pair = NULL;
    }

    /* Now find the '=' in the previous pair, replace it with '\0' and save
     * the parameter value string.
     */
    equals = (char *)strchr(equals, ':');
    if(equals) {
      *equals = '\0';
      param_vals[loop] = equals + 1;
    } else {
      param_vals[loop] = NULL;
    }
  }

  return(loop);
}

