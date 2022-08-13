// calc.c
// by Chris Minich

#include "cs50.h"
#include "cs50.c"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> // for chdir()

#define STACK_SIZE 9
#define MAX_INDEX 8
#define REGISTERS 10
#define TRACE true
#define NO_TRACE false
#define RETURN_STACK_SIZE 20
#define RETURN_MAX_INDEX 19
#define MAX_N 200
#define MAX_PRG_SIZE 200

void add(void);
void mult(void);
void subtract(void);
void divide(void);
void modulus(void);
void y_to_x(void);
void truncateX(void);
void abs_value(void);
void sto(int mode);
void rcl(int mode, bool pop);
void push(long double x);
long double pop(void);
void sto_push(int mem_num, long double num);
long double rcl_pop(int mem_num);
void isz(void);
void dsz(void);
void x_lt_0(void);
void x_gt_0(void);
void x_gt_1(void);
void x_ne_0(void);
void x_eq_y(void);
void x_gt_y(void);
void fixed_decimal(void);
void printfloat(long double r);
void printstack(void);
void clearstack(void);
void prtkeys(string keys[], int key_count);
int arr_size(string arr[]);
void read_prgm(void);
int parse(string entry, int prgm_mode);
void fst(int moves);
void bst(int moves);
void skip_1_step(void);
void lbl(int mode);
void mv_step(int steps);
int gto(int mode);
int gsb(int mode);
int rtn(void);
int return_push(int rtn_step);
int return_pop(void);
char* getBuffer(char prompt);

long double stack[STACK_SIZE];
int rtn_stack[RETURN_STACK_SIZE];
int stack_index = -1;
int rtn_index = -1;
long double x, y, i_reg;
long double n_reg[MAX_N];
int register_index[REGISTERS]; //since mem 0-9 are stacks, each one has an index
long double mem[REGISTERS][STACK_SIZE];
string entry;
string keys[] = { "STO", "RCL", "POP", "/", "*", "-", "+", "Y^X", "INT", "ABS", "%", "OFF", "STACK", "CLEAR", "KEYS", "LBL", "ISZ", "DSZ", "N[I]", "X<0", "X>0", "X>1", "X=Y", "X>Y", "X!=0" ,"GTO", "GSB", "HELP", "RTN", "R/S", "SST", "S", "BST", "B", "FST", "F", "FLOAT", "WHERE", "BEGIN", "LABELS", "LOAD", "SHELL", "Q" };
string prg[MAX_PRG_SIZE];
bool is_key;
long double num;
int fix_val = 2;
int step = 0;
bool silent = false; // interactive mode: print any results of interactive commands
int maxstep;
char* buffer; // for getBuffer()
int buffer_position = 0; // for getBuffer()
int buffer_length; // for getBuffer()
bool autofloat = true;
char* last_cmd = "";
char* usrprogram = "";
char* default_program = "default.txt";
bool program_loaded = false;

void help(void)
{
	char* strng = getBuffer('>');
	if ( strlen(strng) == 0 )
	{
		printf("Help is available for any of these commands:\n");
		int length = arr_size(keys);
		prtkeys(keys, length);
		strng = getBuffer('>');
	}
	char* cmd = malloc(sizeof(char) * strlen(strng) + 33);
	sprintf(cmd, "grep -iw \"%s:\" keylist.help | fold -s", strng);
/*	printf("$ %s\n", cmd);*/
	system(cmd);
}

void stringToUpper(char* strng)
{
	int length = strlen(strng);
	for ( int i = 0; i < length; i++ )
		if ( islower(strng[i]) ) strng[i] = toupper(strng[i]);
}

