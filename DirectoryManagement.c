/**************************************************************
* Class:  CSC-415-03 
* Name: Valeria Vallejo, Chung Hei Fong, Erik Chacon
* Student ID: 920594217  917595970  920768287
* Project: File System Project
*
* File: DirectoryManagement.c
*
* Description: Interface of basic Directory Management functions
*
**************************************************************/

#include "mfs.h"
#include "freeSpaceAllocation.h"

// global vars:

char *currentWorkingDirectory; //cwd path
fdDir *curWorkingDirPtr; // points to fdDir of cwd
DirectoryEntry root[NUM_DIR_ENTR]; // always have global root DE
DirectoryEntry *ROOTPTR; // pointer to beginning of root[] (above)
fdDir *globalTemp;  // temp fdDir pointer used whenever we need a temp pointer
fdDir *allDirs[20]; // contains fdDirs of all directories in our file system
int allDirsBitMap[20]; //bit map for previous allDirs[] (above)
static int init = 0; // indicates if root directory has been initialized

int allDirsNumber = 0; // highest index in allDirs array

/***** MAKE DIRECTORY *****/
// associated command: md
int fs_mkdir(const char *pathname)
{
    // if not making root, check to make sure a directory of that name doesn't already exist
    // in our current directory. Error message + exit function if name exists.
    if (init == 1)
    {
        for (int i = 0; i <= curWorkingDirPtr->highestArrayIndex; i++)
        {
            if (strcmp(curWorkingDirPtr->deArray[i].filename, pathname) == 0)
            {
                printf("A directory with this name already exists in the current directory.\n");
                return -1;
            }
        }
    }

    // init a directory entry for the directory if it's root or if the pathname is ok to use
    initDirectoryEntry(pathname, FT_DIRECTORY);
    return 0;
}

/***** REMOVE DIRECTORY *****/
// associated command: rm

// removes a relative directory from file system if it is empty
int fs_rmdir(const char *pathname)
{
    // used to remove empty directories
    for (int i = 0; i <= curWorkingDirPtr->highestArrayIndex; i++)
    {
        // compare current open directory filename and filetype
        if (strcmp(pathname, curWorkingDirPtr->deArray[i].filename) == 0 && curWorkingDirPtr->deArray[i].fileType == FT_DIRECTORY)
        {
            //make sure pathname's dearray is empty (rec len should be 2)
            globalTemp = allDirs[curWorkingDirPtr->deArray[i].allDirsIndex];
            if (globalTemp->d_reclen <= 2)
            {
                //then the dir is empty and we can remove:

                //flip bits in parent's bit array
                flipBits(&curWorkingDirPtr->dirBitMap, i, 1);

                //subtract one from parent's d_reclen to avoid readdir seg fault
                curWorkingDirPtr->d_reclen--;

                //remove from allDirs and flip the bit
                flipBits(allDirsBitMap, curWorkingDirPtr->deArray[i].allDirsIndex, 1);

                //deallocate space in allDirs
                free(allDirs[curWorkingDirPtr->deArray[i].allDirsIndex]);
                allDirs[curWorkingDirPtr->deArray[i].allDirsIndex] = NULL;

                //set to -1 to show that it's been deleted
                curWorkingDirPtr->deArray[i].deID = -1;
            }
            else
            {
                printf("You can only remove an empty directory.\n");
                return -1;
            }
        }
    }
}

