//
// Created by 010010 on 15/06/2020.
//

#include "vmManage.h"
#include <stdio.h>
#include <sys/wait.h>
#include "iLibX.h"
#include <stdlib.h>
#define BUFF 800

//Pass port of VM, returns 0 if VM active and accepting SSH transmissions
int heartBeat(char *port){
    char *ping = "nc -z";
    char *silence = ">/dev/null 2>/dev/null";
    char command[800];
    sprintf(command,"%s 127.0.0.1 %s %s",ping,port,silence);
    FILE * shell;
    shell = popen(command, "r");
    int r = pclose(shell);
    if(WEXITSTATUS(r)==0)
    {
        return 0;
    } else {
        return 1;
    }
}

//add database value checks (just for double checking...)
//VMID should be a 4 digit number

int prepdirs(){
    macos_run_comm("rm -rf oxygenData");
    if(macos_run_ge("mkdir oxygenData oxygenData/Users")==0){
        macos_run_comm("cp Master.zip oxygenData/Master.zip");
        system("cp Oxygen oxygenData/Oxygen");
        macos_run_ge("cd oxygenData && unzip Master.zip");
        return 0;
    } else{
        return 1;
    }
}

int clonerepo(char *pre,char *repoaddr){
    char clone[1200];
    sprintf(clone,"%s git clone %s",pre,repoaddr);
    if(macos_run_ge(clone)==0){
        return 0;
    } else{
        return 1;
    }
}

int init(char *iosurl,char *devicetype,char *codename, char *rootfs, char *updatedmg) {
    printf("Preparing Directory Structure...\n");
    if (prepdirs()==0) {
        printf("Directory Structure Created!\n");
        printf("Cloning XNU-QEMU-64\n");
        if (clonerepo("cd oxygenData/Master &&", "https://github.com/alephsecurity/xnu-qemu-arm64.git") == 0) {
            printf("Success!\n");
            printf("Cloning XNU-QEMU-64-Tools\n");
            if (clonerepo("cd oxygenData/Master &&", "https://github.com/alephsecurity/xnu-qemu-arm64-tools") == 0) {
                printf("Success!\n");
                printf("Cloning Darwin XNU\n");
                if (clonerepo("cd oxygenData/Master &&", "https://github.com/apple/darwin-xnu.git") == 0) {
                    printf("Success!\n");
                    printf("Downloading iOS for %s\n", devicetype);
                    char iURL[2400];
                    sprintf(iURL, "cd oxygenData/Master && curl %s --output rootfs.zip", iosurl);
                    if (macos_run_ge(iURL)==0) {
                        printf("Downloaded iOS...Extracting!\n");
                        if (macos_run_ge("cd oxygenData/Master && unzip rootfs.zip") == 0) {
                            printf("Success!\n");
                            char patchmdir[6000];
                            sprintf(patchmdir,"sed -i '' 's/MASTERDIR/\\/Users\\/oxygen\\/Documents\\/Oxygen\\/OxygenData\\/Master/g' oxygenData/Master/serverInit.sh");
                            if(macos_run_ge(patchmdir)!=20){
                                char patchrdisk[6000];
                                sprintf(patchrdisk,"sed -i '' 's/UPDATERAMDISK/%s/g' oxygenData/Master/serverInit.sh",updatedmg);
                                if(macos_run_ge(patchrdisk)!=20){
                                    char patchmdisk[6000];
                                    sprintf(patchmdisk,"sed -i '' 's/MAINDISK/%s/g' oxygenData/Master/serverInit.sh",rootfs);
                                    if(macos_run_ge(patchmdisk)!=20){
                                        char patchcname[6000];
                                        sprintf(patchcname,"sed -i '' 's/CNAME/%s/g' oxygenData/Master/serverInit.sh",codename);
                                        if(macos_run_ge(patchcname)==0){
                                            printf("Setting Up Master Image\n");
                                            system("cd oxygenData/Master && ./serverInit.sh");

                                        }
                                    }
                                }
                            }


                        }
                    }
                }
            }
        }
    } else{
        printf("Directory Structure Could Not Be Created...\n");
        return 1;
    }
}


