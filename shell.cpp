/*
 * shell.C -- CEG433 File Sys Project shell
 * pmateti@wright.edu
 *
 * Modified for Project 2 of CEG-4350
 * by Adrien Abbey, Sept. 2023
 */

#include "fs33types.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <bits/stdc++.h>

extern MountEntry *mtab;
extern VNIN cwdVNIN;
FileVolume *fv; // Suspicious!
Directory *wd;  // Suspicious!

#define nArgsMax 10
char types[1 + nArgsMax]; // +1 for \0

/* An Arg-ument for one of our commands is either a "word" (a null
 * terminated string), or an unsigned integer.  We store both
 * representations of the argument. */

class Arg
{
public:
  char *s;
  uint u;
} arg[nArgsMax];

uint nArgs = 0;

uint TODO()
{
  printf("to be done!\n");
  return 0;
}

uint TODO(char *p)
{
  printf("%s to be done!\n", p);
  return 0;
}

uint isDigit(char c)
{
  return '0' <= c && c <= '9';
}

uint isAlphaNumDot(char c)
{
  return c == '.' || 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || '0' <= c && c <= '9';
}

int toNum(const char *p)
{
  return (p != 0 && '0' <= *p && *p <= '9' ? atoi(p) : 0);
}

/// @brief Attempts to load a simulated disk with the given name, or creates a
/// new one if it does not already exist.  The given name must be defined in
// `diskParams.dat`, which defines the disk's attributes.
/// @param name The name the simulated disk.  If the `name` is defined in
/// `diskparams.dat`, a new simulated disk using the parameters from that file
/// is created.
/// @return Returns a pointer to the new SimDisk object, if successful.
/// Otherwise, returns a `0` pointer.
SimDisk *mkSimDisk(byte *name)
{
  SimDisk *simDisk = new SimDisk(name, 0);
  if (simDisk->nSectorsPerDisk == 0)
  {
    printf("Failed to find/create simDisk named %s\n", name);
    delete simDisk;
    simDisk = 0;
  }
  return simDisk;
}

void doMakeDisk(Arg *a)
{
  SimDisk *simDisk = mkSimDisk((byte *)a[0].s);
  if (simDisk == 0)
    return;
  printf("new SimDisk(%s) = %p, nSectorsPerDisk=%d,"
         "nBytesPerSector=%d, simDiskNum=%d)\n",
         simDisk->name, (void *)simDisk, simDisk->nSectorsPerDisk,
         simDisk->nBytesPerSector, simDisk->simDiskNum);
  delete simDisk;
}

/// @brief Writes a string to the specified sector of the current local disk.
/// The string is repeated to fill the disk sector completely.
/// @param a `a[0]` is the disk name to be used.  If it already exists, it's
/// loaded.  If not, it's created. See `mkSimDisk` for details.
/// `a[1]` is the sector number of the current local disk to be written to.
/// `a[2]` is the string to be written to the specified sector.  If no string
/// is specified, "CEG433/633/Mateti" will be used instead.
void doWriteDisk(Arg *a)
{
  SimDisk *simDisk = mkSimDisk((byte *)a[0].s);
  if (simDisk == 0)
    return;
  char *st = a[2].s; // arbitrary word
  if (st == 0)       // if it is NULL, we use ...
    st = "CEG433/633/Mateti";
  char buf[1024]; // assuming nBytesPerSectorMAX < 1024
  for (uint m = strlen(st), n = 0; n < 1024 - m; n += m)
    memcpy(buf + n, st, m); // fill with several copies of st
  uint r = simDisk->writeSector(a[1].u, (byte *)buf);
  printf("write433disk(%d, %s...) == %d to Disk %s\n", a[1].u, st, r, a[0].s);
  delete simDisk;
}

void doReadDisk(Arg *a)
{
  SimDisk *simDisk = mkSimDisk((byte *)a[0].s);
  if (simDisk == 0)
    return;
  char buf[1024]; // assuming nBytesPerSectorMAX < 1024
  uint r = simDisk->readSector(a[1].u, (byte *)buf);
  buf[10] = 0; // sentinel
  printf("read433disk(%d, %s...) = %d from Disk %s\n", a[1].u, buf, r, a[0].s);
  delete simDisk;
}