// init directory entry function (called in mkdir)
// associated command: md
int initDirectoryEntry(char *pathname, unsigned char fileType)
{
    // create DE for a directory:
    if (fileType == FT_DIRECTORY)
    {
        // make directory entry for parent directory to hold
        DirectoryEntry newDirectoryEntry;
        DirectoryEntry self;
        DirectoryEntry parent;

        // if we need to init root
        if (init == 0)
        {
            // init self (.)
            self.deID = 0; //0th block in directory entry array
            self.blockID = findFreeDataBlocks(1); // get one data block for root to start with
            self.numBlocks = 1;
            strcpy(self.filename, ".");
            self.fileType = fileType;
            self.fileSize = 0;
            self.allDirsIndex = getNextAllDirs(); // gets next index in the allDirs array
            flipBits(allDirsBitMap, self.allDirsIndex, 1); //flip 1 bit at the curr directory's index

            // init parent (..)
            parent.deID = 0; //0th block in directory entry array bc root is its own parent
            parent.blockID = findFreeDataBlocks(1);
            parent.numBlocks = 1;
            strcpy(parent.filename, "..");
            parent.fileType = fileType;
            parent.fileSize = 0;
            parent.allDirsIndex = self.allDirsIndex;

            // all malloc for global vars
            globalTemp = (fdDir *)malloc(sizeof(fdDir));
            curWorkingDirPtr = (fdDir *)malloc(sizeof(fdDir));
            allDirs[0] = (fdDir *)malloc(sizeof(fdDir));
            currentWorkingDirectory = malloc(256);
            // init bit map for all directories
            initAllDirsBitMap(allDirsBitMap);
            // first spot taken
            allDirsBitMap[0] = 1;
            // set first two DE's in root
            root[0] = self;
            root[1] = parent;
            ROOTPTR = &root;
            // init DE Array
            initDEArray(curWorkingDirPtr->deArray);

            // init all directories array with root
            // set current open directory to root
            allDirs[0]->deArray[0] = root[0];
            allDirs[0]->dirName = "/";
            allDirs[0]->d_reclen = 2;
            allDirs[0]->deArrayIndex = 0;
            allDirs[0]->directoryStartLocation = 1;
            allDirs[0]->dirEntryPosition = 0;
            allDirs[0]->deArray[1] = root[1];
            globalTemp = allDirs[0];
            curWorkingDirPtr = allDirs[0];
            initDirBitMap(curWorkingDirPtr->dirBitMap);
            flipBits(&curWorkingDirPtr->dirBitMap, 0, 2);
            //true root has now been initialized
            init = 1;
            // set cwd string
            strcpy(currentWorkingDirectory, "/");
            return 0;
        } // end if root

        // if not root, but is an FT_DIRECTORY, wee use this to init

        // self init
        self.deID = 0; // 0th index in its own DE array
        self.blockID = findFreeDataBlocks(1);
        self.numBlocks = 1;
        strcpy(self.filename, ".");
        self.fileType = fileType;
        self.fileSize = 0;
        self.allDirsIndex = getNextAllDirs(); // get next index in allDirs

        // update allDirsNumber so it always equals the highest index in the allDirs array
        if (self.allDirsIndex > allDirsNumber)
        {
            allDirsNumber = self.allDirsIndex;
        }
        flipBits(allDirsBitMap, self.allDirsIndex, 1);

        // copy directory entry info to the newDirectoryEntyr struct
        // we're working with this one for the rest of the initialization 
        newDirectoryEntry = self;
        // change name so it isn't .
        strcpy(newDirectoryEntry.filename, pathname);

        // make space for new directory in allDirs array
        allDirs[self.allDirsIndex] = (fdDir *)malloc(sizeof(fdDir));
        initFdDir(allDirs[self.allDirsIndex], pathname); // init an fdDir struct for our new directory

        // parent init
        parent = curWorkingDirPtr->deArray[0]; // deArray[0] = self of cwd in cwd's array of DEs
        strcpy(parent.filename, ".."); // this is the parent of the new DE we're making

        // set '.' '..' respectively
        allDirs[self.allDirsIndex]->deArray[0] = self;
        allDirs[self.allDirsIndex]->deArray[1] = parent;
        // flip bits to used
        flipBits(&allDirs[self.allDirsIndex]->dirBitMap, 0, 2);

        // we've only set 0 and 1 in our new directory DE array, so the highest index used
        // at initialization is 1 (this is tracked in each directory's fdDir struct)
        allDirs[self.allDirsIndex]->highestArrayIndex = 1; //highest index taken up = 0

        // find next slot in its parent's DE array
        int nextFreeParentIndex = findNextFreeIndex(curWorkingDirPtr->dirBitMap);

        //set deID to the index in its parent's DE
        newDirectoryEntry.deID = nextFreeParentIndex;

        //update the highest array index that's taken up in the parent's deArray
        if (nextFreeParentIndex > allDirs[parent.allDirsIndex]->highestArrayIndex)
        {
            allDirs[parent.allDirsIndex]->highestArrayIndex = nextFreeParentIndex;
        }

        // add new DE to parent DE array
        allDirs[parent.allDirsIndex]->deArray[nextFreeParentIndex] = newDirectoryEntry;
        // update parent's record lengtha and flip bits
        allDirs[parent.allDirsIndex]->d_reclen++;
        flipBits(&curWorkingDirPtr->dirBitMap, nextFreeParentIndex, 1);

        return 0;
    }
    //create a DE for a regular file:
    else if (fileType == FT_REGFILE)
    {
        DirectoryEntry parent; //use for holder
        DirectoryEntry newDirectoryEntry;

        //setting up attributes to neutral values, filling the ones we know
        newDirectoryEntry.deID = 0; // will get next free open from paretn later
        newDirectoryEntry.blockID = findFreeDataBlocks(1); // get 1 for now, more when we write
        newDirectoryEntry.numBlocks = 1; 
        strcpy(newDirectoryEntry.filename, pathname);
        newDirectoryEntry.fileType = fileType;
        newDirectoryEntry.fileSize = 0; // nothing yet
        newDirectoryEntry.allDirsIndex = -1; // not a directory, so no allDirs index

        // var to hold parent info, not set to anything in our file's DE
        parent = curWorkingDirPtr->deArray[0];

        // find next available spot in our parent's DE array, set to file's deID
        int nextFreeParentIndex = findNextFreeIndex(curWorkingDirPtr->dirBitMap);
        newDirectoryEntry.deID = nextFreeParentIndex;

        //update the highest array index that's taken up in the parent's deArray
        if (nextFreeParentIndex > allDirs[parent.allDirsIndex]->highestArrayIndex)
        {
            allDirs[parent.allDirsIndex]->highestArrayIndex = nextFreeParentIndex;
        }

        //update parent's bit map & record length; flip bits in parent's bitmap
        allDirs[parent.allDirsIndex]->deArray[nextFreeParentIndex] = newDirectoryEntry;
        allDirs[parent.allDirsIndex]->d_reclen++;
        flipBits(&curWorkingDirPtr->dirBitMap, nextFreeParentIndex, 1);

        return 0;
    }

    // return -1 if something went wrong
    printf("Something went wrong in init Directory function\n");
    return -1;
}

