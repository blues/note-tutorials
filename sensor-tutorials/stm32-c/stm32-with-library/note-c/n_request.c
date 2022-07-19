/*!
 * @file n_request.c
 *
 * Written by Ray Ozzie and Blues Inc. team.
 *
 * Copyright (c) 2019 Blues Inc. MIT License. Use of this source code is
 * governed by licenses granted by the copyright holder including that found in
 * the
 * <a href="https://github.com/blues/note-c/blob/master/LICENSE">LICENSE</a>
 * file.
 *
 */

#include "n_lib.h"

// For flow tracing
static int suppressShowTransactions = 0;

// Flag that gets set whenever an error occurs that should force a reset
static bool resetRequired = true;

/**************************************************************************/
/*!
    @brief  Create an error response document.
    @param   errmsg
               The error message from the Notecard
	@returns a `J` cJSON object with the error response.
*/
/**************************************************************************/
static J *errDoc(const char *errmsg) {
    J *rspdoc = JCreateObject();
    if (rspdoc != NULL) {
        JAddStringToObject(rspdoc, c_err, errmsg);
	}
	if (suppressShowTransactions == 0) {
	    _Debug("{\"err\":\"");
		_Debug(errmsg);
		_Debug("\"}\n");
	}
    return rspdoc;
}

/**************************************************************************/
/*!
    @brief  Suppress showing transaction details.
*/
/**************************************************************************/
void NoteSuspendTransactionDebug() {
	suppressShowTransactions++;
}

/**************************************************************************/
/*!
    @brief  Resume showing transaction details.
*/
/**************************************************************************/
void NoteResumeTransactionDebug() {
	suppressShowTransactions--;
}

/**************************************************************************/
/*!
    @brief  Create a new request object to populate before sending to the Notecard.
            Lock for mutual exclusion, not only because access to the card must be serialized, but also because
            both C++ and ArduinoJSON call malloc() which is not a thread-safe operation.
    @param   request
               The name of the request, for example `hub.set`.
	@returns a `J` cJSON object with the request name pre-populated.
*/
/**************************************************************************/
J *NoteNewRequest(const char *request) {
    J *reqdoc = JCreateObject();
    if (reqdoc != NULL)
        JAddStringToObject(reqdoc, c_req, request);
    return reqdoc;
}

/**************************************************************************/
/*!
    @brief  Send a request to the Notecard.
            Frees the request structure from memory after sending the request.
    @param   req
               The `J` cJSON request object.
	@returns a boolean. Returns `true` if successful or `false` if an error
            occurs, such as an out-of-memory or if an error was returned from
            the transaction in the c_err field.
*/
/**************************************************************************/
bool NoteRequest(J *req) {
    // Exit if null request.  This allows safe execution of the form NoteRequest(NoteNewRequest("xxx"))
    if (req == NULL)
        return false;
    // Execute the transaction
    J *rsp = NoteTransaction(req);
    if (rsp == NULL) {
        JDelete(req);
        return false;
    }
    // Check for a transaction error, and exit
    bool success = JIsNullString(rsp, c_err);
    JDelete(req);
    JDelete(rsp);
    return success;
}

/**************************************************************************/
/*!
    @brief  Send a request to the Notecard and return the response.
            Frees the request structure from memory after sending the request.
    @param   req
               The `J` cJSON request object.
	@returns a `J` cJSON object with the response, or NULL if there is
             insufficient memory.
*/
/**************************************************************************/
J *NoteRequestResponse(J *req) {
    // Exit if null request.  This allows safe execution of the form NoteRequestResponse(NoteNewRequest("xxx"))
    if (req == NULL)
        return NULL;
    // Execute the transaction
    J *rsp = NoteTransaction(req);
    if (rsp == NULL) {
        JDelete(req);
        return NULL;
    }
    // Free the request and exit
    JDelete(req);
    return rsp;
}