char* getBuffer(char prompt) // Get next string from buffer
{
	if ( buffer_position == -1 ) return buffer; // for commandline mode
	int start_position = buffer_position;
	int pos = buffer_position;
	if ( buffer_position == 0 ) // read a line from stdin
	{
		printf("%c ", prompt);
		buffer = GetString();
		buffer_length = strlen(buffer);
		if ( strcmp(buffer, "") == 0 )
		{
			if ( ( strcmp(last_cmd, "S") == 0 ) || 
			     ( strcmp(last_cmd, "F") == 0 ) || 
			     ( strcmp(last_cmd, "B") == 0 ) )
				return last_cmd;
		}
	}	
	int count = 0;
	do // find beginning of string
	{
		if ( ( buffer[pos] == ' ' ) || ( pos == buffer_length ) )
		{
			if ( pos < buffer_length ) buffer_position = pos + 1;
			else buffer_position = 0;
			break;
		}
		else
		{
			count++;
			pos++;
		}
	} while ( pos <= buffer_length );
	
	// return the string
	char* word1 = malloc(sizeof(char) * (count + 1));
	for ( int i = 0; i < count; i++) word1[i] = buffer[start_position + i];
	word1[count] = '\0';	
	last_cmd = word1;
	return word1;
}

void list_of_labels(void)
{
	for (int i=0; i<maxstep; i++)
	{
		if (strcmp(prg[i], "LBL") == 0)
			printf("LBL %s\n", prg[i+1]);
	}
}

void start(bool trace) // run user program
{
    int prgm_mode;
    if ( trace ) prgm_mode = 2;
    else prgm_mode = 1;
    string op;
    int success = 1; // for gto()
    int not_done;
    do
    {
        op = prg[step];
        if ( trace ) // print the commands
        {
        	silent = false;
            if (( strcmp(op, "STO") != 0 ) && ( strcmp(op, "RCL") != 0 )) // these print elsewhere
            {
                printf("%s ", op);
                if  (
                    ( strcmp(op, "LBL") != 0 ) && 
                    ( strcmp(op, "GTO") != 0 ) &&
                    ( strcmp(op, "GSB") != 0 )
                    )
                    printf("\n");
            }
        }
        else silent = true;
        not_done = parse(op, prgm_mode);
        mv_step(1);
    }
    while( not_done && success && !trace );
    silent = false;
}

void read_prgm(void) // read user program
{
	int start_pos;
	int count = 0;
	
	chdir("user");
	
	if ( strlen(usrprogram) == 0 ) usrprogram = default_program;
	else
	{
		for ( int i = 0; i < maxstep; i++ ) prg[i] = "";
		system("ls *.txt");
		printf("load: ");
		usrprogram = GetString();
	}

    FILE *file_in;
    file_in = fopen(usrprogram, "r");

    if (file_in == NULL)
    {
    	printf("Error reading the file\n");
    	program_loaded = false;
        return;
    }
    else program_loaded = true;
    
    char line[150], buffer[150];
    
	while ( fgets(buffer,150,file_in) )
	{
		if ( count >= MAX_PRG_SIZE )
		{
			printf("Overflow error: Maximum program size reached.\n");
			break;
		}
		strcpy(line, buffer);
		start_pos = 0;
		if ( line[0] != '#' ) // not a comment
		{
			for (int i = 0, length = strlen(line); i <= length; i++)
			{
				if ( ( line[i] == ' ' ) || (i + 1 == length) )
				{
					if ( i - start_pos > 0 )
					{
						char* word = malloc((i - start_pos) * sizeof(char));
						for ( int j = 0; j < (i - start_pos); j++ )
						{
							word[j] = line[j + start_pos];
							if ( islower(word[j]) ) word[j] = toupper(word[j]);
						}
						prg[count++] = word;
						start_pos = i + 1;
					}
				}
			}
		}
	}
	maxstep = --count;
	fclose(file_in);
}

void stop(void) // stop user program and output x
{
	bst(1);
   	if( strcmp(prg[step], "INT") == 0 ) printf("%i\n", (int) x);
   	else printfloat(x);
   	fst(1);
}

void skip_1_step(void)
{
	// if a conditional is false, skip 1 step. The following count as 1/2 step: LBL, GTO, GSB, STO, RCL.
	mv_step(1);
	if ( ( strcmp(prg[step], "LBL") == 0 ) || ( strcmp(prg[step], "GTO") == 0 ) || ( strcmp(prg[step], "GSB") == 0 ) || 
	( strcmp(prg[step], "STO") == 0 ) || ( strcmp(prg[step], "RCL") == 0 ) )
		mv_step(1);
}

void isz(void)
{
    i_reg++;
    if ( i_reg == 0 ) skip_1_step();
    if ( i_reg > 10 ) 
    {
         printf("Logic error. Aborting\n");
         skip_1_step();
    }
}

