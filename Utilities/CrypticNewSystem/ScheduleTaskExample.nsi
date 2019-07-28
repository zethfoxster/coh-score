; This is just an example of System Plugin
; 
; (c) brainsucker, 2002
; (r) BSForce

Name "System Plugin Example"
OutFile "Sheduletask.exe"
SetPluginUnload  alwaysoff

!include "ScheduleTask.nsi"

Section "ThisNameIsIgnoredSoWhyBother?"
    SetOutPath $TEMP

     ; general TASK_TRIGGER structure arguments
     ; 1, 2 - skip
     ; 3,4,5 - BEGIN year, month, day
     ; 6,7,8 - END year, month, day (SHOULD be = 0, if 1 flag is skipped)
     ; 9, 10 - Start hour, minute
     ; 11, 12 - Duration and Interval of task in minutes. (duration - the whole 
     ;          time task will work, interval - time between runs. D = 120, 
     ;          I = 10: task will be runned every 10 minutes for 2 hours).
     ; 13 - flags (should be ored (|)):  
     ;          1 - task has end date (6,7,8 defined)
     ;          2 - task will be killed at Duration end
     ;          4 - task trigger disabled
     ; 14 - trigger type: there are 7 different types, every one with it own 
     ;          structure
     ;       0 = ONCE        task will be runned once
     ;       5 = On IDLE     task will run on system IDLE (ITask->SetIdleWait)
     ;       6 = At System Startup
     ;       7 = At Logon
     ; these types use the following structure (so 7 means trigger at Logon):
     ;    push "*(&l2, &i2 0, &i2 2003, &i2 9, &i2 4, &i2 0, &i2 0, &i2 0, &i2 14, &i2 20, i 0, i 0, i 0, i 7, i 0, &i2 0, i 0, &i2 0) i.s"     
     ;       1 = DAILY  - field 15 gives interval in days (here it 15)
     ;    push "*(&l2, &i2 0, &i2 2003, &i2 9, &i2 3, &i2 0, &i2 0, &i2 0, &i2 13, &i2 10, i 0, i 0, i 0, i 1, &i2 15, i 0, i 0, &i2 0) i.s"
     ;       2 = WEEKLY  - field 15 gives interval in weeks (here 17), 
     ;                  field 16 - shows which days to run (OR them with |): 
     ;                  Sunday-Saturday (0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40)
     ;                  in example monday and friday
     ;    push "*(&l2, &i2 0, &i2 2003, &i2 9, &i2 3, &i2 0, &i2 0, &i2 0, &i2 13, &i2 10, i 0, i 0, i 0, i 2, &i2 13, &i2 0x2|0x20, &i2 0, i 0, &i2 0) i.s"
     ;       3 = MONTHLYDATE  - field 15 bit field of days (OR them) to run 
     ;                          (0x1-0x40000000), 
     ;                  field 16 - bit field of month (OR them with |): 
     ;                  January-December (0x1-0x800)
     ;                  in example (3 and 5 days of February and June)
     ;    push "*(&l2, &i2 0, &i2 2003, &i2 9, &i2 3, &i2 0, &i2 0, &i2 0, &i2 13, &i2 10, i 0, i 0, i 0, i 3, i 0x4|0x20, &i2 0x2|0x20, i 0, &i2 0) i.s"
     ;       4 = MONTHLYDOW  - field 15 week of month to run (1-5)
     ;                  field 16 - shows which days to run (OR them with |): 
     ;                  Sunday-Saturday (0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40)
     ;                  field 17 - bit field of month (or them with |): 
     ;                  January-December (0x1-0x800)
     ;                  in example (first week, monday and friday of February and June)
     ;    push "*(&l2, &i2 0, &i2 2003, &i2 9, &i2 3, &i2 0, &i2 0, &i2 0, &i2 13, &i2 10, i 0, i 0, i 0, i 4, &i2 1, &i2 0x2|0x20, &i2 0x2|0x20, i 0, &i2 0) i.s"

     push "My Task"
     push "My Task Comment"
     push "c:\Working Dir\My Task Program.exe"
     push "c:\Working Dir"
     push "My Program Args"        
     push "*(&l2, &i2 0, &i2 2003, &i2 9, &i2 3, &i2 0, &i2 0, &i2 0, &i2 9, &i2 0, i 0, i 0, i 0, i 2, &i2 1, &i2 0x2, &i2 0, i 0, &i2 0) i.s"   
     Call CreateTask
     Pop $0
     MessageBox MB_OK "CreateTask result: $0"

     ; last plugin call must not have /NOUNLOAD so NSIS will be able to delete the temporary DLL
     SetPluginUnload manual
     ; do nothing
     System::Free 0

SectionEnd 

; eof
