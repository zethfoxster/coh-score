
typedef struct BuildCommand;

void runBuildCommandSystem( BuildCommand *command );
void runBuildCommandRun( BuildCommand *command );
void runBuildCommandPushDir( BuildCommand *command );
void runBuildCommandPopDir( BuildCommand *command );
void runBuildCommandKillProcesses( BuildCommand *command );
void runBuildCommandWaitForProcs( BuildCommand *command );
//FileScanAction deleteFileCallback( FileScanContext *context );
void runBuildCommandDeleteFiles( BuildCommand *command );
void runBuildCommandCopy( BuildCommand *command );
void runBuildCommandRename( BuildCommand *command );
void runBuildCommandDeleteFolder( BuildCommand *command );
void runBuildCommandMakeFolder( BuildCommand *command );
void runExternalCommand( const char *fileToRun, const char *cmdLine, char *procName, int detach );
void runBuildCommandGimme( BuildCommand *command );
void runBuildCommandSVN( BuildCommand *command );
void runBuildCommandInFolder( BuildCommand *command, const char *cmdName, const char *folderVarName );
void runBuildCommandTool( BuildCommand *command );
void runBuildCommandCompile( BuildCommand *command );
void runBuildCommandBuild( BuildCommand *command );
void runBuildCommandSrcExe( BuildCommand *command );
void runBuildCommandFunction( BuildCommand *command );
void runBuildCommandWriteLogFile( BuildCommand *command );

void runBuildCommandPrint( BuildCommand *command );
void runBuildCommandError( BuildCommand *command );
void runBuildCommandSleep( BuildCommand *command );

void runBuildCommandVarAdd( BuildCommand *command );
void runBuildCommandVarSub( BuildCommand *command );
void runBuildCommandVarMul( BuildCommand *command );
void runBuildCommandVarDiv( BuildCommand *command );
void runBuildCommandListSort( BuildCommand *command );

void runBuildCommandProcKill( BuildCommand *command );