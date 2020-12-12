/*
Oindrila Bhadra
1801CS34
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Struct for symbol table for storing labels and addresses*/
struct symAddr
{
    char* symbol;
    int addr;
};

/*Struct for storing label, mnemonic and operand of each line of code*/
struct separate
{
    char *label, *mnemonic, *opLabel;
    int operand;
};

struct instruction /*Specify name,opcode and operand type of all mnemonics*/
{
    char* mne;
    int opc;
    char oprType; /*'O' for offset, 'V' for value, 'N' for none*/
};

struct symAddr* sym_tab; /*Storing labels and their addresses*/
struct separate* separateOp; /*Storing labels, mnemonics and operands of a line separately*/
struct instruction* operations;

int e = 0; /*e will become 1 if any error is there*/
int* foundLabel; /*For keeping a track whether all labels have been used or not*/

FILE* logFile;

/*Remove comments from each line*/
char* removeComment(char* line)
{
    char* ans = malloc(1000);
    int i, c = 0, t = 0;
    for (i = 0; line[i] != '\0'; i++)
    {
        if (t == 0 && line[i] == ' ') /*Skip any leading blank space*/
            continue;
        if (t == 0)
            t = 1;
        if (line[i] == ';' || line[i] == '\r'
            || line[i] == '\n') /*Comment encountered or line end*/
            break;
        ans[c++] = line[i];
    }

    return ans; /*Line with comments removed*/
}

/*Insert values into the operations table*/
void insert(char* mn, int op, char t)
{
    operations[op].mne = mn;
    operations[op].opc = op;
    operations[op].oprType = t;
}

/*Fill operations table*/
void fillOperations(void)
{
    operations = malloc(
        21 * sizeof(struct instruction)); /*Allocate operations table with size to hold 21 values.*/
    insert((char*)"ldc", 0, 'V');
    insert((char*)"adc", 1, 'V');
    insert((char*)"ldl", 2, 'O');
    insert((char*)"stl", 3, 'O');
    insert((char*)"ldnl", 4, 'O');
    insert((char*)"stnl", 5, 'O');
    insert((char*)"add", 6, 'N');
    insert((char*)"sub", 7, 'N');
    insert((char*)"shl", 8, 'N');
    insert((char*)"shr", 9, 'N');
    insert((char*)"adj", 10, 'V');
    insert((char*)"a2sp", 11, 'N');
    insert((char*)"sp2a", 12, 'N');
    insert((char*)"call", 13, 'O');
    insert((char*)"return", 14, 'N');
    insert((char*)"brz", 15, 'O');
    insert((char*)"brlz", 16, 'O');
    insert((char*)"br", 17, 'O');
    insert((char*)"HALT", 18, 'N');
    insert((char*)"data", 19, 'V');
    insert((char*)"SET", 20, 'V'); /*Populate the table with mnemonics, opcodes and operand types.*/
}

/*For issuing warnings for unused labels*/
void unusedLabel(int len)
{
    int i, j;
    for (i = 0; sym_tab[i].symbol != NULL; i++)
    {
        if (foundLabel[i] == 1) /*ith label has been used already*/
            continue;
        /*If label at ith position is not used yet.
          Check at which line that label was actually declared.*/

        for (j = 0; j < len;
             j++) /*Check if the ith label of the sym_lab table is present in which line of the
                     code*/
        {
            if (strcmp(sym_tab[i].symbol, separateOp[j].label) == 0)
            {
                char warning[50];
                sprintf(warning, "Line %d: warning: Unused label\n",
                    j + 1); /*Issue a warning in the log file*/
                fputs(warning, logFile);
                break;
            }
        }
    }
}

/*Check if a line is null or filled with blank spaces*/
int isEmpty(char* line)
{
    int i;
    for (i = 0; line[i] != '\0'; i++)
    {
        if (line[i] != ' ' && line[i] != '\t')
            return 0; /*Not empty line*/
    }
    return 1; /*Empty line*/
}