void dsz(void)
{
    i_reg--;
    if ( i_reg == 0 ) skip_1_step();
    if ( i_reg < -10 ) 
    {
         printf("Logic error. Aborting\n");
         skip_1_step();
    }
}

void x_lt_0(void)
{
    if ( ! ( x < 0 ) ) skip_1_step();
}

void x_gt_0(void)
{
    if ( ! ( x > 0 ) ) skip_1_step();
}

void x_gt_1(void)
{
	if ( ! ( x > 1 ) ) skip_1_step();
}

void x_ne_0(void)
{
    if ( x == 0 ) skip_1_step();
}

void x_eq_y(void)
{
	if ( ! ( x == y ) ) skip_1_step();
}

void x_gt_y(void)
{
	if ( ! ( x > y ) ) skip_1_step();
}

void lbl(int mode) // 0: interactive, 1: program, 2: trace
{
    mv_step(1);
    if ( mode == 2 ) printf("%s\n", prg[step]);
}

void mv_step(int moves)
{
    // int length = arr_size(prg); // includes "Q" (which is like EOF)
    while ( moves > 0 )
    {
        if ( (step + 1) <= maxstep ) // (was < length)
            step++;
        else
            step = 0;
        moves--;
    }
}

int find_label(int mode, char* search)
{
    int start = step;
    int prev_step;
    do
    {
        prev_step = step;
        mv_step(1);
        if ( step == start )
        {
            printf("GSB error: %s not found\n", search);
        }
        else
        {
            if (( strcmp(prg[prev_step], "LBL" ) == 0 ) && ( strcmp(search, prg[step]) == 0 ))
            {
                if ( mode == 2 ) printf("%s %s\n", prg[prev_step], search);
                if ( mode == 0 ) step = prev_step; // Interactive mode: back up to lbl
                return 1;
            }
        }
    }
    while ( step != start );
    return 0;
}

int gsb(int mode)
{
	char* search;
	if ( ! mode ) // interactive
	{
		search = getBuffer('#');
		stringToUpper(search);
		find_label(mode, search);
		start(NO_TRACE);
	}
    else
    {
    	mv_step(1);
    	search = prg[step];

	    if ( mode == 2 ) printf("%s\n", search);
    	if ( step + 1 <= maxstep ) return_push(step);
    	else return_push(0);
    }
    return (find_label(mode, search));
}

int gto(int mode)
{
    char* search;
    if ( mode ) // program or trace mode
    {
        mv_step(1);
        search = prg[step];
        if ( mode == 2 ) printf("%s\n", search);
    }
    else // interactive mode
    {
        search = getBuffer('#');
        if ( search[0] == '.' ) // go to a step number by using form: .00n
        {
        	float gtostep = atof(search);
        	gtostep = gtostep * 1000;
        	// printf("atof of your entry is: %f\n", gtostep);
        	if ((gtostep >= 0) && (gtostep <= maxstep))
        	{
        		step = gtostep;
        		return 1;
        	}
        	else
        	{
        		printf("Invalid step\n");
        		return 0;
        	}      
        }
    }
    stringToUpper(search);
    return (find_label(mode, search));
}

int find(char* search)
{
    int start = step;
    int prev_step;
    if ( step - 1 >= 0 ) prev_step = step - 1;
    else prev_step = maxstep;

	stringToUpper(search);
    do
    {     
        if (( strcmp(prg[prev_step], "LBL" ) == 0 ) && ( strcmp(search, prg[step]) == 0 ))
        	return 1;
        prev_step = step;
        mv_step(1);
    }
    while ( step != start );
    return 0;
}

int arr_size(string arr[])
{
    int n = 0;
    while ( 1 )
    {
        if ( strcmp(arr[n++], "Q") == 0 )
            break;
    }
    return n;
}

void where()
{
	printf("Step %i: %s", step, prg[step]);
	if ( ( strcmp(prg[step], "LBL") == 0 ) || ( strcmp(prg[step], "GTO") == 0 ) || ( strcmp(prg[step], "GSB") == 0 ) || 
	( strcmp(prg[step], "STO") == 0 ) || ( strcmp(prg[step], "RCL") == 0 ) )
	{
        printf(" %s", prg[++step]);
    }
    printf("\n");
}

