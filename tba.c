/* The 3TB4 Assembler
    Written by: Joshua Barkovic
    April 1, 2014
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char * BR       = "br";
const char * BRZ      = "brz";
const char * ADDI     = "addi";
const char * SUBI     = "subi";
const char * SR0      = "sr0";
const char * SRH0     = "srh0";
const char * CLR      = "clr";
const char * MOV      = "mov";
const char * MOVA     = "mova";
const char * MOVR     = "movr";
const char * MOVRHS   = "movrhs";
const char * PAUSE    = "pause";

const char * BR_OP       = "100";
const char * BRZ_OP      = "101";
const char * ADDI_OP     = "000";
const char * SUBI_OP     = "001";
const char * SR0_OP      = "0100";
const char * SRH0_OP     = "0101";
const char * CLR_OP      = "011000";
const char * MOV_OP      = "0111";
const char * MOVA_OP     = "110000";
const char * MOVR_OP     = "110001";
const char * MOVRHS_OP   = "110010";
const char * PAUSE_OP    = "11111111";

/* CHANGE THESE IF THE LAB CHANGES*/
const char * MEM_DEPTH	= "256"; // ie number of words
const char * MEM_WIDTH	= "8"; // ie size of each word

char * ERR_PREFIX = "tba: ERROR: ";
char * ERR_MSG_ARG = "Not enough arguments\n";
char * HELP_MSG = "Help: \n\tNeed: \ttba <source> <dest> for output to file <dest>, or \n\t\ttba <source> for output to screen\n";
char * HELP_PROMPT = "Try: tba --help, for more.\n";
char file_suffix[4] = ".mif";