/***** OPEN DIRECTORY *****/
// associated command: ls

// looks 
fdDir *fs_opendir(const char *name)
{ 
    // name = path (relative or absolute)
    char *tempPath = malloc(256);
    strcpy(tempPath, name);

    // if absolute path
    if (tempPath[0] == '/' && strlen(name) > 1)
    {

        // tokenize absolute path
        char *dirNames[50]; //holds tokens of all directory names in path
        char *token = strtok(tempPath, "/");
        int i = 0;
        while (token != NULL)
        {
            dirNames[i] = malloc(50);
            dirNames[i] = token;
            i++;
            token = strtok(NULL, "/");
        }

        // set globalTemp to root so we can make our way through the directories
        globalTemp = allDirs[0];

        // counter to keep track of how many tokens we have parsed
        int m = 0;
        // parse through tokens and compare with DE's

        // for every token in the dirNames array
        for (int j = 0; j < i; j++)
        {
            // for every DE in the temp directory we're in
            for (int k = 0; k <= globalTemp->highestArrayIndex; k++)
            {

                //if found correct DE
                if (strcmp(globalTemp->deArray[k].filename, dirNames[j]) == 0)
                {
                    // set temp equal to correct directory we found
                    globalTemp = allDirs[globalTemp->deArray[k].allDirsIndex];
                    m++; // one more directory found
                }
            }
        }
        // checking if we got everything (if we went through every token)
        if (m == i)
        {
            return globalTemp; // holds fdDir of absolute path's directory
        }
    }
    // relative path (no parameter)
    else
    {
        // just concatenate to current path
        if (strlen(name) > 1)
        {
            strcat(currentWorkingDirectory, tempPath); // where do we add extra /
        }
        return curWorkingDirPtr;
    }
}

