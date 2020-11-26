#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "command.h"

int do_help(struct cmd_table *table,int argc,char *const argv[])
{
    return 0;
}

static cmd_tbl_t g_cmds[] =
{
    {
        "help",do_help
    }
};

int cmd_process(int argc,char *argv[])
{
    int pos = 0;
    for(pos = 0; pos < sizeof(g_cmds) / sizeof(g_cmds[0]); pos++)
    {
        if(strcmp(argv[0],g_cmds[pos].name) == 0)
        {
            return g_cmds[pos].cmd(&g_cmds[pos],argc,argv);
        }
    }
    return 1; /* error */
}

int readline(char *buffer)
{
    char *p = buffer;
    char *p_buf = p;
    int n = 0;
    int plen = 0;
    int col;
    char c;

    /* print prompt */
    const char *prompt = "Cmd# ";
    if(prompt)
    {
        plen = strlen(prompt);
        printf("%s",prompt);
    }
    col = plen;

    for(;;)
    {
        c = getchar();
        switch(c)
        {
        case '\r':
        case '\n':
            *p = '\0';
            printf("\r\n");
            return (p - p_buf);
        case '\0':
            continue;
        case 0x03:      /* ^C break*/
            p_buf[0] = '\0';
            return (-1);
        default:
            /* must be a normal character then */
            if(n < 512 - 2)
            {
                if(c == '\t')
                {

                }
                else
                {
                    ++col; /* echo input */
                    putchar(c);
                }
                *p++ = c;
                ++n;
            }
            else   /* buffer is full */
            {
                putchar('\a');
            }
        }
    }
    return n;
}

int parse_line(char *line,char *argv[])
{
    int nargs = 0;

    printf ("parse_line: \"%s\"\n", line);

    while (nargs < 512)
    {

        /* skip any white space */
        while (isblank(*line))
            ++line;

        if (*line == '\0')  	/* end of line, no more args	*/
        {
            argv[nargs] = NULL;

            printf ("parse_line: nargs=%d\n", nargs);

            return (nargs);
        }

        argv[nargs++] = line;	/* begin of argument string	*/

        /* find end of string */
        while (*line && !isblank(*line))
            ++line;

        if (*line == '\0')  	/* end of line, no more args	*/
        {
            argv[nargs] = NULL;

            printf ("parse_line: nargs=%d\n", nargs);

            return (nargs);
        }

        *line++ = '\0';		/* terminate current arg	 */
    }

    printf ("** Too many args (max. %d) **\n", 512);

    printf ("parse_line: nargs=%d\n", nargs);

    return (nargs);
}

void run_command(char *cmd)
{
    char *argv[ARGVS_MAX + 1];
    int argc;
    argc = parse_line(cmd,argv);

    cmd_process(argc,argv);
}

void main_loop()
{
    int len;
    char buffer[CONSOLE_BUFFER_LEN] = "";

    for(;;)
    {
        len = readline(buffer);
        if(len > 0)
        {
            run_command(buffer);
            memset(buffer,0x00,sizeof(buffer));
        }
    }
}