/**************************************************************************/
/*!
    @brief  Given a JSON string, send a request to the Notecard.
            Frees the request structure from memory after sending the request.
    @param   reqJSON
               A c-string containing the JSON request object.
	@returns a c-string with the JSON response from the Notecard. After
             parsed by the developer, should be freed with `JFree`.
*/
/**************************************************************************/
char *NoteRequestResponseJSON(char *reqJSON) {

    // Parse the incoming JSON string
    J *req = JParse(reqJSON);
    if (req == NULL)
        return NULL;

    // Perform the transaction and free the req
    J *rsp = NoteRequestResponse(req);
    if (rsp == NULL)
        return NULL;

    // Convert response back to JSON and delete it
    char *json = JPrintUnformatted(rsp);
    NoteDeleteResponse(rsp);
    if (json == NULL)
        return NULL;

    // Done
    return json;

}

/**************************************************************************/
/*!
    @brief  Initiate a transaction to the Notecard and return the response.
            Does NOT free the request structure from memory after sending
            the request.
    @param   req
               The `J` cJSON request object.
	@returns a `J` cJSON object with the response, or NULL if there is
             insufficient memory.
*/
/**************************************************************************/
J *NoteTransaction(J *req) {

    // If a reset of the module is required for any reason, do it now.
    // We must do this before acquiring lock.
    if (resetRequired) {
        if (!NoteReset())
            return NULL;
    }

    // Lock
    _LockNote();

    // Serialize the JSON requet
    char *json = JPrintUnformatted(req);
    if (json == NULL) {
        J *rsp = errDoc(ERRSTR("can't convert to JSON",c_bad));
        _UnlockNote();
        return rsp;
    }
    
	if (suppressShowTransactions == 0) {
	    _Debugln(json);
	}

    // Pertform the transaction
    char *responseJSON;
    const char *errStr = _Transaction(json, &responseJSON);

    // Free the json
    JFree(json);

    // If error, queue up a reset
    if (errStr != NULL) {
		NoteResetRequired();
        J *rsp = errDoc(errStr);
        _UnlockNote();
        return rsp;
    }

    // Parse the reply from the card on the input stream
    J *rspdoc = JParse(responseJSON);
    if (rspdoc == NULL) {
        _Debug("invalid JSON: ");
		_Debug(responseJSON);
        _Free(responseJSON);
        J *rsp = errDoc(ERRSTR("unrecognized response from card",c_bad));
        _UnlockNote();
        return rsp;
    }

    // Debug
	if (suppressShowTransactions == 0) {
	    _Debugln(responseJSON);
	}

    // Discard the buffer now that it's parsed
    _Free(responseJSON);

    // Unlock
    _UnlockNote();

    // Done
    return rspdoc;
    
}

/**************************************************************************/
/*!
    @brief  Mark that a reset will be required before doing further I/O on
            a given port.
*/
/**************************************************************************/
void NoteResetRequired() {
    resetRequired = true;
}

/**************************************************************************/
/*!
    @brief  Initialize or re-initialize the module, returning false if
            anything fails.
    @returns a boolean. `true` if the reset was successful, `false`, if not.
*/
/**************************************************************************/
bool NoteReset() {
    _LockNote();
    resetRequired = !_Reset();
    _UnlockNote();
    return !resetRequired;
}

/**************************************************************************/
/*!
    @brief  Check to see if a Notecard error is present in a JSON string.
    @param   errstr
               The error string.
    @param   errtype
               The error type string.
	@returns boolean. `true` if the string contains the error provided, `false`
             if not.
*/
/**************************************************************************/
bool NoteErrorContains(const char *errstr, const char *errtype) {
    return (strstr(errstr, errtype) != NULL);
}

/**************************************************************************/
/*!
    @brief  Clean error strings out of the specified buffer.
    @param   begin
               The string buffer to clear of error strings.
*/
/**************************************************************************/
void NoteErrorClean(char *begin) {
    while (true) {
        char *end = &begin[strlen(begin)+1];
        char *beginBrace = strchr(begin, '{');
        if (beginBrace == NULL)
            break;
        if (beginBrace>begin && *(beginBrace-1) == ' ')
            beginBrace--;
        char *endBrace = strchr(beginBrace, '}');
        if (endBrace == NULL)
            break;
        char *afterBrace = endBrace + 1;
        memmove(beginBrace, afterBrace, end-afterBrace);
    }
}