void doQuit(Arg *a)
{
  exit(0);
}

void doEcho(Arg *a)
{
  printf("%s#%d, %s#%d, %s#%d, %s#%d\n", a[0].s, a[0].u,
         a[1].s, a[1].u, a[2].s, a[2].u, a[3].s, a[3].u);
}

/// @brief Creates an empty file volume on the simulated disk `a`.dsk.  The
/// name must match one of the parameters listed in `diskparams.dat`.
/// @param a The name of the simulated disk being created.  This must be
/// already defined in the `diskparams.dat` file.
void doMakeFV(Arg *a)
{
  SimDisk *simDisk = mkSimDisk((byte *)a[0].s);
  if (simDisk == 0)
    return;
  fv = simDisk->make33fv();
  printf("make33fv() = %p, Name == %s, Disk# == %d\n",
         (void *)fv, a[0].s, simDisk->simDiskNum);

  if (fv)
  {
    wd = new Directory(fv, 1, 0);
    cwdVNIN = mkVNIN(simDisk->simDiskNum, 1);
  }
}

void doCopyTo(byte *from, byte *to)
{
  uint r = fv->write33file(to, from);
  printf("write33file(%s, %s) == %d\n", to, from, r);
}

void doCopyFrom(byte *from, byte *to)
{
  uint r = fv->read33file(to, from);
  printf("read33file(%s, %s) == %d\n", to, from, r);
}

void doCopy33(byte *from, byte *to)
{
  uint r = fv->copy33file(to, from);
  printf("copy33file(%s, %s) == %d\n", to, from, r);
}

/// @brief Copies a file from the given source file to the destination file
/// name given.  At most one of these files can be prepended with `\@`,
/// denoting a host system file.
/// @param a The arguments given.  These arguments should be a `source file`
/// and `destination`.  At most one of these arguments can start with `\@`,
/// which denotes a host system source file or destination.
void doCopy(Arg *a)
{
  // TODO: Also work with subdirs?

  byte *to = (byte *)a[0].s;
  byte *from = (byte *)a[1].s;

  if (a[0].s[0] == '@' && a[1].s[0] != '@')
  {
    doCopyTo(from, (to + 1));
  }
  else if (a[0].s[0] != '@' && a[1].s[0] == '@')
  {
    doCopyFrom((from + 1), to);
  }
  else if (a[0].s[0] != '@' && a[1].s[0] != '@')
  {
    doCopy33(from, to);
  }
  else
  {
    puts("Wrong arguments to cp.");
  }
}

/// @brief Attempts to find the file with the given path.
/// @param path Relative or absolute path, including the file name.
/// @return Returns the inode of the directory, if found.
/// Returns 0 if nothing found.
uint findFile(char *path)
{
  // Create a working directory that can change without mangling wd:
  Directory *workingDirectory;

  // Check if relative or absolute path:
  if (path[0] == '/')
  {
    // Absolute path.  Find the directory.
    // Start at the root directory:
    workingDirectory = new Directory(fv, fv->root->nInode, fv->root->nInode);
  }
  else
  {
    // Relative path.  Find the directory.
    // Start in the current working directory:
    workingDirectory = new Directory(fv, wd->nInode, fv->root->nInode);
  }

  // Start splitting the path into usable parts:
  char *pathPart = strtok(path, "/");

  // Search through each path part, looking for valid directories:
  while (pathPart != NULL)
  {
    // Check if the next path part exists:
    uint nextInode = workingDirectory->iNumberOf((byte *)pathPart);
    if (nextInode != 0)
    {
      // If the next path part is a directory:
      if (fv->inodes.getType(nextInode) == 2)
      {
        // Directory exists, switch to it:
        workingDirectory = new Directory(fv, nextInode, workingDirectory->iNumberOf((byte *)".."));
      }
      else
      {
        // It's a file.
        return nextInode;
      }

      // Move to the next path part:
      pathPart = strtok(NULL, "/");
    }
    else
    {
      // The path was not found, invalid path, abort:
      return 0;
    }
  }

  // Return the inode of the directory found:
  return workingDirectory->nInode;
}