/***** READ DIRECTORY *****/
// associated command: ls

//makes an fs_diriteminfo structure for the directory we want to ls
struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{
    // if the fdDir is a valid directory (not past highest index and self of directory's deID is valid)
    if ((dirp->deArrayIndex <= dirp->highestArrayIndex) && (dirp->deArray[dirp->deArrayIndex].deID != -1))
    {
        // create struct and copy info from fdDir
        fs_diriteminfo *temp = (fs_diriteminfo *)malloc(sizeof(fs_diriteminfo));
        temp->d_reclen = dirp->d_reclen;
        temp->fileType = dirp->deArray[dirp->deArrayIndex].fileType;
        strcpy(temp->d_name, dirp->deArray[dirp->deArrayIndex].filename);
        dirp->deArrayIndex++; // increase our array index so we know when to stop returning
                              //from readdir in fsshell
        return temp;
    }
    // we want to skip any directories in the middle of the DE array that we may have deleted, 
    // but don't want to return NULL because then it'll skip any valid DEs that may come after it
    else if ((dirp->deArray[dirp->deArrayIndex].deID == -1))
    {
        fs_diriteminfo *temp = (fs_diriteminfo *)malloc(sizeof(fs_diriteminfo));
        temp->d_reclen = 0;
        temp->fileType = FT_DIRECTORY;
        strcpy(temp->d_name, "."); //doing this so it ignores the . files without
                                   //returning null and stopping the loop
        dirp->deArrayIndex++;
        return temp;
    }
    return NULL;
}

/***** CLOSE DIRECTORY *****/
//associated command: ls

//close by resetting the DE array index back to 0 so we can ls the same directory again
int fs_closedir(fdDir *dirp)
{
    dirp->deArrayIndex = 0;
}

/***** GET CWD *****/
// associated command: pwd
// also used in ls command if no pathname is specified
char *fs_getcwd(char *buf, size_t size)
{
    // if buffer has enough space
    if (strlen(currentWorkingDirectory) < size)
    {
        // copy contents of currentWorkingDirectory to buffer
        strcpy(buf, currentWorkingDirectory);
        return buf;
    }
    return "CWD Error. Not enough space for CWD in buffer.";
}

/***** SET CWD *****/
//associated command: cd

