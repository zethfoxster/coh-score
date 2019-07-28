@cd test1

@del AAA_testpackage.pkg

..\..\..\bin\debug\streamingclientprototype -createheader AAA_files.txt -markzero sf_boot_rocket_01.texture -create

@cd ..

@del /q test2\*.*
@copy test1\AAA_testpackage.pkg test2\AAA_testpackage.pkg

@cd test2


..\..\..\bin\debug\streamingclientprototype -loadheader AAA_testpackage.pkg -unpackfile sf_boot_rocket_01.texture -unpackfile sf_boot_rocket_01_bump.texture

..\..\..\bin\debug\streamingclientprototype -loadheader AAA_testpackage.pkg -markunavailable sf_boot_rocket_01_dual.texture -markunavailable sf_boot_schoolgirl_01b.texture -markunavailable sf_boot_skin_bare.texture -flushavailable

@copy AAA_testpackage.pkg AAA_testpackage-0.pkg

@copy ..\test1\sf_boot_rocket_01.texture .
@copy ..\test1\sf_boot_rocket_01_dual.texture .

..\..\..\bin\debug\streamingclientprototype -loadheader AAA_testpackage.pkg -updatefile sf_boot_rocket_01.texture

@copy AAA_testpackage.pkg AAA_testpackage-1.pkg

..\..\..\bin\debug\streamingclientprototype -loadheader AAA_testpackage.pkg -updatefile sf_boot_rocket_01_dual.texture

@cd ..
