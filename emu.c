/*
Oindrila Bhadra
1801CS34
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*implementing an unordered_map as in C++*/
/*One node is one memory location*/
struct node
{
    int key; /*Memory address*/
    int val; /*Value stored there*/
    struct node* next; /*Pointer to next node*/
};

/*Create entire memory structure consisting of a size variable and nodes*/
struct memory
{
    int size;
    struct node** list;
};

/*Initialise the memory*/
struct memory* createMemoryTable(int size)
{
    struct memory* t = (struct memory*)malloc(sizeof(struct memory));
    int i;

    t->size = size;
    t->list = (struct node**)malloc(sizeof(struct node*) * size);

    for (i = 0; i < size; i++)
        t->list[i] = NULL; /*Fill all the memory spaces as NULL initially*/
    return t;
}

/*Specify name,opcode and operand type of all mnemonics*/
struct instruction
{
    char* mne;
    int opc;
    char oprType; /*'O' for offset, 'V' for value, 'N' for none*/
};

struct instruction* operations;

FILE* output;

/*Return the position of the node in the memory space*/
int hashCode(struct memory* t, int key)
{
    if (key < 0)
        return -(key % t->size);
    return key % t->size;
}

/*Insert values into memory*/
void insert(struct memory* t, int key, int val,
    int flag) /*flag=0 when values are inserted in the memory for the first time, otherwise flag=0*/
{
    int pos, opc;
    struct node *list, *newNode, *temp;

    opc = val & 255; /*Extract last 8 bits of machine code*/

    if (opc == 19 && flag == 0) /*If instruction is data, then we will store the operand only into
                                   the memory without the opcode*/
        val >>= 8; /*Extract the 1st 24 bits of the machine code*/

    pos = hashCode(t, key); /*Find the position of memory location key in the memory*/
    list = t->list[pos];
    newNode = (struct node*)malloc(sizeof(struct node)); /*This memory address will be inserted*/
    temp = list;
    while (temp) /*Check if that position is already filled*/
    {
        if (temp->key
            == key) /*If the address at this position is equal to the address at which the value is
                       to be inserted*/
        {
            temp->val = val; /*Replace the value*/
            return;
        }
        temp = temp->next; /*Move to the next memory space*/
    }

    /*If the memory address is not already filled*/
    newNode->key = key;
    newNode->val = val;
    newNode->next = list;
    t->list[pos] = newNode; /*Insert the newNode at the position of key*/
}

/*Search if the key is present in the memory or not and return the value stored there.
Key here denotes the memory location to be searched.*/
int lookup(struct memory* t, int key)
{
    int pos = hashCode(t, key);
    struct node* list = t->list[pos];
    struct node* temp = list;
    while (temp)
    {
        if (temp->key == key) /*If that memory location is found*/
            return temp->val;
        temp = temp->next;
    }
    return -2147483648; /*If that memory location is empty*/
}

/*Insert values into the operations table*/
void insertOp(char* mn, int op, char t)
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
    insertOp((char*)"ldc", 0, 'V');
    insertOp((char*)"adc", 1, 'V');
    insertOp((char*)"ldl", 2, 'O');
    insertOp((char*)"stl", 3, 'O');
    insertOp((char*)"ldnl", 4, 'O');
    insertOp((char*)"stnl", 5, 'O');
    insertOp((char*)"add", 6, 'N');
    insertOp((char*)"sub", 7, 'N');
    insertOp((char*)"shl", 8, 'N');
    insertOp((char*)"shr", 9, 'N');
    insertOp((char*)"adj", 10, 'V');
    insertOp((char*)"a2sp", 11, 'N');
    insertOp((char*)"sp2a", 12, 'N');
    insertOp((char*)"call", 13, 'O');
    insertOp((char*)"return", 14, 'N');
    insertOp((char*)"brz", 15, 'O');
    insertOp((char*)"brlz", 16, 'O');
    insertOp((char*)"br", 17, 'O');
    insertOp((char*)"HALT", 18, 'N');
    insertOp((char*)"data", 19, 'V');
    insertOp(
        (char*)"SET", 20, 'V'); /*Populate the table with mnemonics, opcodes and operand types.*/
}