// sets cwd to an absolute or relative path
int fs_setcwd(char *buf)
{
    int rv = -1; // return value -1 unless we find the directory
    char *tempPath = malloc(256);
    strcpy(tempPath, buf);

    // if absolute path
    if (tempPath[0] == '/' && strlen(buf) > 1)
    {
        // copy buffer to cwd string
        strcpy(currentWorkingDirectory, buf);
        // tokenize absolute path
        char *dirNames[50];
        char *token = strtok(buf, "/");
        int i = 0;
        while (token != NULL)
        {
            dirNames[i] = malloc(50);
            dirNames[i] = token;
            i++;
            token = strtok(NULL, "/");
        }

        // set global temp to root array
        globalTemp = allDirs[0];

        int m = 0;
        // parse through tokens and compare with DE's

        // for every token in dirNames array
        for (int j = 0; j < i; j++)
        {
            // for every DE in directory
            for (int k = 0; k < NUM_DIR_ENTR; k++)
            {
                // if found correct DE
                if (strcmp(globalTemp->deArray[k].filename, dirNames[j]) == 0)
                {
                    // set temp equal to correct directory
                    globalTemp = allDirs[globalTemp->deArray[k].allDirsIndex];
                    m++;
                }
            }
        }
        // checking if we got everything
        if (m == i)
        {
            curWorkingDirPtr = globalTemp;
            rv = 0;
        }
    }
    // hard-code cd /
    else if(strcmp(tempPath, "/") == 0){
        strcpy(currentWorkingDirectory, "/");
        rv = 0;
        // set temp equal to directory
        curWorkingDirPtr = allDirs[0];
    }
    // relative path
    else
    {
        // loop through directory
        for (int i = 0; i <= curWorkingDirPtr->highestArrayIndex; i++)
        {
            // compare current open directory filename and filetype
            if (strcmp(buf, curWorkingDirPtr->deArray[i].filename) == 0 && curWorkingDirPtr->deArray[i].fileType == FT_DIRECTORY && curWorkingDirPtr->deArray[i].deID != -1) //delete if decide to wipe out data upon rm
            {
                // if not '.' or '..'
                if (i != 0 && i != 1)
                {
                    // if cwd doesn't end with a slash
                    if (currentWorkingDirectory[strlen(currentWorkingDirectory) - 1] != '/')
                    {
                        // add a slash
                        strcat(currentWorkingDirectory, "/");
                    }
                    // add to current path
                    strcat(currentWorkingDirectory, buf);
                }
                // if we're going into the parent (cd ..)
                else if (i == 1)
                {
                    strcpy(currentWorkingDirectory, truncateCWD()); // truncate the cwd
                }
                rv = 0;
                // get index of directory in allDirs
                int index = curWorkingDirPtr->deArray[i].allDirsIndex;
                // set temp equal to directory
                curWorkingDirPtr = allDirs[index];

                break;
            }
        }
    }
    return rv;
}

/***** IS FILE *****/

// check if directory entry is file or not
int fs_isFile(char *path)
{
    // cycle through directory (cwd)
    for (int i = 0; i < NUM_DIR_ENTR; i++)
    {
        // if found correct directory entry
        if (strcmp(curWorkingDirPtr->deArray[i].filename, path) == 0)
        {
            
            // chech if file
            if (curWorkingDirPtr->deArray[i].fileType == FT_REGFILE)
            {
                // return 1 (true)
                return 1;
            }
        }
    }
    // return 0 (false) if directory entry
    // not found or not file
    return 0;
}

/***** IS DIRECTORY *****/
// check if directory or not
int fs_isDir(char *path)
{
    // cycle through directory (cwd)
    for (int i = 0; i < NUM_DIR_ENTR; i++)
    {
        // if found correct directory entry
        if (strcmp(curWorkingDirPtr->deArray[i].filename, path) == 0)
        {
            // chech if file
            if (curWorkingDirPtr->deArray[i].fileType == FT_DIRECTORY)
            {
                // return 1 (true)
                return 1;
            }
        }
    }
    // return 0 (false) if directory entry
    // not found or not directory
    return 0;
}

/***** DELETE FILE *****/
// associated command: rm