/*Find out if any label is present in a line*/
char* findLabel(char* line, int lno)
{
    int i, c = 0, temp = 0;
    char* ans = malloc(1000);
    for (i = 0; line[i] != '\0'; i++)
    {
        if (line[i] == ':') /*Find the position of the ':' */
        {
            temp = 1; /*temp=1 means there is a label*/
            break;
        }
        ans[c++] = line[i]; /*Store the label in ans*/
    }
    if (temp == 0)
        return (char*)""; /*No label found in the line*/

    for (i = c - 1; i >= 0; i--)
    {
        if (ans[i] == ' '
            || ans[i] == '\t') /*Remove any blank spaces from the end of the label stored in ans*/
            c--;
        else
            break;
    }
    ans[c] = '\0'; /*Terminate the label string*/

    /*Check if the label is valid or not*/
    for (i = 0; i < c; i++)
    {
        if (!((ans[i] >= 65 && ans[i] <= 90) || (ans[i] >= 97 && ans[i] <= 122)
                || (i != 0 && ans[i] >= 48 && ans[i] <= 57))) /*Only numbers and letters can be
                                                                 present in the label. First
                                                                 character must be a letter.*/
            break; /*If naming convention of label is not followed then break from the loop*/
    }
    if (i != c) /*If entire label string could not be traversed because of wrong naming convention*/
    {
        char error[50];
        sprintf(error, "Line %d: error: Invalid label\n", lno + 1); /*Issue an error in log file*/
        fputs(error, logFile);
        if (e == 0)
            printf("Line %d: error: Invalid label\n", lno + 1);
        e = 1;
    }
    return ans; /*Return the label*/
}

/*Find the mnemonic instruction from the line*/
char* findMnemonic(char* line)
{
    char* ans = malloc(1000);
    int i, c = 0, st, temp = 0;

    char* found = strstr(line, ":");
    if (found == NULL) /*No label present*/
        st = 0;
    else
        st = found - line + 1; /*st is position of ':' */

    for (i = st; line[i] != '\0'; i++) /*Traverse the line from after the label*/
    {
        if (temp == 0)
        {
            if (line[i] == ' ' || line[i] == '\t') /*Discard any spaces preceding the mnemonic*/
                continue;
            temp = 1;
        }
        if (line[i] == ' ' || line[i] == '\t')
            break;
        ans[c++] = line[i]; /*Store mnemonic in ans*/
    }
    return ans; /*Return mnemonic*/
}

/*Find operand from the line*/
int findOperand(char* line)
{
    char* ans = malloc(1000);
    char* done;
    int i, c = 0, st, temp = 0, num;

    char* found = strstr(line, ":");
    if (found == NULL)
        st = 0;
    else
        st = found - line + 1; /*Store position of ':' in st*/


    /*Start traversing the line from after the label*/
    for (i = st; line[i] != '\0'; i++)
    {
        if (temp == 0)
        {
            if (line[i] == ' ' || line[i] == '\t') /*Skip any leading space before the mnemonic*/
                continue;
            temp = 1;
        }
        if (line[i] == ' ' || line[i] == '\t')
            break; /*Mnemonic has been traversed*/
    }

    st = i; /*Start traversing from after the mnemonic*/
    for (i = st; line[i] != '\0'; i++)
    {
        if (line[i] == ' ' || line[i] == '\t') /*Skip any leading space before the operand*/
            continue;
        ans[c++] = line[i]; /*Store operand in ans*/
    }

    if (isEmpty(ans)) /*No operand present*/
        return 2147483647;
    num = strtol(ans, &done, 0); /*Convert the operand stored as a string into a number*/
    if (*done) /*If conversion not possible i.e. if a label is present as operand*/
        return -2147483648;
    return num;
}

/*Find if any label is present as the operand*/
char* findOpLabel(char* line)
{
    char* ans = malloc(1000);
    int i, c = 0, st, temp = 0;

    char* found = strstr(line, ":");
    if (found == NULL)
        st = 0;
    else
        st = found - line + 1;

    /*Start from after the position of ':'.
      Skip the mnemonic part*/
    for (i = st; line[i] != '\0'; i++)
    {
        if (temp == 0)
        {
            if (line[i] == ' ' || line[i] == '\t') /*Skip any leading space before the mnemonic*/
                continue;
            temp = 1;
        }
        if (line[i] == ' ' || line[i] == '\t')
            break;
    }

    st = i;
    /*Start traversing from after the mnemonic*/
    for (i = st; line[i] != '\0'; i++)
    {
        if (line[i] == ' ' || line[i] == '\t')
            continue; /*Skip any leading space before the label present as operand*/
        ans[c++] = line[i];
    }
    return ans; /*Return the label present as operand*/
}

