#define VERSION 0
#include <stdio.h>
#include "string.h"
#include "vmManage.h"


int main(int argc, char *argv[]) {
    printf("//////////////////////\n");
    printf("Project Oxygen V%d\n",VERSION);
    printf("//////////////////////\n");
    if(argv[1]==0){
        printf("Invalid Request To Oxygen...\n");
        return 1;
    }else if(strcmp(argv[1],"-h")==0){
        printf("Check Oxygen Instructions -> Oxygen.pdf\n");
        return 0;
    }else if(strcmp(argv[1],"-c")==0){
        printf("Oxygen -> Create...\n");
        if(argv[2]==0){
            printf("Owner ID Not Supplied\n");
            return 1;
        } else if(argv[3]==0){
            printf("VM ID Not Supplied...\n");
            return 1;
        } else if(argv[4]==0){
            printf("HasAutoRun Not Supplied...\n");
            return 1;
        } else if(argv[5]==0){
            printf("CBPack Info Not Supplied...\n");
            return 1;
        }
        printf("Creating!\n");
    }

        //generate random number to return as main output? file with info maybe
    return 0;
}