void bst(int moves)
{
	do
	{
		step--;
		if (step < 0 ) step = maxstep;
	}
	while (--moves);
	if (!silent) printf("Step %i: %s\n", step, prg[step]);
}

void fst(int moves)
{
	do
	{
		step++;
		if (step > maxstep ) step = 0;
	}
	while (--moves);
	if (!silent) where();
}

void shell(void)
{
	char* commands[] = { "cat", "ls", "gedit", "grep", "head", "less", "man", "tail", "vi", "exit", "Q" };
	char* entry;
	int length, cmd_size;

	cmd_size = arr_size(commands) - 1;
	if ( strlen(usrprogram) > 0 )
		printf("Current user program: %s\n", usrprogram);
	printf("Valid commands: ");
	for ( int i = 0; i < cmd_size; i++) printf("%s ", commands[i]);
	printf("\n");
	bool valid;
	int position;
	do
	{
		valid = false;
		printf("$ ");
		entry = GetString();
		length = strlen(entry);
		if ( length <= 1 ) break; // exit shell
		position = 0;
		for ( int i = 0; i < length; i++ )
		{
			entry[i] = tolower(entry[i]);
			if (( entry[i] == ' ' ) && ( position == 0 ) )
			{
				position = i;
			}
		}
		if ( ! position ) position = length;

		char* w1 = malloc(position * sizeof(char));
		for ( int i = 0; i < position; i++ ) w1[i] = entry[i];
		w1[position] = '\0';

		for ( int i = 0; i < cmd_size; i++)
		{
			if ( strcmp(w1, commands[i]) == 0 ) valid = true;
		}
		free(w1);
    	if ( strcmp(entry, "exit") == 0 ) break; // exit shell
		else
		{
			if ( valid ) system(entry);
			else printf("Invalid command\n");
		}
				
	} while ( 1 );
}

int parse(string entry, int mode) // 0: interactive, 1: program, 2: trace
{
    int success = 1; // for gto()
    is_key = false;
    int length = arr_size(keys);
    
    stringToUpper(entry);
    
    if ( strcmp(entry, "STACK") == 0 ) printstack();
    else if ( strcmp(entry, "CLEAR") == 0 ) clearstack();
    else if ( strcmp(entry, "+") == 0 ) add();
    else if ( strcmp(entry, "*") == 0 ) mult();
    else if ( strcmp(entry, "-") == 0 ) subtract();
    else if ( strcmp(entry, "/") == 0 ) divide();
    else if ( strcmp(entry, "STO") == 0 ) sto(mode);
    else if ( strcmp(entry, "RCL") == 0 ) rcl(mode, false);
    else if ( strcmp(entry, "POP") == 0 ) rcl(mode, true);
    else if ( strcmp(entry, "KEYS") == 0 ) prtkeys(keys, length);
    else if ( strcmp(entry, "Y^X") == 0 ) y_to_x();
    else if ( strcmp(entry, "INT") == 0 ) truncateX();
    else if ( strcmp(entry, "ABS") == 0 ) abs_value();
    else if ( strcmp(entry, "%") == 0 ) modulus();
    else if ( strcmp(entry, "LBL") == 0 ) lbl(mode); 
    else if ( strcmp(entry, "ISZ") == 0 ) isz(); // add mode param
    else if ( strcmp(entry, "DSZ") == 0 ) dsz(); // add mode param
    else if ( strcmp(entry, "X<0") == 0 ) x_lt_0();
    else if ( strcmp(entry, "X>0") == 0 ) x_gt_0();
    else if ( strcmp(entry, "X>1") == 0 ) x_gt_1();
    else if ( strcmp(entry, "X=Y") == 0 ) x_eq_y();
    else if ( strcmp(entry, "X>Y") == 0 ) x_gt_y();
    else if ( strcmp(entry, "X!=0") == 0 ) x_ne_0();
    else if ( strcmp(entry, "GTO") == 0 ) success = gto(mode);
    else if ( strcmp(entry, "GSB") == 0 ) success = gsb(mode);
    else if ( strcmp(entry, "RTN") == 0 ) { if(! rtn()) { stop(); return 0; } }
    else if ( strcmp(entry, "WHERE") == 0 ) where();
    else if ( strcmp(entry, "SST") == 0 ) start(TRACE); // do 1 step of program
    else if ( strcmp(entry, "S") == 0 ) start(TRACE); // shortcut for SST
    else if ( strcmp(entry, "BST") == 0 ) bst(1);
    else if ( strcmp(entry, "B") == 0 ) bst(1);
    else if ( strcmp(entry, "FST") == 0 ) fst(1);
    else if ( strcmp(entry, "F") == 0 ) fst(1);
    else if ( strcmp(entry, "FLOAT") == 0 ) fixed_decimal();
    else if ( strcmp(entry, "LABELS") == 0 ) list_of_labels();
    else if ( strcmp(entry, "SHELL") == 0 ) shell();
    else if ( strcmp(entry, "LOAD") == 0 ) read_prgm();
    else if ( strcmp(entry, "BEGIN") == 0 ) step = 0;
    else if ( strcmp(entry, "HELP") == 0 ) help();
    else if ( strcmp(entry, "OFF") == 0 ) success = 1; // had to do something
    else if ( strcmp(entry, "Q") == 0 ) success = 1; // but it's really just a pass-thru
    else if ( strcmp(entry, "R/S") == 0 )
    {
        if ( ! mode ) // if mode is interactive, start user program
            start(NO_TRACE);
        else // from within the program (mode 1 or greater), R/S means stop
        {
          	bst(1);
           	if( strcmp(prg[step], "INT") == 0 ) printf("%i\n", (int) x);
           	else printfloat(x);
           	fst(1);
            return 0;
        }
    }
    
    else // push(num) unless entry isn't numeric and not a member of keys[]
    {
        num = atof(entry); // non-numeric entry results in value of 0 (don't push)
        if (num == 0)
            if ( entry[0] != '0' ) // 00 is still 0, 0word counts as 0 too
            {
            	if (find(entry)) // run program at label
            	{
            		mv_step(1);
            		start(NO_TRACE);
            		return 0; // **** was return 1
            	}
            	else
            	{
            		if ( strlen(entry) > 0 )
	                	printf("%s is invalid\n", entry);
    	            return 0;
                }
            }
        push(num);
    }
    return 1;
}