// removes a file from the file system
int fs_delete(char *filename)
{
    for (int i = 0; i <= curWorkingDirPtr->highestArrayIndex; i++)
    {
        // compare current open directory filename and filetype
        if (strcmp(filename, curWorkingDirPtr->deArray[i].filename) == 0 && curWorkingDirPtr->deArray[i].fileType == FT_REGFILE)
        {
            globalTemp = allDirs[curWorkingDirPtr->deArray[i].allDirsIndex];

            //flip bits in parent's bit array
            flipBits(&curWorkingDirPtr->dirBitMap, i, 1);

            //subtract one from parent's d_reclen to avoid readdir seg fault
            curWorkingDirPtr->d_reclen--;

            //set to -1 to show that it's been deleted
            curWorkingDirPtr->deArray[i].deID = -1;
            strcpy(curWorkingDirPtr->deArray[i].filename, "");

            //show data blocks as available in bitmap
            FileFD fd = findFileDataBlocks(filename);
            for(int i = 0; i < fd.totalDataBlocks; i++){
                flipBits(dataBitMap, fd.fileBlockIDs[i], 1);
            }
        }
    }
    return 0;
}

/***** STAT *****/

// fills fs_stat buf with information about directory
int fs_stat(const char *path, struct fs_stat *buf)
{
    //path is just the directory's name, so search for it in allDirs
    int i = 0;
    while (i < 0)
    {
        if (strcmp(allDirs[i]->dirName, path) == 0)
        {
            break;
        }
        else
        {
            i++;
        }
    }
    //fill the struct fs_stat buf with info we have about the directory
    buf->st_size = allDirs[i]->d_reclen * sizeof(DirectoryEntry);
    buf->st_blksize = blockSize;

    //might change to j < NUM_DIR_ENTRS bc i dont think we'll shift the array
    //if we delete something later on.
    //leave it to this until we make sure to init whole array
    for (int j = 0; j <= allDirs[i]->highestArrayIndex; j++)
    {
        buf->st_blocks += allDirs[i]->deArray[j].numBlocks;
    }

    // NOT IMPLEMENTED:
    buf->st_accesstime = 0; /* time of last access */
    buf->st_modtime = 0;    /* time of last modification */
    buf->st_createtime = 0; /* time of last status change */
}

/****** MOVE FILE ******/
// associated command: mv

// moves a file (fromName) to a relative directory (called toName)
// or renames a file (fromName) to the name toName if a file by the name of toName doesn't exist
int fs_mvFile(char* fromName, char* toName){
    //if toName is a directory name, mv to dir
    if( fs_isDir(toName) ){
        
        //change fromName's DE info 
        // find fromName in currentDirectory
        for (int i = 0; i <= curWorkingDirPtr->highestArrayIndex; i++)
        {
            // compare current open directory filename and filetype
            if ( (strcmp(fromName, curWorkingDirPtr->deArray[i].filename) == 0) && (curWorkingDirPtr->deArray[i].fileType == FT_REGFILE) )
            {
                //find the toName directory
                DirectoryEntry fileDE;
                DirectoryEntry toDir;

                //copy all attributes over to new DE
                fileDE.deID = curWorkingDirPtr->deArray[i].deID;
                fileDE.blockID = curWorkingDirPtr->deArray[i].blockID;
                fileDE.numBlocks = curWorkingDirPtr->deArray[i].numBlocks;
                strcpy(fileDE.filename, curWorkingDirPtr->deArray[i].filename);
                fileDE.fileSize = curWorkingDirPtr->deArray[i].fileSize;
                fileDE.fileType = curWorkingDirPtr->deArray[i].fileType;
                fileDE.allDirsIndex = curWorkingDirPtr->deArray[i].allDirsIndex;

                //this isnt gonna work for paths, must be in same directory
                // find the directory in our current directory with the name toName
                for(int j = 0; j < 20; j++){
                    if( strcmp(curWorkingDirPtr->deArray[j].filename, toName) == 0 && curWorkingDirPtr->deArray[j].fileType == FT_DIRECTORY){
                        toDir = curWorkingDirPtr->deArray[j];
                    }
                }

                //remove file from parent's dearray 
                curWorkingDirPtr->deArray[i].deID = -1;
                // flip bit in old parent's bitmap
                flipBits(curWorkingDirPtr->dirBitMap, fileDE.deID, 1);
                // update fileDE's deID by finding new space in new directory
                fileDE.deID = findNextFreeIndex(allDirs[toDir.allDirsIndex]->dirBitMap);
                // add to parent's dearray, update fdDIr vars
                allDirs[toDir.allDirsIndex]->deArray[fileDE.deID] = fileDE;
                allDirs[toDir.allDirsIndex]->d_reclen++;

                // update highest index if fileDE is the new highest index
                if(fileDE.deID > allDirs[toDir.allDirsIndex]->highestArrayIndex){
                    allDirs[toDir.allDirsIndex]->highestArrayIndex = fileDE.deID;
                }
                // flip bit in new parent's bitmap
                flipBits(allDirs[toDir.allDirsIndex]->dirBitMap, fileDE.deID, 1);
                return 0;
            }
        }
    }
    // if file of name toName already exists, error
    else if(fs_isFile(toName)){
        printf("Cannot rename, file already exists.\n");
        return -1;
    }
    // rename file to toName
    else{
        for (int i = 0; i <= curWorkingDirPtr->highestArrayIndex; i++){
            if ( (strcmp(fromName, curWorkingDirPtr->deArray[i].filename) == 0) && (curWorkingDirPtr->deArray[i].fileType == FT_REGFILE) ){
                strcpy(curWorkingDirPtr->deArray[i].filename, toName);
            }
        }
        return 0;
    }
    return -1;
}