/// @brief Print a listing of the current local directory's contents, much like
/// `ls -lisa` would.
/// @param a Arguments, if any.  These are likely ignored.
void doLsLong(Arg *a)
{
  // If an argument is given, treat it as a directory path to ls:
  if (a[0].s != NULL)
  {
    // Confirm the path is valid:
    uint pathInode = findFile(a[0].s);
    // NOTE: Also check to confirm that the given inode is a directory!
    if (pathInode > 0 && fv->inodes.getType(pathInode) == 2)
    {
      // Valid path.  Make it so:
      Directory *pathDir = new Directory(fv, findFile(a[0].s), fv->root->nInode);
      // Safe to specify the parent as root, as a parent dir should already exist
      // and won't be overwritten.

      printf("\nDirectory listing for disk %s, cwdVNIN == 0x%0lx begins:\n",
             wd->fv->simDisk->name, (ulong)cwdVNIN);
      pathDir->ls(); // Not suspicious at all anymore!
      printf("Directory listing ends.\n");
    }
    else
    {
      // Invalid path, complain loudly:
      printf("%s", "Invalid path.\n");
    }
  }
  else
  {
    // If no directory path specified, use the current working directory:
    printf("\nDirectory listing for disk %s, cwdVNIN == 0x%0lx begins:\n",
           wd->fv->simDisk->name, (ulong)cwdVNIN);
    wd->ls(); // Suspicious!
    printf("Directory listing ends.\n");
  }
}

/// @brief Deletes the given file from the current local directory.
/// TODO: Handle local folders like `rmdir`?
/// @param a The file to be deleted from the current local directory.
void doRm(Arg *a)
{
  // NOTE: This is functional, but only works for files/directories that exist
  // within the root directory.  I'm modifying it to also work with non-root
  // files/directories.

  uint fileCount;

  // Check if the given file is a directory:
  if (fv->inodes.getType(wd->iNumberOf((byte *)a[0].s)) == 2)
  {
    // Check if the directory has any contents:
    Directory *currentDir = new Directory(fv, wd->iNumberOf((byte *)a[0].s), 1);

    // NOTE: ls returns the number of files in a given directory!
    // Unfortunately, it also prints out stuff.  I could make lsPrivate
    // public, but I have a  feeling that'd be bad.
    // Solution: redirect stdout briefly to suppress that printing.

    // Redirect stdout to a temporary file:
    int backupStdout = dup(STDOUT_FILENO);
    int tempOut = open("/dev/null", O_WRONLY); // /dev/null exists to consume
    dup2(tempOut, STDOUT_FILENO);

    // Get the number of files in the given folder:
    fileCount = currentDir->ls();

    // If there are files in the folder:
    if (fileCount > 0)
    {
      // Restore stdin and return:
      dup2(backupStdout, STDOUT_FILENO);
      close(backupStdout);

      // Abort:
      printf("%s%d%s\n", "Directory not empty: contains ", fileCount, " files.");

      return;
    }

    // Restore stdout:
    dup2(backupStdout, STDOUT_FILENO);
    close(backupStdout);

    // Delete the folder:
    printf("%s%d%s\n", "Deleting empty directory: contains ", fileCount, " files in it.");
    wd->deleteFile((byte *)a[0].s, 1);
  }
  else
  {
    // Delete the given file in the current working directory:
    uint in = wd->deleteFile((byte *)a[0].s, 1);
    printf("rm %s returns %d.\n", a[0].s, in);
  }
}

/// @brief Prints out the contents of the given inode number, if any.
/// @param a The inode number to be printed.
void doInode(Arg *a)
{
  uint ni = a[0].u;

  wd->fv->inodes.show(ni);
}

void doInodeStr(Arg *a)
{
  // Find the given file:
  char *filePath = (char *)a[0].s;
  uint fileInode = findFile(filePath);

  // Print the inode, if it exists:
  if (fileInode > 0)
  {
    fv->inodes.show(fileInode);
  }
  else
  {
    printf("%s\n", "Invalid path.");
  }
}