int main(int argc, string argv[])
{
	int arg = 1;
	read_prgm();
	if ( arg < argc ) autofloat = true;
    while ( arg < argc ) // do command line instructions
    {
        entry = argv[arg];
        stringToUpper(entry);
        if ( ( strcmp(entry, "FLOAT") == 0 ) 
            || ( strcmp(entry, "STO") == 0 )
            || ( strcmp(entry, "RCL") == 0 ) 
            || ( strcmp(entry, "POP") == 0 )
            || ( strcmp(entry, "GTO") == 0 )
            || ( strcmp(entry, "GSB") == 0 ) )
        	{
        		buffer_position = -1; // sentinel value
        		if ( arg + 1 < argc ) buffer = argv[++arg];
        		else buffer = "";
        	}
       	parse(entry, 0);
       	arg++;
    }
    
    if (argc > 1) exit(0);

    do // interactive mode
    {
        entry = getBuffer('?');
        parse(entry, 0);
    }
    while ( (strcmp(entry, "OFF") != 0 ) && (strcmp(entry, "Q") != 0 ) );
}

void prtkeys(string keys[], int key_count)
{
	int count = 0;
    for ( int i = 0; i < key_count; i++ )
    {
        printf("%s \t", keys[i]);
        if ( count++ == 7 )
        {
        	printf("\n");
        	count = 0;
        }
    }
    if ( count ) printf("\n");
}

void sto(int mode)
{
    string s;
    if ( ! mode ) // Interactive
    {
        s = getBuffer('#');
        if ( islower(s[0]) ) s[0] = toupper(s[0]);
    }
    else // Program or Trace mode: print STO REGISTER
    {
        if ( mode == 2 )
        	printf("%s ", prg[step]);
        mv_step(1);
        if ( mode == 2 )
        	printf("%s\n", prg[step]);        
        s = prg[step];
    }
    
    if ( strcmp(s, "N[i]") == 0 ) // STO N[I]
    {
    	   if ( (i_reg >= 0) && (i_reg < MAX_N) )
    	        n_reg[(int) i_reg] = x;
    	   else printf("Array index must be 0-%i\n", MAX_N-1);
    }
    else if ( strcmp(s, "I") == 0 ) // STO I
    {
        i_reg = x;
    }
    else // STO 0-9
    {
        int m = atoi(s);
        while( ( (m + 1) > REGISTERS) || (m < 0) )
        {
            if ( mode ) // Program or Trace mode
            {
                printf("Error: Invalid register\n");
                return;
            }
            printf("0-9: ");
            m = GetInt();
        }
        sto_push(m, x);
        // mem[m] = x;
    }
}