int generateSubVM(char *owner,char *vmid, char *hasauto, char *cbpack, char *mdir){
    //Create User Directory
    char ownercreate[BUFF];
    sprintf(ownercreate,"mkdir Users/%s Users/%s/%s",owner,owner,vmid);
    if(macos_run_ge(ownercreate)==0){
        //Copy Master Files
        char copyxnu[9000];
        sprintf(copyxnu,"cp -R Master/xnu-qemu-arm64/ Users/%s/%s/xnu-qemu-arm64 && cp Master/kernelcache.release.n66.out Users/%s/%s/kernelcache.release.n66.out && cp Master/dtree Users/%s/%s/dtree && cp Master/aleph_bdev_drv.bin Users/%s/%s/aleph_bdev_drv.bin && cp Master/hfs.main Users/%s/%s/hfs.main && cp Master/hfs.sec Users/%s/%s/hfs.sec && cp Master/static_tc Users/%s/%s/static_tc",owner,vmid,owner,vmid,owner,vmid,owner,vmid,owner,vmid,owner,vmid,owner,vmid);
        if(macos_run_ge(copyxnu)==0){
            //Set Custom Port
            char attach[BUFF];
            sprintf(attach,"cd Users/%s/%s/ && hdiutil attach -imagekey diskimage-class=CRawDiskImage hfs.main",owner,vmid);
            system(attach);
            char repval[4000];
            sprintf(repval,"sudo sed -i '' 's/2222/%s/g' /Volumes/PeaceB16B92.arm64UpdateRamDisk/System/Library/LaunchDaemons/tcptunnel.plist",vmid);
            system("cp /Volumes/PeaceB16B92.arm64UpdateRamDisk/System/Library/LaunchDaemons/tcptunnel.plist ~/t.plist");
            system(repval);
            system("cp /Volumes/PeaceB16B92.arm64UpdateRamDisk/System/Library/LaunchDaemons/tcptunnel.plist ~/patched.plist");
            system("hdiutil detach /Volumes/PeaceB16B92.arm64UpdateRamDisk");
            system(attach);
            system("hdiutil detach /Volumes/PeaceB16B92.arm64UpdateRamDisk");
            char attachsec[BUFF];
            sprintf(attachsec,"cd Users/%s/%s/ && hdiutil attach -imagekey diskimage-class=CRawDiskImage hfs.main",owner,vmid);
            system(attachsec);
            system("hdiutil detach /Volumes/PeaceB16B92.arm64UpdateRamDisk");
            printf("VM Ready For Launch\n");
        } else{
            printf("SERVER ERROR\n");
        }
    }
}

void startVM(char *owner, char *vmid){
    char vmStart[4000];
    sprintf(vmStart,"cd Users/%s/%s/ && xnu-qemu-arm64/aarch64-softmmu/qemu-system-aarch64 -M iPhone6splus-n66-s8000,kernel-filename=kernelcache.release.n66.out,dtb-filename=dtree,driver-filename=aleph_bdev_drv.bin,qc-file-0-filename=hfs.main,qc-file-1-filename=hfs.sec,tc-filename=static_tc,kern-cmd-args=\"debug=0x8 kextlog=0xfff cpus=1 rd=disk0 serial=2\",xnu-ramfb=on -cpu max -m 6G -serial mon:stdio",owner,vmid);
    system(vmStart);
    char attachsec2[BUFF];
    sprintf(attachsec2,"cd Users/%s/%s/ && hdiutil attach -imagekey diskimage-class=CRawDiskImage hfs.sec",owner,vmid);
    system(attachsec2);
    system("hdiutil detach /Volumes/PeaceB16B92.arm64UpdateRamDisk");
}