#!/usr/bin/awk -f 
BEGIN { 
    FS=":| ";}
{
    cmd="sudo timeout 2s ./arp.out eth0 " $4
    print cmd
    ret=system(cmd)
    print cmd, (ret==0)?"Sucess":"Fail"
}