/*Separate the label, mnemonic and operand from each line*/
void separateOperation(char* code[], int len)
{
    int i;
    separateOp = malloc(len * sizeof(struct separate)); /*Allocate memory to the array of structs*/
    for (i = 0; i < len; i++)
    {
        if (code[i][0] == '*') /*If the line is blank or has only comments*/
        {
            separateOp[i].label = (char*)"";
            separateOp[i].mnemonic = (char*)"";
            separateOp[i].operand = -1;
            separateOp[i].opLabel = (char*)"";
        }
        else
        {
            separateOp[i].label = findLabel(code[i], i);
            separateOp[i].mnemonic = findMnemonic(code[i]);
            separateOp[i].operand = findOperand(code[i]);
            if (separateOp[i].operand == -2147483648) /*No number is present as operand. Label is
                                                         present as the operand.*/
                separateOp[i].opLabel = findOpLabel(code[i]);
            else /*Number present as operand*/
                separateOp[i].opLabel = (char*)"";
        }
    }
}


/*PASS 1*/

void fillSymTab(int len)
{
    int i, pc = 0, temp, c = 0, j;
    sym_tab = malloc(len * sizeof(struct symAddr));
    foundLabel = malloc(len * sizeof(int));

    for (i = 0; i < len; i++)
    {
        foundLabel[i] = 0;
        temp = 0;
        if (separateOp[i].label[0] != '\0') /*If a label is present in the line*/
        {
            for (j = 0; j < c; j++)
            {
                if (strcmp(separateOp[i].label, sym_tab[j].symbol)
                    == 0) /*If the label at line i has already been found in some previous line*/
                {
                    char error[50];
                    sprintf(error, "Line %d: error: Duplicate label\n", i + 1);
                    fputs(error, logFile); /*Issue a duplicate label error in log file*/
                    temp = 1;
                    if (e == 0)
                        printf("Line %d: error: Duplicate label\n", i + 1);
                    e = 1;
                    break;
                }
            }
            if (temp == 0) /*If label at line i is not a duplicate label*/
            {
                sym_tab[c].symbol = separateOp[i].label;
                if (strcmp(separateOp[i].mnemonic, "SET") == 0) /*If mnemonic is set, store the
                                                                   value of the operand in the
                                                                   address position*/
                    sym_tab[c].addr = separateOp[i].operand;
                else
                    sym_tab[c].addr = pc; /*Otherwise store the address of the label in sym_tab*/
                c++;
            }
        }
        if (separateOp[i].mnemonic[0] != '\0')
            pc++; /*Increment program counter if mnemonic present in the line*/
    }
}

/*Find opcode of mn*/
int position(char* mn)
{
    int i;
    for (i = 0; i < 21; i++)
    {
        if (strcmp(mn, operations[i].mne)
            == 0) /*If mnemonic mn matches mnemonic at position i of operations array*/
            return operations[i].opc; /*Return the position which is the opcode*/
    }
    return -1; /*mnemonic mn not found in operations array*/
}

/*Find the address of the label from the sym_tab*/
int findLabelAddr(char* lab)
{
    int i;
    for (i = 0; sym_tab[i].symbol != NULL;
         i++) /*Traverse sym_tab as long as the value in it does not become NULL*/
    {
        if (strcmp(sym_tab[i].symbol, lab)
            == 0) /*If label matches the label value at position i of sym_tab*/
        {
            foundLabel[i] = 1; /*For ensuring all labels are used*/
            return sym_tab[i].addr;
        }
    }
    return -1; /*Label lab not found*/
}

/*Extend the data to b*4 bits in hexadecimal form*/
char* extend(int data, int b)
{
    char *bin_str = malloc(8 * sizeof(data)), *ans = malloc(10);
    int i, c = b - 1, mask, v, BITS = 32;
    for (i = 0; i < BITS; i++)
    {
        mask = 1u << (BITS - 1 - i);
        bin_str[i] = (data & mask) ? '1' : '0'; /*Store binary equivalent of data in bin_str*/
    }

    /*According to our program b can only be 2,6 or 8*/
    /*Initialise the answer with zeroes*/
    if (b == 2)
        strcpy(ans, "00");
    else if (b == 6)
        strcpy(ans, "000000");
    else
        strcpy(ans, "00000000");

    for (i = 31; i >= 32 - b * 4; i -= 4)
    {
        char sub[4];
        memcpy(sub, &bin_str[i - 3], 4);
        v = (sub[0] - 48) * 8 + (sub[1] - 48) * 4 + (sub[2] - 48) * 2
            + (sub[3] - 48); /*Convert groups of 4 bits in binary to hexadecimal*/
        if (v <= 9) /*If 0-9 */
            v += 48;
        else /*If A-F */
            v += 87;
        ans[c--] = (char)v; /*Store hex equivalent of data in ans*/
    }

    return ans; /*Return hex equivalent of data extended to b*4 bits in 2's complement form*/
}

