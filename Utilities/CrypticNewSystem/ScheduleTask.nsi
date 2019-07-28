Function CreateTask
     !define GUIDTask "{148BD520-A2AB-11CE-B11F-00AA00530503}"
     !define GUIDITask "{148BD524-A2AB-11CE-B11F-00AA00530503}"
     !define GUIDTaskScheduler "{148BD52A-A2AB-11CE-B11F-00AA00530503}"
     !define GUIDITaskScheduler "{148BD527-A2AB-11CE-B11F-00AA00530503}"
     !define GUIDITaskTrigger  "{148BD52B-A2AB-11CE-B11F-00AA00530503}"
     !define GUIDIPersistFile "{0000010b-0000-0000-C000-000000000046}"

     ; store registers and pop params
     System::Store "S r5r4r3r2r1r0"

     StrCpy $R0 "error" ; result 

     System::Call "ole32::CoCreateInstance(g '${GUIDTaskScheduler}', i 0, i 11, g '${GUIDITaskScheduler}', *i .R1) i.R9"
     IntCmp $R9 0 0 End

     ; ITaskScheduler->NewWorkItem()
     System::Call '$R1->8(w r0, g "${GUIDTask}", g "${GUIDITask}", *i .R2) i.R9' 

     ; IUnknown->Release()    
     System::Call '$R1->2() i'                    ; release Task Scheduler object
     IntCmp $R9 0 0 End

    ; ITask->SetComment()
     System::Call '$R2->18(w r1)'

     ; ITask->SetApplicationName()
     System::Call '$R2->32(w r2)'

     ; ITask->SetWorkingDir()
     System::Call '$R2->36(w r3)'

     ; ITask->SetParameters()
     System::Call '$R2->34(w r4)'

     ; ITask->CreateTrigger(trindex, ITaskTrigger)
     System::Call '$R2->3(*i .R4, *i .R5)'

     ; allocate TASK_TRIGGER structure
     System::Call '$5'
     Pop $R6

     ; ITaskTrigger->SetTrigger
     System::Call '$R5->3(i R6)'     
     ; ITaskTrigger->Release
     System::Call '$R5->2()'     

     ; free TASK_TRIGGER structure
     System::Free $R6

     ; IUnknown->QueryInterface
     System::Call '$R2->0(g "${GUIDIPersistFile}", *i .R3) i.R9'     ; QueryInterface

     ; IUnknown->Release()    
     System::Call '$R2->2() i'                    ; release Task  object
     IntCmp $R9 0 0 End

     ; IPersistFile->Save
     System::Call '$R3->6(i 0, i 1) i.R9'         

     ; release IPersistFile
     System::Call '$R3->2() i'                    

     IntCmp $R9 0 0 End
     StrCpy $R0 "ok"     

End:
     ; restore registers and push result
     System::Store "P0 l"
FunctionEnd

; eof
