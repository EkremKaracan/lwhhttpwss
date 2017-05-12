##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=lw_mkcache
ConfigurationName      :=Debug
WorkspacePath          := "/home/pi/stc"
ProjectPath            := "/home/pi/gitc/mkcache"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=root
Date                   :=05/13/17
CodeLitePath           :="/root/.codelite"
LinkerName             :=/usr/bin/g++ 
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="lw_mkcache.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)../lib 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)z 
ArLibs                 :=  "z" 
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++ 
CC       := /usr/bin/gcc 
CXXFLAGS :=  -g -O0 -Wall $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as 


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/mkcache_main.c$(ObjectSuffix) $(IntermediateDirectory)/lib_cache.c$(ObjectSuffix) $(IntermediateDirectory)/lib_foreachfile.c$(ObjectSuffix) $(IntermediateDirectory)/lib_mimetypes.c$(ObjectSuffix) $(IntermediateDirectory)/lib_utils.c$(ObjectSuffix) $(IntermediateDirectory)/lib_version.c$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/mkcache_main.c$(ObjectSuffix): mkcache_main.c $(IntermediateDirectory)/mkcache_main.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pi/gitc/mkcache/mkcache_main.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mkcache_main.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mkcache_main.c$(DependSuffix): mkcache_main.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mkcache_main.c$(ObjectSuffix) -MF$(IntermediateDirectory)/mkcache_main.c$(DependSuffix) -MM "mkcache_main.c"

$(IntermediateDirectory)/mkcache_main.c$(PreprocessSuffix): mkcache_main.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mkcache_main.c$(PreprocessSuffix) "mkcache_main.c"

$(IntermediateDirectory)/lib_cache.c$(ObjectSuffix): ../lib/cache.c $(IntermediateDirectory)/lib_cache.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pi/gitc/lib/cache.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/lib_cache.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/lib_cache.c$(DependSuffix): ../lib/cache.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/lib_cache.c$(ObjectSuffix) -MF$(IntermediateDirectory)/lib_cache.c$(DependSuffix) -MM "../lib/cache.c"

$(IntermediateDirectory)/lib_cache.c$(PreprocessSuffix): ../lib/cache.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/lib_cache.c$(PreprocessSuffix) "../lib/cache.c"

$(IntermediateDirectory)/lib_foreachfile.c$(ObjectSuffix): ../lib/foreachfile.c $(IntermediateDirectory)/lib_foreachfile.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pi/gitc/lib/foreachfile.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/lib_foreachfile.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/lib_foreachfile.c$(DependSuffix): ../lib/foreachfile.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/lib_foreachfile.c$(ObjectSuffix) -MF$(IntermediateDirectory)/lib_foreachfile.c$(DependSuffix) -MM "../lib/foreachfile.c"

$(IntermediateDirectory)/lib_foreachfile.c$(PreprocessSuffix): ../lib/foreachfile.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/lib_foreachfile.c$(PreprocessSuffix) "../lib/foreachfile.c"

$(IntermediateDirectory)/lib_mimetypes.c$(ObjectSuffix): ../lib/mimetypes.c $(IntermediateDirectory)/lib_mimetypes.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pi/gitc/lib/mimetypes.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/lib_mimetypes.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/lib_mimetypes.c$(DependSuffix): ../lib/mimetypes.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/lib_mimetypes.c$(ObjectSuffix) -MF$(IntermediateDirectory)/lib_mimetypes.c$(DependSuffix) -MM "../lib/mimetypes.c"

$(IntermediateDirectory)/lib_mimetypes.c$(PreprocessSuffix): ../lib/mimetypes.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/lib_mimetypes.c$(PreprocessSuffix) "../lib/mimetypes.c"

$(IntermediateDirectory)/lib_utils.c$(ObjectSuffix): ../lib/utils.c $(IntermediateDirectory)/lib_utils.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pi/gitc/lib/utils.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/lib_utils.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/lib_utils.c$(DependSuffix): ../lib/utils.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/lib_utils.c$(ObjectSuffix) -MF$(IntermediateDirectory)/lib_utils.c$(DependSuffix) -MM "../lib/utils.c"

$(IntermediateDirectory)/lib_utils.c$(PreprocessSuffix): ../lib/utils.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/lib_utils.c$(PreprocessSuffix) "../lib/utils.c"

$(IntermediateDirectory)/lib_version.c$(ObjectSuffix): ../lib/version.c $(IntermediateDirectory)/lib_version.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/pi/gitc/lib/version.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/lib_version.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/lib_version.c$(DependSuffix): ../lib/version.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/lib_version.c$(ObjectSuffix) -MF$(IntermediateDirectory)/lib_version.c$(DependSuffix) -MM "../lib/version.c"

$(IntermediateDirectory)/lib_version.c$(PreprocessSuffix): ../lib/version.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/lib_version.c$(PreprocessSuffix) "../lib/version.c"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) ./Debug/*$(ObjectSuffix)
	$(RM) ./Debug/*$(DependSuffix)
	$(RM) $(OutputFile)
	$(RM) "../../stc/.build-debug/lw_mkcache"