/*Check if the operation is valid or not in terms of the mnemonics and their operands involved*/
void checkValidOp(int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        char* x = separateOp[i].mnemonic;
        if (x[0] == '\0') /*If no mnemonic present*/
            continue;

        if (!strcmp(x, "data") || !strcmp(x, "adc") || !strcmp(x, "adj") || !strcmp(x, "SET")
            || !strcmp(x, "ldc") || !strcmp(x, "ldl") || !strcmp(x, "stl") || !strcmp(x, "ldnl")
            || !strcmp(x, "stnl") || !strcmp(x, "call") || !strcmp(x, "brz")
            || !strcmp(x, "brlz") || !strcmp(x, "br")) /*If mnemonic is any of these values, then
                                                          there must be an operand present*/
        {
            if (separateOp[i].operand == 2147483647
                && separateOp[i].opLabel[0] == '\0') /*But no operand is present*/
            {
                char error[50];
                sprintf(error, "Line %d: error: Missing operand\n",
                    i + 1); /*Issue an error in log file*/
                fputs(error, logFile);
                if (e == 0)
                    printf("Line %d: error: Missing operand\n", i + 1);
                e = 1;
            }
        }
        /*For other operations, no operand should be present*/
        else if (separateOp[i].operand != 2147483647) /*But if operand is present*/
        {
            char error[50];
            sprintf(error, "Line %d: error: Unexpected operand\n",
                i + 1); /*Issue an error in log file*/
            fputs(error, logFile);
            if (e == 0)
                printf("Line %d: error: Unexpected operand\n", i + 1);
            e = 1;
        }
    }
}


/*PASS 2*/

void formLst(char* code[], int len, char* fl)
{
    FILE *lstFile, *objFile;
    char *pcExt, fl1[1000], fl2[1000];
    int i, pc = 0, v2, pos, *arr, tmp, c = 0;
    arr = (int*)malloc(len * sizeof(int));

    strcpy(fl1, fl);
    strcpy(fl2, fl);
    strcat(fl1, ".l");
    strcat(fl2, ".o");
    lstFile = fopen(fl1, "w"); /*Open list file in text mode*/

    for (i = 0; i < len; i++)
    {
        tmp = 0;
        if (code[i][0] != '*') /*If line is not empty*/
        {
            char val[1000];
            pcExt = extend(pc, 8); /*Find hex form of the program counter*/
            strcpy(val, pcExt);
            /*v1 = strtol(pcExt, NULL, 16);*/
            strcat(val, " ");

            if (separateOp[i].mnemonic[0] == '\0') /*If no mnemonic found*/
            {
                strcat(val, "        ");
                tmp = 1;
            }
            else /*If mnemonic present*/
            {
                if (separateOp[i].operand == 2147483647
                    && separateOp[i].opLabel[0] == '\0') /*If no operand present*/
                {
                    pos = position(separateOp[i].mnemonic); /*Find opcode of mnemonic*/
                    if (pos != -1) /*Mnemonic is valid*/
                    {
                        pcExt = extend(pos, 8); /*Extend opcode to 32 bits*/
                        strcat(val, pcExt);
                        v2 = strtol(pcExt, NULL, 16);
                    }
                    else /*Mnemonic is invalid*/
                    {
                        char error[50];
                        sprintf(error, "Line %d: error: Wrong mnemonic\n",
                            i + 1); /*Issue an error in log file*/
                        fputs(error, logFile);
                        if (e == 0)
                            printf("Line %d: error: Wrong mnemonic\n", i + 1);
                        e = 1;
                    }
                }
                else /*Operand is present*/
                {
                    char *pcExt1, *pcExt2;
                    if (separateOp[i].operand != 2147483647
                        && separateOp[i].operand != -2147483648) /*Operand is a number*/
                    {
                        pcExt1 = extend(separateOp[i].operand, 6); /*Extend operand to 24 bits*/
                        strcat(val, pcExt1);
                    }
                    else if (separateOp[i].opLabel[0] != '\0') /*If operand is a label*/
                    {
                        int lblAddr
                            = findLabelAddr(separateOp[i].opLabel); /*Find address of that label*/
                        if (lblAddr == -1) /*Label not present in sym_tab*/
                        {
                            char error[50];
                            sprintf(error, "Line %d: error: No such label\n",
                                i + 1); /*Issue an error in log file*/
                            fputs(error, logFile);
                            pcExt1 = extend(0, 6);
                            if (e == 0)
                                printf("Line %d: error: No such label\n", i + 1);
                            e = 1;
                        }
                        else /*Label present in sym_tab*/
                        {
                            if (strcmp(separateOp[i].mnemonic, "br") == 0
                                || strcmp(separateOp[i].mnemonic, "ldl") == 0
                                || strcmp(separateOp[i].mnemonic, "stl") == 0
                                || strcmp(separateOp[i].mnemonic, "ldnl") == 0
                                || strcmp(separateOp[i].mnemonic, "stnl") == 0
                                || strcmp(separateOp[i].mnemonic, "call") == 0
                                || strcmp(separateOp[i].mnemonic, "brz") == 0
                                || strcmp(separateOp[i].mnemonic, "brlz") == 0)

                                lblAddr = lblAddr - pc - 1; /*Find offset*/
                            pcExt1 = extend(lblAddr, 6); /*Extend operand to 24 bits*/
                            strcat(val, pcExt1);
                        }
                    }
                    else
                    {
                        pcExt1 = extend(0, 6);
                        strcat(val, pcExt2);
                    }
                    pos = position(separateOp[i].mnemonic); /*Find opcode of mnemonic*/
                    if (pos != -1) /*Mnemonic found*/
                    {
                        pcExt2 = extend(pos, 2); /*Extend opcode to 8 bits*/
                        strcat(val, pcExt2);
                        strcat(pcExt1, pcExt2);
                        v2 = strtol(pcExt1, NULL, 16);

                        if (!strcmp(separateOp[i].mnemonic, "br")
                            || !strcmp(separateOp[i].mnemonic, "brz")
                            || !strcmp(separateOp[i].mnemonic, "brlz")
                            || !strcmp(separateOp[i].mnemonic, "call"))
                        {
                            if (findLabelAddr(separateOp[i].opLabel) == pc)
                            {
                                char warning[50];
                                sprintf(
                                    warning, "Line %d: warning: Probable infinite loop\n", i + 1);
                                fputs(warning, logFile);
                            }
                        }
                    }
                    else /*Mnemonic not found in operations table*/
                    {
                        char error[50];
                        sprintf(error, "Line %d: error: Wrong mnemonic\n",
                            i + 1); /*Issue an error in log file*/
                        fputs(error, logFile);
                        if (e == 0)
                            printf("Line %d: error: Wrong mnemonic\n", i + 1);
                        e = 1;
                    }
                }
            }
            strcat(val, " ");

            strcat(val, code[i]); /*Add the corresponding code line to the program counter and
                                     machine code*/
            strcat(val, "\n");
            fputs(val, lstFile);

            if (tmp == 0)
                arr[c++] = v2; /*Store machine code in array*/
        }
        if (separateOp[i].mnemonic[0] != '\0')
            pc++; /*Increment program counter if mnemonic present in the line*/
    }
    
    fclose(lstFile);
    
    if (e == 0)
    {
	objFile = fopen(fl2, "wb"); /*Open object file in binary mode*/
	fwrite(arr, sizeof(int), c, objFile); /*Write the machine code in the object file*/
	fclose(objFile); /*Close list and object files*/
    }
}