/***** MOVE DIRECTORY *****/
// associated command: mv

// moves an empty directory (fromName) to a relative directory (called toName)
int fs_mvDir(char* fromName, char* toName){
    if(fs_isDir(toName)){
        for (int i = 0; i <= curWorkingDirPtr->highestArrayIndex; i++)
        {
            // compare current open directory filename and filetype
            if (strcmp(fromName, curWorkingDirPtr->deArray[i].filename) == 0 && curWorkingDirPtr->deArray[i].fileType == FT_DIRECTORY)
            {
                DirectoryEntry dirDE;
                fdDir* fd;
                DirectoryEntry toDir;

                // copy the directory's info into a new struct
                dirDE.deID = curWorkingDirPtr->deArray[i].deID;
                dirDE.blockID = curWorkingDirPtr->deArray[i].blockID;
                dirDE.numBlocks = curWorkingDirPtr->deArray[i].numBlocks;
                strcpy(dirDE.filename, curWorkingDirPtr->deArray[i].filename);
                dirDE.fileType = curWorkingDirPtr->deArray[i].fileType;
                dirDE.fileSize = curWorkingDirPtr->deArray[i].fileSize;
                dirDE.allDirsIndex = curWorkingDirPtr->deArray[i].allDirsIndex;
                
                fd = allDirs[dirDE.allDirsIndex]; // holds fd of the directory we want to move
                if (fd->d_reclen <= 2)
                {
                    //dir is empty and we can move
                    // find the de of directory to go to
                    for(int j = 0; j < 20; j++){
                        if( strcmp(curWorkingDirPtr->deArray[j].filename, toName) == 0 && curWorkingDirPtr->deArray[j].fileType == FT_DIRECTORY){
                            toDir = curWorkingDirPtr->deArray[j]; // set the directory we want to move to to toDir
                        }
                    }

                    //change deArrayIndex 
             
                    //remove from parent's dearray 
                    curWorkingDirPtr->deArray[i].deID = -1;
                    // flip bit in old parent's bitmap
                    flipBits(curWorkingDirPtr->dirBitMap, dirDE.deID, 1);

                    // update fromName's DE attrs
                    dirDE.deID = findNextFreeIndex(allDirs[toDir.allDirsIndex]->dirBitMap);
                    
                    // add to parent's dearray, update fdDir vars
                    allDirs[toDir.allDirsIndex]->deArray[dirDE.deID] = dirDE;
                    allDirs[toDir.allDirsIndex]->d_reclen++;

                    // update highest index if fileDE is the new highest index
                    if(dirDE.deID > allDirs[toDir.allDirsIndex]->highestArrayIndex){
                        allDirs[toDir.allDirsIndex]->highestArrayIndex = dirDE.deID;
                    }

                    // flip bit in new parent's bitmap
                    flipBits(allDirs[toDir.allDirsIndex]->dirBitMap, dirDE.deID, 1);
                    return 0;
                }
                else
                {
                    printf("You can only remove an empty directory.\n");
                    return -1;
                }
            }
        }
    }
    else{
        printf("This directory does not exist in the current working directory.\n");
        return 0;
    }
}