/// @brief Create a directory with the given name in the current local
/// directory, then print the new directory's inode number.  If a directory of
/// the same name already exists in the current local directory, create
/// nothing and return `0`.  Note that this includes modifications to inodes
/// to indicate whether its a directory or not.  Assume that the given
/// argument does NOT contain slashes.
/// @param a The name of the directory to be created.
void doMkDir(Arg *a)
{
  // Check to see if the directory already exists:
  uint existingDir = wd->iNumberOf((byte *)a[0].s);
  if (existingDir != 0)
  {
    printf("The directory already exists.\n");
    return; // The directory already exists, do nothing.
  }

  // Create a new directory (file):
  uint newDir = wd->createFile((byte *)a[0].s, 1);

  // Print the inode of the new directory:
  printf("The new directory inode is: %d\n", newDir);
}

/// @brief Generates the path string for the given directory.  It does this by
/// recursively calling itself on each parent directory.
/// @param dir Directory for which to find the full path string of.
/// @return Returns a string containing the full path for the given directory.
std::string getPwdString(Directory *dir)
{
  // If this is the root node:
  if (dir->nInode == 1)
  {
    // No parent directory, return the root path:
    return "/";
  }
  else
  {
    // Get the path string of this directory's parent:
    Directory *parentDir = new Directory(fv, dir->iNumberOf((byte *)".."), dir->iNumberOf((byte *)".."));
    std::string parentPath = getPwdString(parentDir);

    // Get the name of the current directory:
    byte *dirNameByte = parentDir->nameOf(dir->nInode);
    std::string dirNameStr = (char *)dirNameByte;

    // Prepend the parent path to the directory name:
    std::string returnString = parentPath;
    returnString.append(dirNameStr);
    returnString.append("/");

    return returnString;
  }
}

/// @brief Change the current directory to the given directory or path.  If
/// the argument begins with a slash, it's an absolute name.  If it does not,
/// it's relative to the current working directory.  In either case, print the
/// absolute path name of the new current working directory.
/// @param a The directory to change to, either as a relative path or as an
/// absolute path.
void doChDir(Arg *a)
{
  // Find the path given:
  char *pathArg = a[0].s;
  uint pathInode = findFile(pathArg);

  // If the path exists and is a directory:
  if (pathInode > 0 && fv->inodes.getType(pathInode) == 2)
  {
    // Change the working directory:
    wd = new Directory(fv, pathInode, fv->root->nInode);

    // Print the new working directory:
    printf("%s\n", getPwdString(wd).c_str());
  }
  else
  {
    // Invalid path.  Complain loudly:
    printf("%s", "Invalid path.\n");
  }
}

/// @brief Prints out the path string of the current working directory.
/// @param a Not used!
void doPwd(Arg *a)
{
  // Call the recursive function on the current working directory to build the string:
  std::string pathStr = getPwdString(wd);

  // Print out the current working directory path:
  printf("%s\n", pathStr.c_str());
}

/// @brief Move a file or directory to another location.
/// If the first argument does not exist, do nothing.
/// If the second argument does not exist in the current directory, the first
/// argument is renamed to the second argument.
/// If the second argument is an existing directory within the current
/// directory, the first argument is moved (along with all its contents) into
/// the second directory.
/// If the second argument is an existing but ordinary file, do nothing.
/// @param a Must have two arguments: the first is the file or directory being
/// moved, with the second being the destination file or directory.
void doMv(Arg *a)
{
  // Ensure the first argument exists:
  uint mvFromInode = wd->iNumberOf((byte *)a[0].s);
  if (mvFromInode == 0)
  {
    printf("%s\n", "File or folder not found.");
    return;
  }

  // Check if the second argument already exists:
  uint mvToInode = wd->iNumberOf((byte *)a[1].s);
  if (mvToInode > 0)
  {
    // Destination exists.
    // Check if it's a file or folder:
    if (fv->inodes.getType(mvToInode) == 2)
    {
      // Destination is a folder.
      Directory *destinationFolder = new Directory(fv, mvToInode, 1);

      // Verify that the destination folder does NOT already have a file of the
      // same name:
      if (destinationFolder->iNumberOf((byte *)a[0].s) > 0)
      {
        // The destination file already exists, abort
        printf("%s\n", "Destination file already exists.");
        return;
      }

      // Move the source file/folder to the destination folder:
      destinationFolder->moveFile(wd->nInode, (byte *)a[0].s);
    }
    else
    {
      // Destination is an existing file.  Abort:
      printf("%s\n", "Destination file already exists.");
      return;
    }
  }
  else
  {
    // Destination does not exist.
    // Rename the source file/folder to the destination file/folder:
    wd->deleteFile((byte *)a[0].s, 0);
    wd->addLeafName((byte *)a[1].s, mvFromInode);
  }
}

