/*******************************************************************************************************/
/**
*   @file         SimpleJsonExample.c
*   @addtogroup   json
*   @brief        Example of json parsing with jsmn.
*   @author       Wenderson Oliveira
*   @details      This is a simple example of a way to use jsmn to parse and get real data from a json
*                 file. The example has different json inputs, selectable from dJSON_INPUT define. Also,
*                 you can use dynamic allocation (malloc) or a simple RAM buffer to run jsmn. This feature
*                 is enable in dEXAMPLE_WITH_DYNAMIC_ALLOC define.
*
*   @version      1.0.0
*
*   @copyright    https://github.com/programacao-eletronica/jsmn
*   @{
********************************************************************************************************/

/********************************************************************************************************
*   INCLUDES
********************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "string.h"

#include "..\..\jsmn.h"

/********************************************************************************************************
*   EXAMPLE SETTINGS
********************************************************************************************************/

/// This number should be enough to parse all tokens in the desired json input. Read more about it at 
/// jsmn documentation.
#define dMAX_EXPECTED_JSON_TOKENS       64     
    
/// Enables or disables dynamic alloc
#define dEXAMPLE_WITH_DYNAMIC_ALLOC     true

/// Select the json input. Right now, we have 3 different samples:
/// 1 - Json with all data in the expected order
/// 2 - Json with all tada, but not in the expected order (wich works fine too)
/// 3 - Json with incomplete data
#define dJSON_INPUT                     1       

/********************************************************************************************************
*   EXAMPLE DATA
********************************************************************************************************/
#if dJSON_INPUT == 1
const char jsonString[] =
"{\r\n\
  \"Specs\": \r\n\
  {\r\n\
    \"Range\": \r\n\
    {\r\n\
      \"voltage\": [\"-30\",\"750\"],\r\n\
      \"current\": [\"0\", \"20\"]\r\n\
    },\r\n\
    \"colors\": [\"blue\", \"black\"]\r\n\
  },\r\n\
  \"Test\":true\r\n\
}";
#elif dJSON_INPUT == 2
const char jsonString[] =
"{\r\n\
  \"Specs\": \r\n\
  {\r\n\
    \"colors\": [\"orange\", \"black\"],\r\n\
    \"Range\": \r\n\
    {\r\n\
      \"current\": [\"10\", \"45\"]\r\n\
      \"voltage\": [\"-35\",\"150\"],\r\n\
    }\r\n\
  },\r\n\
  \"Test\":true\r\n\
}";
#elif dJSON_INPUT == 3
const char jsonString[] =
"{\r\n\
  \"Settings\": \r\n\
  {\r\n\
    \"colors\": [\"orange\", \"black\"],\r\n\
    \"Range\": \r\n\
    {\r\n\
      \"current\": [\"10\", \"45\"]\r\n\
      \"voltage\": [\"-35\",\"150\"],\r\n\
    }\r\n\
  },\r\n\
  \"TestData\":true\r\n\
}";
#else
    #error "Define a json input from 1 to 3!"
#endif

#if dEXAMPLE_WITH_DYNAMIC_ALLOC == false
jsmntok_t jsonTokens[sizeof(jsmntok_t) * dMAX_EXPECTED_JSON_TOKENS] = { 0 };
#endif