void rcl(int mode, bool pop)
{
    string s;
    if ( ! mode ) // Interactive mode
    {
        s = getBuffer('#');
        if ( islower(s[0]) ) s[0] = toupper(s[0]);
    }
    else // Program or Trace mode: print RCL REGISTER
    {
        if ( mode == 2 ) printf("%s ", prg[step]);
        mv_step(1);
        if ( mode == 2 ) printf("%s\n", prg[step]);
        s = prg[step];
    }
    if ( strcmp(s, "N[i]") == 0 ) // RCL N[I]
    {
    	   if ( (i_reg >= 0) && (i_reg < MAX_N) )
             x = n_reg[(int) i_reg];
    	   else printf("Array index must be 0-%i\n", MAX_N-1);
    }
    else if ( strcmp(s, "I") == 0 ) // RCL I
    {
        x = i_reg;   
    }
    else // RCL 0-9
    {
   	    int m = atoi(s);
   	    while( ( (m + 1) > REGISTERS) || (m < 0) )
   	    {
   	        if ( mode ) // Program or Trace mode
   	        {
   	            printf("Error: Invalid register\n");
   	            return;
   	        }
   	        printf("0-9: ");
   	        m = GetInt();
   	    }
   	    if ( pop ) x = rcl_pop(m);
   	    else x = mem[m][register_index[m]];
    }
    push(x);
    if (!silent) printfloat(x);
}

void fixed_decimal(void)
{
	char* s = getBuffer('#');
	int f = atoi(s);
	autofloat = false;	
	
	if ( f <= 0 ) autofloat = true;
	if ( f > 9 ) fix_val = 12;
	else fix_val = f;
	if ( stack_index >= 0 )
		printfloat(x);
}

void printfloat(long double r)
{
	if ( autofloat )
	{
		long double r1;
		r1 = r * 10000000;
		r1 = (int) r1;
		r1 = r1 / 10000000;

		fix_val = 0;
		int count = 0;
		long double temp = (int) r1;
		if ( r1 - temp > .000001 )
		{
			fix_val = 9;
			temp = r1;
			long double t1;
			do
			{
				count++;
				temp = temp * 10;
				t1 = (int) temp;
				if ( temp - t1 < .000001 )
				{
					fix_val = count;
					break;
				}
			} while ( count < 10 );
			if ( fix_val > 7 ) fix_val = 9;
		}
	}


	switch ( fix_val ) {
		case 0: printf("%.0Lf\n", r); break;
		case 1: printf("%.1Lf\n", r); break;
		case 2: printf("%.2Lf\n", r); break;
		case 3: printf("%.3Lf\n", r); break;
		case 4: printf("%.4Lf\n", r); break;
		case 5: printf("%.5Lf\n", r); break;
		case 6: printf("%.6Lf\n", r); break;
		case 7: printf("%.7Lf\n", r); break;
		case 8: printf("%.8Lf\n", r); break;
		case 9: printf("%.9Lf\n", r); break;
		default: printf("%.12Lf\n", r); }
}

void truncateX(void)
{
    int r = (int) pop();
    push(r);
    if (!silent) printf("%i\n", r);
}

void abs_value(void)
{
	long double r = pop();
	if (r < 0) r = -1 * r;
	push(r);
    if (!silent) printfloat(r);
}

void modulus(void)
{
    int x = (int) pop();
    int y = (int) pop();
    int r = y % x;
    push(r);
    if (!silent) printf("%i\n", r);
}

void y_to_x(void)
{
    long double x = pop();   
    long double y = pop();
    long double r = pow(y, x);
    push(r);
    if (!silent) printfloat(r);
}