void doMountDF(Arg *a) // arg a ignored
{
  TODO("doMountDF");
}

void doMountUS(Arg *a)
{
  TODO("doMountUS");
}

void doUmount(Arg *a)
{
  TODO("doUmount");
}

/* The following describes one entry in our table of commands.  For
 * each cmmdName (a null terminated string), we specify the arguments
 * it requires by a sequence of letters.  The letter s stands for
 * "that argument should be a string", the letter u stands for "that
 * argument should be an unsigned int."  The data member (func) is a
 * pointer to the function in our code that implements that command.
 * globalsNeeded identifies whether we need a volume ("v"), a simdisk
 * ("d"), or a mount table ("m").  See invokeCmd() below for exact
 * details of how all these flags are interpreted.
 */

class CmdTable
{
public:
  char *cmdName;
  char *argsRequired;
  char *globalsNeeded; // need d==simDisk, v==cfv, m=mtab
  void (*func)(Arg *a);
} cmdTable[] = {
    {"cd", "s", "v", doChDir},
    {"cp", "ss", "v", doCopy},
    {"echo", "ssss", "", doEcho},
    {"inode", "u", "v", doInode},
    {"inode", "s", "v", doInodeStr},
    {"ls", "", "v", doLsLong},
    {"ls", "s", "v", doLsLong}, // Allow ls to specify a path
    {"lslong", "", "v", doLsLong},
    {"mkdir", "s", "v", doMkDir},
    {"mkdisk", "s", "", doMakeDisk},
    {"mkfs", "s", "", doMakeFV},
    {"mount", "us", "", doMountUS},
    {"mount", "", "", doMountDF},
    {"mv", "ss", "v", doMv},
    {"rddisk", "su", "", doReadDisk},
    {"rmdir", "s", "v", doRm},
    {"rm", "s", "v", doRm},
    {"pwd", "", "v", doPwd},
    {"q", "", "", doQuit},
    {"quit", "", "", doQuit},
    {"umount", "u", "m", doUmount},
    {"wrdisk", "sus", "", doWriteDisk}};

uint ncmds = sizeof(cmdTable) / sizeof(CmdTable);

void usage()
{
  printf("The shell has only the following cmds:\n");
  for (uint i = 0; i < ncmds; i++)
    printf("\t%s\t%s\n", cmdTable[i].cmdName, cmdTable[i].argsRequired);
  printf("Start with ! to invoke a Unix shell cmd\n");
}

/* pre:: k >= 0, arg[] are set already;; post:: Check that args are
 * ok, and the needed simDisk or cfv exists before invoking the
 * appropriate action. */

void invokeCmd(int k, Arg *arg)
{
  uint ok = 1;
  if (cmdTable[k].globalsNeeded[0] == 'v' && cwdVNIN == 0)
  {
    ok = 0;
    printf("Cmd %s needs the cfv to be != 0.\n", cmdTable[k].cmdName);
  }
  else if (cmdTable[k].globalsNeeded[0] == 'm' && mtab == 0)
  {
    ok = 0;
    printf("Cmd %s needs the mtab to be != 0.\n", cmdTable[k].cmdName);
  }

  char *req = cmdTable[k].argsRequired;
  uint na = strlen(req);
  for (uint i = 0; i < na; i++)
  {
    if (req[i] == 's' && (arg[i].s == 0 || arg[i].s[0] == 0))
    {
      ok = 0;
      printf("arg #%d must be a non-empty string.\n", i);
    }
    if ((req[i] == 'u') && (arg[i].s == 0 || !isDigit(arg[i].s[0])))
    {
      ok = 0;
      printf("arg #%d (%s) must be a number.\n", i, arg[i].s);
    }
  }
  if (ok)
    (*cmdTable[k].func)(arg);
}

/* pre:: buf[] is the command line as typed by the user, nMax + 1 ==
 * sizeof(types);; post:: Parse the line, and set types[], arg[].s and
 * arg[].u fields.
 */