static int assemble(char * asmbly,char * machine);
static int getimmed(char* immed,char* arg,int n,int signd);
int main(int argc,char ** argv) {
    char * w_filename;
    FILE * asmb = NULL;
    FILE * mach = NULL;
    if (argc < 2) {
        fprintf(stderr,"%s %s %s",ERR_PREFIX,ERR_MSG_ARG,HELP_PROMPT);
        return -1;
    } else {
	int i;
	for (i=0;i<argc;i++) {
            if (strcmp("--help",argv[i]) | strcmp("-h",argv[i]) == 0) {
	        fprintf(stdout,"tba - The 3TB4 Assembler \n Joshua Barkovic \n April 1, 2014 \n%s",HELP_MSG);
		return 0;
	    }
	}
    }
    asmb = fopen(argv[1],"r");
    if (asmb == NULL) {
        fprintf(stderr,"%s Could not open input file \"%s\"\n%s \n\t%s",ERR_PREFIX,argv[1],ERR_MSG_ARG,HELP_PROMPT);
        return -1;
    }
	if (argc > 2 && strcmp(argv[2],"-mif")) {
	    mach = fopen(argv[2],"w");
	    if (mach == NULL) {
	        fprintf(stderr,"%s Could not open output file \"%s\"\n\t %s",ERR_PREFIX,argv[2],HELP_PROMPT);
	        return -1;
	    }
	}
	short int print_mif = 0;
	if (argc == 4 && strcmp(argv[3],"-mif") == 0) print_mif = 1;
	if (argc == 3 && strcmp(argv[2],"-mif") == 0) print_mif = 1;
    char line[128];
	char temp[128];
	int line_num=0;
	char machine_code[9]; // 0 terminated
	int error = 0;
	if (print_mif) {
		if (mach != NULL) fprintf(mach,"%s %s %s %s %s","DEPTH = ",MEM_DEPTH,"; % Memory depth and width are required % \n % DEPTH is the number of addresses % \nWIDTH = ",MEM_WIDTH,"; % WIDTH is the number of bits of data per word % \n% DEPTH and WIDTH should be entered as decimal numbers % \n \nADDRESS_RADIX = DEC; % Address and value radixes are required % \nDATA_RADIX = BIN; % Enter BIN, DEC, HEX, OCT, or UNS; unless % \n % otherwise specified, radixes = HEX % \n \n-- Specify values for addresses, which can be single address or range \nCONTENT \nBEGIN \n",MEM_DEPTH,MEM_WIDTH);
		else printf("%s","DEPTH = 256; % Memory depth and width are required % \n % DEPTH is the number of addresses % \nWIDTH = 8; % WIDTH is the number of bits of data per word % \n% DEPTH and WIDTH should be entered as decimal numbers % \n \nADDRESS_RADIX = DEC; % Address and value radixes are required % \nDATA_RADIX = BIN; % Enter BIN, DEC, HEX, OCT, or UNS; unless % \n % otherwise specified, radixes = HEX % \n \n-- Specify values for addresses, which can be single address or range \nCONTENT \nBEGIN \n");
	}
    while (fgets(line,sizeof line,asmb) != NULL) {
	memcpy(temp,line,128);
        machine_code[0] = '\0';
	int line_err = 0;
        line_err = assemble(line,machine_code);
	if (line_err != 2) error |= line_err;
	if (line_err != 2 && line_err != 0) {
		fprintf(stderr,"  -> at line (%i): %s",line_num,temp);
	} else if (line_err == 2) {line_num--; // do nothing
	} else {
		if (mach != NULL) {if (!print_mif) fprintf(mach,"%s\n",machine_code); else fprintf(mach,"%i : %s;\n",line_num,machine_code);}
		else if (print_mif) printf("%i : %s;\n",line_num,machine_code);
		else printf("%s\n",machine_code);
	}
	line_num++;
	}
	if (print_mif) {
		int mem_max = atoi(MEM_DEPTH);
		mem_max--;
		if (mach != NULL) fprintf(mach,"[%i..%i] : 0; \n%s\n",line_num,mem_max,"END;");
		else printf("[%i..%i] : 0; \n%s\n",line_num,mem_max,"END;");
	}
	if (error) fprintf(stderr,"%s[1] Could not assemble \"%s\" see previous errors.\n",ERR_PREFIX,argv[1]);
    if (mach != NULL) fclose(mach);fclose(asmb);
    return 0;
}
static int assemble(char * asmbly,char * machine) {
    int i; char c;
    char * op = NULL; char * arg0 = NULL; char * arg1 = NULL; char * end = NULL;
	// Trim out the garbage (documentation)
	/* The following separates each line of assembly into:
		the operation, arg0 and arg1
	*/
    if (asmbly != NULL) {
        op = asmbly;
        end = (char*) strpbrk(asmbly, "#");
        if (end) {
            while (*(end-1)==' '||*(end-1)=='\t') {end--;if(end == asmbly+1) break;}
            *end = '\0';
        }
		arg0 = (char*) strpbrk(op, " \t,");
        char * delim = "-rR0123456789"; // matches any possibe immediate (in decimal or straight-up binary) and register
        if (arg0 != NULL) {
            *arg0 = '\0'; arg0++;
            if( (char*) strpbrk(arg0, delim)) arg0 = (char*) strpbrk(arg0, delim);
            arg1 = (char*) strpbrk(arg0, " \t,");
            if (arg1 != NULL) {
				*arg1 = '\0';
				arg1++;
                arg1 = (char*) strpbrk(arg1, delim);
                if (arg1 != NULL) *(arg1-1) = '\0';
            }
        }
    }
    if (arg0 == NULL) arg0 = ""; // prevent segfaults
    if (arg1 == NULL) arg1 = "";
    machine[0] = '\0';
    char * r_a0=NULL;char * r_a1=NULL;
    // get registers from arg0 (note: immediates and registers are parsed for each instruction, even if not needed
    if (!strncmp(arg0,"R0",2) || !strncmp(arg0,"r0",2)) r_a0 = "00";
    if (!strncmp(arg1,"R0",2) || !strncmp(arg1,"r0",2)) r_a1 = "00";
    if (!strncmp(arg0,"R1",2) || !strncmp(arg0,"r1",2)) r_a0 = "01";
    if (!strncmp(arg1,"R1",2) || !strncmp(arg1,"r1",2)) r_a1 = "01";
    if (!strncmp(arg0,"R2",2) || !strncmp(arg0,"r2",2)) r_a0 = "10";
    if (!strncmp(arg1,"R2",2) || !strncmp(arg1,"r2",2)) r_a1 = "10";
    if (!strncmp(arg0,"R3",2) || !strncmp(arg0,"r3",2)) r_a0 = "11";
    if (!strncmp(arg1,"R3",2) || !strncmp(arg1,"r3",2)) r_a1 = "11";
    if (r_a0 == NULL) r_a0 = "";
    if (r_a1 == NULL) r_a1 = "";
	// convert op to lower case
	for(i = 0; op[i]; i++){
		op[i] = tolower(op[i]);
	}
    char * immed = (char*)malloc(6*sizeof(char));
    short int error = 0;
    /*BR*/		if (strcmp(op,BR) == 0) {
		strcat(machine,BR_OP);
		error |= getimmed(immed,arg0,5,1);
		strcat(machine,immed);
    }
    /*BRZ*/		else if (strcmp(op,BRZ) == 0) {
        strcat(machine,BRZ_OP);
        error |= getimmed(immed,arg0,5,1);
       	strcat(machine,immed);
    }
    /*ADDI*/	else if (strcmp(op,ADDI) == 0) {
        strcat(machine,ADDI_OP);
        error |= getimmed(immed,arg1,3,0);
        strcat(machine,immed);
        strcat(machine,r_a0);
	if (strlen(r_a0)==0) {
		error = 1;
		fprintf(stderr,"%s invalid register name: %s\n",ERR_PREFIX,arg0);
	}
    }
    /*SUBI*/	else if (strcmp(op,SUBI) == 0) {
        strcat(machine,SUBI_OP);
        error |= getimmed(immed,arg1,3,0);
        strcat(machine,immed);
        strcat(machine,r_a0);
        if (strlen(r_a0)==0) {
                error = 1;
                fprintf(stderr,"%s invalid register name: %s\n",ERR_PREFIX,arg0);
        }
    }
    /*SR0*/		else if (strcmp(op,SR0) == 0) {
        strcat(machine,SR0_OP);
        error |= getimmed(immed,arg0,4,0);
       	strcat(machine,immed);
    }
    /*SRH0*/	else if (strcmp(op,SRH0) == 0) {
        strcat(machine,SRH0_OP);
        error |= getimmed(immed,arg0,4,0);
       	strcat(machine,immed);
    }
    /*CLR*/		else if (strcmp(op,CLR) == 0) {
        strcat(machine,CLR_OP);
        strcat(machine,r_a0);
        if (strlen(r_a0)==0) {
                error = 1;
                fprintf(stderr,"%s invalid register name: %s\n",ERR_PREFIX,arg0);
        }
    }
    /*MOV*/		else if (strcmp(op,MOV) == 0) {
        strcat(machine,MOV_OP);
        strcat(machine,r_a0);
        strcat(machine,r_a1);
        if (strlen(r_a0)==0) {
                error = 1;
                fprintf(stderr,"%s invalid register name: %s\n",ERR_PREFIX,arg0);
        }
        if (strlen(r_a1)==0) {
                error = 1;
                fprintf(stderr,"%s invalid register name: %s\n",ERR_PREFIX,arg1);
        }
    }
    /*MOVA*/	else if (strcmp(op,MOVA) == 0) {
        strcat(machine,MOVA_OP);
     	strcat(machine,r_a0);
        if (strlen(r_a0)==0) {
                error = 1;
                fprintf(stderr,"%s invalid register name: %s\n",ERR_PREFIX,arg0);
        }
    }
    /*MOVR*/	else if (strcmp(op,MOVR) == 0) {
        strcat(machine,MOVR_OP);
        strcat(machine,r_a0);
        if (strlen(r_a0)==0) {
                error = 1;
                fprintf(stderr,"%s invalid register name: %s\n",ERR_PREFIX,arg0);
        }
    }
    /*MOVRHS*/	else if (strcmp(op,MOVRHS) == 0) {
        strcat(machine,MOVRHS_OP);
        strcat(machine,r_a0);
        if (strlen(r_a0)==0) {
                error = 1;
                fprintf(stderr,"%s invalid register name: %s\n",ERR_PREFIX,arg0);
        }
    }
    /*PAUSE*/	else if (strcmp(op,PAUSE) == 0) {
        strcat(machine,PAUSE_OP);
    }
	/*NOTHING*/ else if (strlen(op) == 0) {
		error = 2;
	}
	else {
		int all_spaces = 1;
		int i;
		for (i=0;i<strlen(op);i++) {
			char c = op[i];
			if (c!=' ' && c!='\t' && c!='\n') { all_spaces = 0;break;}
			error = 2;
		}
		if (!all_spaces) {fprintf(stderr,"%s Unrecognized symbol: \"%s\"\n",ERR_PREFIX,op); error = 1;}
	}
	return error;
}
static int getimmed(char* immed,char* arg,int n,int signd) {
	immed[n] = '\0';
	int err_ret = 0;
	int mask = 0;
	mask = ~mask;
	mask = mask << n;
    long int l_num = strtol(arg,NULL,0);
	int num = (int) l_num;

	int res = mask;
	if (!signd) {
		res &= num;
		if (res) {
			err_ret = -1;
			fprintf(stderr,"%s %i-bit immediate (unsigned out of range): %li, max range : [0 - %i]\n",ERR_PREFIX,n,l_num,-1*mask-1);
		}
	} else {
		res >> 1;		// max is 2^(n-1) magnitude
		int p_num = num;
		if (num < 0) p_num = -1*num;
		res &= p_num;
		if (res) {
			err_ret = -1;
			fprintf(stderr,"%s %i-bit immediate (signed out of range): %li, max range : [%i - %i]\n",ERR_PREFIX,n,l_num,mask>>1,-1*(mask>>1)-1);
		}
	}
	int i;
	int bit = 1;
	immed[n] = '\0';
	while (--n>=0) {
		immed[n] = ((char) ((bit&num)&&1)) + '0';
		bit<<=1;
	}
	return err_ret;
}
