@echo off
del bzh_dell_smm_lowlevel.lib
del bzh_dell_smm_lowlevel.obj
del bzh_dell_smm_lowlevel.lst
cls
ml64 /c /Fl bzh_dell_smm_lowlevel.asm
lib /name:bzh_dell_smm_lowlevel.lib bzh_dell_smm_lowlevel.obj
dumpbin /all bzh_dell_smm_lowlevel.lib