void setArgsGiven(char *buf, Arg *arg, char *types, uint nMax)
{
  for (uint i = 0; i < nMax; i++)
  {
    arg[i].s = 0;
    types[i] = 0;
  }
  types[nMax] = 0;

  strtok(buf, " \t\n"); // terminates the cmd name with a \0

  for (uint i = 0; i < nMax;)
  {
    char *q = strtok(0, " \t");
    if (q == 0 || *q == 0)
      break;
    arg[i].s = q;
    arg[i].u = toNum(q);
    types[i] = isDigit(*q) ? 'u' : 's';
    nArgs = ++i;
  }
}

/* pre:: name pts to the command token, argtypes[] is a string of
 * 's'/'u' indicating the types of arguments the user gave;; post::
 * Find the row number of the (possibly overloaded) cmd given in
 * name[].  Return this number if found; return -1 otherwise. */

int findCmd(char *name, char *argtypes)
{
  for (uint i = 0; i < ncmds; i++)
  {
    if (strcmp(name, cmdTable[i].cmdName) == 0 && strcmp(argtypes, cmdTable[i].argsRequired) == 0)
    {
      return i;
    }
  }
  return -1;
}

void ourgets(char *buf)
{
  fgets(buf, 1024, stdin);
  char *p = index(buf, '\n');
  if (p)
    *p = 0;
}

/* The following are all custom functions I wrote for Project 1. */

// Copy/paste of provided code that executes a local command, from main()
//   method to reduce repetition:
void doCommand(char *buf)
{
  setArgsGiven(buf, arg, types, nArgsMax);
  int k = findCmd(buf, types);
  if (k >= 0)
    invokeCmd(k, arg);
  else
    usage();
}

bool checkRedirect(char *str)
{
  // My method to check for a redirect character in the input string.
  // Return true if the `>` character exists in the string.

  // Since we're using character arrays, I need a way to find the size of a
  // given char array: https://stackoverflow.com/a/4180826

  for (long unsigned int i = 0; i < strlen(str); i++)
  {
    if (str[i] == '>')
    {
      return true;
    }
  }
  return false;
}

bool checkPipe(char *str)
{
  // My method to check for a pipe character in the input string.
  // Return true if the `|` character exists in the string.

  for (long unsigned int i = 0; i < strlen(str); i++)
  {
    if (str[i] == '|')
    {
      return true;
    }
  }
  return false;
}

bool checkBackground(char *str)
{
  // Check for a background character in the input string.

  for (long unsigned int i = 0; i < strlen(str); i++)
  {
    if (str[i] == '&')
    {
      return true;
    }
  }
  return false;
}

std::string getRedirectFile(char *str)
{
  // My method to extract the file name from the given string.
  // Search for the `>` character, then return a new string containing every
  // character after that: https://cplusplus.com/reference/cstring/strtok/

  // Tokenize the given string:
  std::string returnString = strtok(str, ">");

  // Do this twice to get the file name:
  returnString = strtok(NULL, ">");

  // If the first character of the string is a space, remove that:
  // https://stackoverflow.com/a/23834717
  if (returnString[0] == ' ')
  {
    returnString.erase(0, 1);
  }

  return returnString;
}

// My method for executing a redirect:
void doRedirect(char *buf)
{
  // I need to redirect `stdout` to the given file name.
  // To do this, I can use `dup2`:
  // https://www.geeksforgeeks.org/dup-dup2-linux-system-call/

  // First I need to get the filename from the string:
  std::string fileName = getRedirectFile(buf);

  // Remove the redirect from the command:
  char *redirectedCommand = strtok(buf, ">");

  // Open the file: https://www.geeksforgeeks.org/convert-string-char-array-cpp/
  // https://stackoverflow.com/a/35186153
  int fileDescriptor = open(fileName.c_str(), O_WRONLY | O_CREAT, 0644);

  // Save original stdout: https://stackoverflow.com/a/11042581
  int savedStdout = dup(STDOUT_FILENO);

  // Redirect `stdout` to the given file: https://stackoverflow.com/q/26666012
  dup2(fileDescriptor, STDOUT_FILENO);

  // Check for host commands:
  if (redirectedCommand[0] == '!')
  {
    system(redirectedCommand + 1);
  }
  else
  {
    doCommand(redirectedCommand); // Copy of original command handler from main()
  }

  // Restore `stdout`:
  dup2(savedStdout, STDOUT_FILENO);
  close(savedStdout);
}

