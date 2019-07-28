again:
  EnumCDs::next /NOUNLOAD
  Pop $0
  StrCmp $0 "error" done
  DetailPrint $0
StrCmp $0 "done" done again
done:
  EnumCDs::next
  Pop $0