/*******************************************************************************************************/
/** @brief      Entry point.
*   @param      None.
*   @retval     None.
********************************************************************************************************/
int main()
{
    // Info that will be extracted from json
    int voltageMin = 0;
    int voltageMax = 0;
    int currentMin = 0;
    int currentMax = 0;
    char colors[2][64] = { 0 };
    bool test = false;

    // Print the selected json file
    printf("Json file:\r\n\r\n");
    printf(jsonString);
    printf("\r\n");

#if dEXAMPLE_WITH_DYNAMIC_ALLOC == true
    // Allocate the memory area for jsmn to run
    // OBS: if you do this plenty of times in your code, you can use dynamic allocation or
    // a fixed area. If you go the fixed area approach, just remember to memset(0) the working
    // buffer before use it
    jsmntok_t* jsonTokens = malloc(sizeof(jsmntok_t) * dMAX_EXPECTED_JSON_TOKENS);
#endif

    // The interpretation of json string by jsmn originally uses a go-to approach. To avoid it, we
    // will use a do-while(false) architecture just to be able to "break" anywere.
    do
    {
#if dEXAMPLE_WITH_DYNAMIC_ALLOC == true
        // Check if the malloc has done correctly
        if (jsonTokens == NULL)
        {
            printf("\r\nMalloc error of json parser");
            break;
        }
#endif

        // Initialization of the library
        jsmn_parser jsonParser_all;
        jsmn_init(&jsonParser_all);

        // Verifies how much json tokens are in the jsonString
        int tokens = jsmn_parse(&jsonParser_all, jsonString, strlen(jsonString), jsonTokens, dMAX_EXPECTED_JSON_TOKENS);

        // Handle error case
        if (tokens < 0)
        {
            printf("\r\nThis is not a json file or it is corrupted.");
            break;
        } // And also the empty case
        else if (tokens == 1 || jsonTokens[0].type != JSMN_OBJECT)
        {
            printf("\r\nThis json file is empty.");
            break;
        }
        else // Continue if is a valid json
        {
            printf("\r\nThis is a valid json file!");
        }

        // Here, the hunting for known tokens begins!
        // The start index is 1 because the token 0 is the json object itself
        for (int i = 1; i < tokens; i++)
        {
            // Search in jsonString, at the token position, if the "key" is equal "Specs".
            // Also, because we are expecting that the "Specs" key is a object, this tests the type as JSMN_OBJECT
            if (json_equals(jsonString, &jsonTokens[i], "Specs") && (jsonTokens[i + 1].type == JSMN_OBJECT))
            {
                int specsObjEnd = jsonTokens[i + 1].end;

                // Perform the parsing inside a json object, using the same index. Just "jump" inside de object,
                // using the "i + 2" index.
                for (i = i + 2; i < tokens; i++)
                {
                    // If we reach the end of the "Specs" object, then we are out of it. So, continue the rest of parsing
                    if (jsonTokens[i].start >= specsObjEnd)
                    {
                        // Decrements the index to avoid double increment from the couple of for loops.
                        i--;
                        break;
                    }

                    // Search for the "voltage" array
                    if (json_equals(jsonString, &jsonTokens[i], "voltage") && (jsonTokens[i + 1].type == JSMN_ARRAY))
                    {
                        // Jumps inside the voltage array
                        i = i + 2;

                        voltageMin = atoi(jsonString + jsonTokens[i].start);
                        i++;
                        voltageMax = atoi(jsonString + jsonTokens[i].start);
                    }

                    // Search for the "current" array
                    if (json_equals(jsonString, &jsonTokens[i], "current") && (jsonTokens[i + 1].type == JSMN_ARRAY))
                    {
                        // Jumps inside the current array
                        i = i + 2;

                        currentMin = atoi(jsonString + jsonTokens[i].start);
                        i++;
                        currentMax = atoi(jsonString + jsonTokens[i].start);
                    }

                    // Search for the "colors" array
                    if (json_equals(jsonString, &jsonTokens[i], "colors") && (jsonTokens[i + 1].type == JSMN_ARRAY))
                    {
                        // Jumps inside the colors array
                        i = i + 2;

                        // Uses %.*s to specify string size
                        snprintf(colors[0], sizeof(colors[0]), "%.*s",
                            jsonTokens[i].end - jsonTokens[i].start, jsonString + jsonTokens[i].start);
                        i++;
                        snprintf(colors[1], sizeof(colors[0]), "%.*s",
                            jsonTokens[i].end - jsonTokens[i].start, jsonString + jsonTokens[i].start);
                    }
                }

                continue;
            }

            // Search in jsonString, at the token position, if the "key" is equal "Test"
            if (json_equals(jsonString, &jsonTokens[i], "Test") && (jsonTokens[i + 1].type == JSMN_PRIMITIVE))
            {
                // Copy the "true" or "false" first char (t or f) to verify the boolean value
                char firstChar = *(char*)(jsonString + jsonTokens[i + 1].start);

                // The suggested approach for boolean vars is to check the first char of them
                if (firstChar == 't')
                {
                    test = true;
                }

                continue;
            }
        }
    } while (false);

#if dEXAMPLE_WITH_DYNAMIC_ALLOC == true
    // Check if the malloc has done correctly
    if (jsonTokens != NULL)
    {
        free(jsonTokens);
    }
#endif

    // Print the results
    printf("\r\n");
    printf("\r\n===== Results =====");
    printf("\r\nVoltage: [%d, %d]", voltageMin, voltageMax);
    printf("\r\nCurrent: [%d, %d]", currentMin, currentMax);
    printf("\r\nColors: [%s, %s]", colors[0], colors[1]);
    
    if (test)
    {
        printf("\r\nTest: true");
    }

    // Avoids console window terminate.
    getch();

    return 0;
}

/** @} DOXYGEN GROUP TAG END OF FILE */