void splitPipeString(char *buf, char *firstCmd, char *secondCmd)
{
  // Split the given string into multiple commands.
  // This is NOT for bonus points.
  // Assumes only two commands with valid formatting.

  // https://stackoverflow.com/a/49698596
  // I made the mistake of forgetting how pointers and shallow copies work.

  // Pull the first command from the string:
  std::string firstCmdTemp = strtok(buf, "|");

  // Pull the second:
  std::string secondCmdTemp = strtok(NULL, "|");

  // Note: The project instructions state: "Implement piping as discussed
  // in class for commands executed on the host."  I'm interpreting that
  // as meaning that piping will *always* only ever involve host commands
  // starting with '!'

  // Strip ' ' from the beginning of each string:
  while (firstCmdTemp[0] == ' ')
  {
    if (firstCmdTemp[0] == ' ')
    {
      firstCmdTemp.erase(0, 1);
    }
  }

  // Strip ' ' from the beginning of each string:
  while (secondCmdTemp[0] == ' ')
  {
    if (secondCmdTemp[0] == ' ')
    {
      secondCmdTemp.erase(0, 1);
    }
  }

  // Copy the temporary strings to the provided pointers for more permanent results:
  strcpy(firstCmd, firstCmdTemp.c_str());
  strcpy(secondCmd, secondCmdTemp.c_str());
}

void doPipe(char *buf)
{
  // Re-reading the instructions, this is actually a lot easier than I
  // was expecting: I only need to handle host commands!

  // For piped commands, I first need to split the input string into
  // multiple commands:

  // Prep the vars:
  char firstCmd[BUFSIZ];  // Buffersize matters, apparently:
  char secondCmd[BUFSIZ]; // https://stackoverflow.com/a/14428470

  // Split the input:
  splitPipeString(buf, firstCmd, secondCmd);

  // I'm hopelessly lost and overly dependent on others for guidance.
  // https://www.geeksforgeeks.org/c-program-demonstrate-fork-and-pipe/
  // I'm using the above as a reference guide to figure out how to use forks
  // and pipes.

  // That wasn't enough.  I ended up asking for guidance via email.  I was
  // going about this all wrong.  I was so fixated on strings and arguments
  // that I completely missed connecting `stdout` to `stdin`.

  int stdPipe[2]; // I only need to send data to the child process.

  pid_t p; // Tracks the process identifiers used by fork

  // Create the pipe:
  pipe(stdPipe);
  // Data written on stdPipe[1] can be read from stdoutPipe[0]

  // Fork this process:
  p = fork(); // p=-1 for errors, p=0 for the new child, and p>0 for this parent process.

  // If this is the parent process:
  if (p > 0)
  {
    // The parent process handles the second command.
    // This means it wants to read data from the child process,
    // which does the first command.

    // Close the writing end of the pipe:
    close(stdPipe[1]);

    // Redirect the piped data to `stdin`.

    // Make a copy of `stdin` so I can restore it later:
    int stdinCpy = dup(STDIN_FILENO);

    // Redirect piped data to `stdin`:
    dup2(stdPipe[0], STDIN_FILENO);

    // Execute the second command using the piped arguments:
    if (secondCmd[0] == '!') // I need to remove the '!' char before running the command:
    {
      system(secondCmd + 1);
    }
    else
    {
      system(secondCmd);
    }
    // TODO: What if this were another checkCommand call?

    // Restore `stdin`:
    dup2(stdinCpy, STDIN_FILENO);

    // Close both ends of the pipe:
    close(stdPipe[0]);
    close(stdPipe[1]);
  }
  else if (p < 0) // If an error occurred while forking...
  {
    fprintf(stderr, "An error occurred while forking.");
    exit(EXIT_FAILURE);
  }
  // Child process:
  else
  {
    // Close the read pipe:
    close(stdPipe[0]);

    // I need to redirect `stdout` to the write end of the pipe, then run the command.

    // Make a copy of `stdout` so I can restore it later:
    int stdoutCpy = dup(STDOUT_FILENO);

    // Redirect `stdout` to the pipe:
    dup2(stdPipe[1], STDOUT_FILENO);

    // Run the first command:
    // Note: the first command might NOT be a host command:
    if (firstCmd[0] == '!') // If the first command IS a host command:
    {
      system(firstCmd + 1); // Run the first command on the host, excluding the '!' char
    }
    else // Otherwise, the first command is a local command:
    {
      // TODO: Consider checking for redirects, backgrounds, additional pipes, etc.
      doCommand(firstCmd); // Note: this assumes the first command is a regular command.
    }

    // Restore `stdout`:
    dup2(stdoutCpy, STDOUT_FILENO);

    // Close the pipes:
    close(stdPipe[0]);
    close(stdPipe[1]);

    // Close the child process:
    exit(EXIT_SUCCESS);
  }
}