int main(int argc, char** argv)
{
    FILE* fp;
    char line[1000], *formattedLine, **code, *fl, fl1[1000];
    int len = 0, fileLen, i;

    fillOperations();

    if (argc >= 2)
        fp = fopen(argv[1], "r"); /*Input asm file*/

    fileLen = strlen(argv[1]) - 3;
    fl = (char*)malloc(fileLen * sizeof(char));
    for (i = 0; i < fileLen - 1; i++)
        fl[i] = argv[1][i];
    fl[i] = '\0'; /*Input filename without extension*/

    strcpy(fl1, fl);
    strcat(fl1, ".log");
    logFile = fopen(fl1, "w"); /*Write errors and warnings in log file*/

    if (fp == NULL)
    {
        perror("Error opening file");
        return (-1);
    }
    while (fgets(line, 1000, fp))
        len++; /*Count number of lines in input file*/

    code = (char**)malloc(len * sizeof(char*));
    len = 0;

    fseek(fp, 0, SEEK_SET);

    while (fgets(line, 1000, fp)) /*Read input file line by line*/
    {
        formattedLine = removeComment(line); /*Remove comments from each line*/
        if (!isEmpty(formattedLine)) /*If line is not empty*/
            code[len++] = formattedLine;
        else /*If empty line*/
            code[len++] = (char*)"*";
    }

    separateOperation(code, len);

    checkValidOp(len);
    fillSymTab(len); /*PASS 1*/
    formLst(code, len, fl); /*PASS 2*/

    unusedLabel(len);

    fclose(fp);
    fclose(logFile);

    return 0;
}