void add(void)
{
    long double x = pop();   
    long double y = pop();
    long double r = x + y;
    push(r);
    if (!silent) printfloat(r);
}

void mult(void)
{
    long double x = pop();   
    long double y = pop();
    long double r = x * y;
    push(r);
    if (!silent) printfloat(r);
}

void subtract(void)
{
    long double x = pop();   
    long double y = pop();
    long double r = y - x;
    push(r);
    if (!silent) printfloat(r);
}

void divide(void)
{
    long double x = pop();   
    long double y = pop();
    long double r = y / x;
    push(r);
    if (!silent) printfloat(r);
}

long double pop(void)
{
    long double r;
    if ( stack_index >= 0 )
    {
    	r = stack[stack_index];
    	stack_index--;
    }
    else
    {
        // stack is empty
        r = 0;
    }
    return r;
}

void printstack(void)
{
    if ( stack_index < 0 )
    {
        printf("Empty\n");
        return;
    }
    for ( int i = 0; i <= stack_index; i++)
    {
    	if ( i == stack_index-1 ) printf("y: ");
    	else if ( i == stack_index ) printf("x: ");
    	else printf("%i: ", i);
        printfloat(stack[i]);
    }
}

void clearstack(void)
{
	stack_index = -1;
	x = 0;
}

void push(long double num)
{
    x = num;
    // get y
    if ( stack_index >= 0 )
    	y = stack[stack_index];
    else
    	y = 0;
    	
    // add 1 to stack_index
    if ( stack_index < 0 )
        stack_index = 0;
    else
        stack_index++;
        
    if ( ( stack_index ) <= ( MAX_INDEX ) )
    {
        stack[stack_index] = num;
    }
    else
    {
        int i;

        for ( i = 0; i < ( MAX_INDEX ); i++ ) // ( i = 0; i < 3; i++ ) for stack of 4
        {
            stack[i] = stack[i+1];
        }
        stack_index = ( MAX_INDEX );
        stack[stack_index] = num;
    }
}

// mem_num: 0-9 (storage registers) (each is a stack)
// num: number to be stored
void sto_push(int mem_num, long double num)
{
	// int mem_index = register_index[mem_num];

	if ( register_index[mem_num] < 0 ) register_index[mem_num] = 0;
	else register_index[mem_num]++;
	
	if ( register_index[mem_num] <= MAX_INDEX ) mem[mem_num][register_index[mem_num]] = num;
	else
	{
		int i;
		
		for ( i = 0; i < ( MAX_INDEX ); i++ )
        {
        	mem[mem_num][register_index[mem_num]] = mem[mem_num][register_index[mem_num + 1]];
            // stack[i] = stack[i+1];
        }
        register_index[mem_num] = MAX_INDEX; // stack_index = ( MAX_INDEX );
        mem[mem_num][register_index[mem_num]] = num; // stack[stack_index] = num;		
	}
}

long double rcl_pop(int mem_num)
{
    long double r;
    if ( register_index[mem_num] >= 0 )
    {
    	r = mem[mem_num][register_index[mem_num]];
    	register_index[mem_num]--;
    	// r = stack[stack_index];
    	// stack_index--;
    }
    else
    {
        // stack is empty
        r = 0;
    }
    return r;
}

int return_push(int rtn_step)
{
	if ( rtn_index < -1 )
		rtn_index = -1;
	else
		rtn_index++;
	if ( ( rtn_index ) <= ( RETURN_MAX_INDEX ) )
	{
		rtn_stack[rtn_index] = rtn_step;
		return 1;
	}
	else
	{
		printf("Error: exceded max returns\n");
		exit(0);
	}
}

// return_pop():
//
// If the return_index is < 0, the stack is empty.
// Otherwise, return the value stored at rtn_stack[rtn_index].
//
int return_pop()
{
	int rstep = -1;
	if ( rtn_index >= 0 )
	{
		rstep = rtn_stack[rtn_index];
		rtn_index--;
	}
	return rstep;
}

// rtn():
//
// Set value of <step> by calling return_pop().
// If a negative value is received, the stack was empty.
// Return 1 for success or 0 for empty return stack.
//
int rtn()
{
	int rtn_step = return_pop();
	if ( rtn_step >= 0 )
	{
		step = rtn_step;
		return 1;
	}
	else return 0;
}
