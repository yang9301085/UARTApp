#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "util.h"
#include "uart.h"

int str_to_int(uart_cfg_type type, const char *s)
{
    if (!s)
    {
        fprintf(stderr,"input null\n");
        return -1;
    }

    if (s[0] == '\0')
    {
        fprintf(stderr,"input empty\n");
        return -1;
    }
    int index = 0;
    int result = 0;

    while(s[index] != '\0')
    {
        if (s[index] < '0' || s[index] > '9')
        {
        fprintf(stderr, "str_to_int: invalid character '%c'\n", s[index]);
        return -1;
        }
        result=result*10+(s[index]-'0');
        index++;
    }

    switch (type)
    {
    case UART_BAUD:
    {
        if (result <= 0)
        {
            fprintf(stderr, "str_to_int: invalid baud %d\n", result);
            return -1;
        }
        return result;
    }
    case UART_DATA_BIT:
    {
        if (result < 5 || result > 8)
        {
            fprintf(stderr, "str_to_int: data_bits must be 5~8, got %d\n", result);
            return -1;
        }
        return result;
    }
    case UART_STOP_BIT:
    {
        if (result != 1 && result != 2)
        {
            fprintf(stderr, "str_to_int: stop_bits must be 1 or 2, got %d\n", result);
            return -1;
        }
        return result;
    }
    default:
        break;
    }
    return result;
}

FLOWCTL str_to_FLOWCTL(const char *s)
{
    if (!s)
    {
        fprintf(stderr,"input null\n");
        return setDefaultFLOWCTLtoFLOW_NONE();
    }

    if (s[0] == '\0')
    {
        fprintf(stderr,"input empty\n");
        return setDefaultFLOWCTLtoFLOW_NONE();
    }

    int result=str_to_int(UART_FLCTL,s);

    if(result>2||result<0)
    {
        fprintf(stderr,"input flowctl error,set to FLOW_NONE\n");
        return setDefaultFLOWCTLtoFLOW_NONE();
    }

    switch (result)
    {
        case 1:
            return FLOW_HW;
        case 2:
            return FLOW_SW;
        default:
            return setDefaultFLOWCTLtoFLOW_NONE();
    }
    return setDefaultFLOWCTLtoFLOW_NONE();
}

char str_to_parity(const char *s)
{
    if (!s || s[0] == '\0')
        return setDefaultParitytoN();

    if (strchr("nNeEoO", s[0]) == NULL)
        return setDefaultParitytoN();
    
    return toupper(s[0]);                  
}


char setDefaultParitytoN()
{
    return 'N';
}

FLOWCTL setDefaultFLOWCTLtoFLOW_NONE()
{
    return FLOW_NONE;
}