/***** HELPER FUNCTIONS *****/

// initializes a directory's DE array to neutral values so it's ready for use
// real intialization for each DE comes in when we call initDirectoryEntry
void initDEArray(DirectoryEntry *DEarray[])
{
    //basic init is in initDirectoryEntry
    for (int i = 0; i < NUM_DIR_ENTR; i++)
    {
        DEarray[i] = (DirectoryEntry *)malloc(sizeof(DirectoryEntry));
        DEarray[i]->deID = i;
        DEarray[i]->numBlocks = -1;
        strcpy(DEarray[i]->filename, "");
        DEarray[i]->fileType = -1;
        DEarray[i]->fileSize = -1;
    }
}

// init allDirs' bit map
void initAllDirsBitMap(int bitMap[])
{
    for (int i = 0; i < 20; i++)
    {
        bitMap[i] = 0;
    }
}

// gets next index in the allDirs array
int getNextAllDirs()
{
    for (int i = 0; i < 20; i++)
    {
        if (allDirsBitMap[i] == 0)
        {
            return i;
        }
    }
    printf("Couldn't find space in all dirs bit map\n");
    return -1;
}

// gets next index in a directory's DE array
int getNextDirIndex(fdDir *directory)
{
    for (int i = 0; i < NUM_DIR_ENTR; i++)
    {
        if (directory->dirBitMap[i] == 0)
        {
            return i;
        }
    }
}

// initialize an fdDir struct to starting values
void initFdDir(fdDir *fd, char *dirname)
{
    fd->d_reclen = 2;
    fd->dirEntryPosition = 0;
    fd->directoryStartLocation = findFreeDataBlocks(1);
    initDEArray(fd->deArray);
    initDirBitMap(fd->dirBitMap);
    fd->deArrayIndex = 0;
    fd->dirName = dirname;
}

// truncates the cwd when we do cd ..
char *truncateCWD()
{
    char *truncatedCWD = malloc(256);
    int lastSlashIndex = 0; // keep track of the last slash we saw

    // go through each character in the cwd string
    for (int i = 0; i < strlen(currentWorkingDirectory); i++)
    {
        if (currentWorkingDirectory[i] == '/')
        {
            lastSlashIndex = i; // update everytime we see a slash
        }
    }

    // copy cwd into truncateCWD up until the last slash we saw
    strncpy(truncatedCWD, currentWorkingDirectory, lastSlashIndex);

    // if only slash is the one at the beginning
    if (lastSlashIndex == 0)
    {
        strcpy(truncatedCWD, "/"); // cwd is just  /
    }
    return truncatedCWD;
}

// find all of the data blocks that belong to a file
FileFD findFileDataBlocks(char* filename){
    dataBlock* temp = HEAD;

    //just init the FileFD in here
    FileFD fd;
    strcpy(fd.filename, filename);
    fd.totalDataBlocks = 0;

    // go through all data blocks
    while(temp->next != NULL){
        // if the data block is associated with the file with name filename
        if( strcmp(filename, temp->fileName) == 0){
            // add blockID to our array of blockIDs
            fd.fileBlockIDs[fd.totalDataBlocks] = temp->blockID; 
            fd.totalDataBlocks++; // increment total number of blocks
        }
        temp = temp->next;
    }
    
    return fd; // return the FileFD
}