/*Sort the array containing memory addresses in ascending order using bubble sort.*/
void sort(int* arr, int n)
{
    int i, j, temp;
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n - i - 1; j++)
        {
            if (arr[j] > arr[j + 1])
            {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/*Print the memory dump*/
void memdump(struct memory* t)
{
    int i, c = 0, arr[1000], last = -1, cnt = 4, idx;
    /*Put those memory addresses into array arr which are not empty.*/
    for (i = 0; i < 1000; i++)
    {
        if (t->list[i] != NULL)
            arr[c++] = t->list[i]->key;
    }
    sort(arr,
        c); /*Sort the memory addresses so that they are printed in ascending order and not in any
               random order*/

    for (i = 0; i < c; i++)
    {
        idx = arr[i] / 4 * 4; /*Print the memory values with 4 values in a row. idx is the address
                                 of the starting value of the row.*/
        if (idx > last) /*New value needs to be stored in another row*/
        {
            last = idx;
            while (cnt < 4) /*If the previous row was not full, fill rest of the spaces with 0 */
            {
                printf("%08x ", 0);
                fprintf(output, "%08x ", 0);
                cnt++;
                if (cnt == 4)
                    printf("\n");
            }
            cnt = 1;
            printf("\n%08x %08x ", idx, lookup(t, arr[i]));
            fprintf(output, "\n%08x %08x ", idx, lookup(t, arr[i]));
        }
        else
        {
            printf("%08x ", lookup(t, arr[i]));
            fprintf(output, "%08x ", lookup(t, arr[i]));
            cnt++; /*Keep count of how many values are getting written onto a row*/
        }
    }
    printf("\n\n");
}

/*Execute the instructions and display the instruction trace*/
void trace(int c, struct memory* t,
    int flag) /*If this function is getting called for -trace, then flag=1. If it is called from
                 -after then flag=0. */
{
    int opr, opc, a = 0, b = 0, pc, sp = 0, cnt = 0, item, addr;

    for (pc = 0; pc < c; pc++)
    {
        addr = lookup(t, pc);
        cnt++; /*Number of instructions executed*/

        opr = addr >> 8; /*First 24 bits is the operand*/
        opc = addr & 255; /*Last 8 bits is the opcode*/

        if (flag == 1) /*Print the trace*/
        {
            printf("PC=%08x, SP=%08x, A=%08x, B=%08x %s", pc, sp, a, b, operations[opc].mne);
            fprintf(
                output, "PC=%08x, SP=%08x, A=%08x, B=%08x %s", pc, sp, a, b, operations[opc].mne);
            if (operations[opc].oprType != 'N') /*If operand is there for the instruction*/
            {
                printf("\t%08x", opr);
                fprintf(output, "\t%08x", opr);
            }
            printf("\n");
            fprintf(output, "\n");
        }
        if (opc == 18) /*Break if HALT*/
            break;
        switch (opc) /*Carry out the operations corresponding to their respective instructions*/
        {
            case 0:
                b = a;
                a = opr;
                break;
            case 1:
                a = a + opr;
                break;
            case 2:
                b = a;
                item = lookup(t, sp + opr);
                if (item != -2147483648) /*If memory location sp+opr is not empty*/
                    a = item;
                else
                    a = 0;
                break;
            case 3:
                insert(t, sp + opr, a, 1);
                a = b;
                break;
            case 4:
                item = lookup(t, a + opr);
                if (item != -2147483648)
                    a = item;
                else
                    a = 0;
                break;
            case 5:
                insert(t, a + opr, b, 1);
                break;
            case 6:
                a = b + a;
                break;
            case 7:
                a = b - a;
                break;
            case 8:
                a = b << a;
                break;
            case 9:
                a = b >> a;
                break;
            case 10:
                sp = sp + opr;
                break;
            case 11:
                sp = a;
                a = b;
                break;
            case 12:
                b = a;
                a = sp;
                break;
            case 13:
                b = a;
                a = pc;
                pc = pc + opr;
                break;
            case 14:
                pc = a;
                a = b;
                break;
            case 15:
                if (a == 0)
                    pc = pc + opr;
                break;
            case 16:
                if (a < 0)
                    pc = pc + opr;
                break;
            case 17:
                pc = pc + opr;
                break;
        }
    }
    printf("%d instructions executed\n", cnt);
    fprintf(output, "%d instructions executed\n", cnt);
}

/*Print the instruction set architecture*/
void isa(void)
{
    int i;
    printf("Opcode\tMnemonic\tOperand\n");
    fprintf(output, "Opcode\tMnemonic\tOperand\n");
    for (i = 0; i < 21; i++) /*Print all 21 instructions from operations table*/
    {
        printf("%d\t%s\t\t", operations[i].opc, operations[i].mne);
        fprintf(output, "%d\t%s\t\t", operations[i].opc, operations[i].mne);
        if (operations[i].oprType == 'V')
        {
            printf("value");
            fprintf(output, "value");
        }
        else if (operations[i].oprType == 'O')
        {
            printf("offset");
            fprintf(output, "offset");
        }
        printf("\n");
        fprintf(output, "\n");
    }
}

int main(int argc, char** argv)
{
    FILE* file;
    int c = 0, val, i, fileLen;
    char fl1[100], *fl;
    struct memory* t = createMemoryTable(1000);

    if (argc != 3) /*If 3 arguments are not present, then print the command format to be followed*/
    {
        printf("usage: emu [options] file.o\n-trace  show instruction trace\n-before show memory "
               "dump before execution\n-after  show memory dump after execution\n-isa    memdump "
               "ISA\n");
        exit(0);
    }

    file = fopen(argv[2], "rb");
    if (file == NULL)
    {
        printf("Error opening file\n");
        exit(0);
    }

    while (fread(&val, sizeof(int), 1, file)) /*Read machine codes from object file one by one*/
    {
        insert(t, c, val, 0); /*Put the machine codes in memory*/
        c++;
    }

    if (c == 0) /*If the object file is empty*/
        printf("Fatal error occurred! No instruction can be found!\n");
    else
    {
        fillOperations();

        fileLen = strlen(argv[2]) - 1;
        fl = (char*)malloc(fileLen * sizeof(char));
        for (i = 0; i < fileLen - 1; i++)
            fl[i] = argv[2][i];
        fl[i] = '\0'; /*Input filename without extension*/

        strcpy(fl1, fl);
        strcat(fl1, ".txt");

        output = fopen(fl1, "w");
        if (strcmp(argv[1], "-before") == 0)
        {
            printf("Memory dump before execution\n\n");
            fprintf(output, "Memory dump before execution\n\n");
            memdump(t);
        }
        else if (strcmp(argv[1], "-isa") == 0)
            isa();
        else if (strcmp(argv[1], "-trace") == 0)
            trace(c, t, 1);
        else if (strcmp(argv[1], "-after")
            == 0) /*For printing memory dump after instructions are executed*/
        {
            trace(c, t, 0); /*Execute the instructions first*/
            printf("\nMemory dump after execution\n");
            fprintf(output, "Memory dump after execution\n\n");
            memdump(t);
        }
    }

    return 0;
}