void doBackground(char *buf)
{
  // Run the given command in the background.
  // This could be a local or a host command.

  // Remove the '&' character from the command string:
  std::string sanitizedCmd = buf;
  for (long unsigned int i = 0; i < sanitizedCmd.length(); i++)
  {
    if (sanitizedCmd[i] == '&')
    {
      sanitizedCmd.erase(i, i + 1);
      i--; // We just deleted a character, go back one index.
    }
  }

  // Create a pid object ot track the process identifiers:
  pid_t pid;

  // Fork the process:
  pid = fork();

  // I only need to have the child process do anything:
  if (pid == 0)
  {
    // Run the given command.  Assume it's properly formatted.
    // If it's a system command:
    if (sanitizedCmd[0] == '!')
    {
      system(sanitizedCmd.c_str() + 1);
      exit(EXIT_SUCCESS);
    }
    else
    {
      // FIXME: Assume it's a local, non-piped, non-redirect command:
      // https://www.geeksforgeeks.org/convert-string-char-array-cpp/
      char *commandChar = new char[sanitizedCmd.length() + 1]; // doCommand ain't happy with str.c_str()
      doCommand(commandChar);
      exit(EXIT_SUCCESS);
    }
  }
  else if (pid > 0)
  {
    // Parent process, do nothing.
  }
  else // Otherwise something went wrong...
  {
    fprintf(stderr, "An error occurred while executing a background process.");
    exit(EXIT_FAILURE);
  }
}

// My method to parse commands, applying redirects, pipes, and background as appropriate:
void parseCommands(char *buf)
{
  // Start by checking for blank commands and comments.
  //   Note: much of this is a copy of code provided in main(), moved to here:
  if (buf[0] == 0)
  {
    return; // no command, so do nothing
  }
  else if (buf[0] == '#')
  {
    return; // this is a comment line, do nothing
  }
  else // If not blank or commented code, do stuff:
  {
    // If the command has a pipe:
    if (checkPipe(buf))
    {
      doPipe(buf); // TODO: does doPipe need to handle redirects, background, and additional commands?
    }
    else if (checkRedirect(buf)) // If the command has a redirect but no pipe:
    {
      doRedirect(buf); // TODO: Likewise as above.
    }
    else // If not a pipe or redirect:
    {
      if (buf[0] == '!') // If a host command:
      {
        if (checkBackground(buf)) // Check for any background:
        {
          doBackground(buf); // TODO: Consider having this method check for local/host execution!
        }
        else // not a background, pipe, or redirect, but it IS a host command:
        {
          system(buf + 1); // Execute the shell command on host, excluding the '!' character
        }
      }
      else // Not a pipe, redirect, or host command:
      {
        // Execute a local command.
        doCommand(buf); // Todo: Should this check for background, etc?
      }
    }
  }
}

/* End of my custom functions. */

int main()
{
  char buf[1024]; // better not type longer than 1023 chars

  usage();
  for (;;)
  {
    *buf = 0;               // clear old input
    printf("%s", "sh33% "); // prompt
    ourgets(buf);
    printf("cmd [%s]\n", buf); // just print out what we got as-is
    // I moved conditional checking of the given command to another method, allowing me to reuse the code elsewhere.
    parseCommands(buf); // Parse the command for comments, host, redirects, pipes, background, etc, then apply those as appropriate.
  }
}

// -eof-
