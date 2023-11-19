/*
 * directory.C of CEG 433/633 File Sys Project
 * pmateti@wright.edu
 * 
 * Additional comments added by Adrien Abbey for Project 2, Oct. 2023.
 */

#include "fs33types.hpp"

#define iNumber(ptr) (*(ulong *)(ptr + strlen((char *)ptr) + 1))

/* pre:: pfv must point to a proper file volume, in should be > 0;;
 * post:: Construct a directory object, parent == 0 means that on the
 * disk image no changes are made, otherwise yes.
 */

Directory::Directory(FileVolume *pfv, uint in, uint parent)
{
  fv = pfv;
  dirf = 0;
  dirEntry = 0;
  nInode = in;
  if (parent == 0)
    return;

  fv->fbvInodes.setBit(in, 0); // the inode is in-use
  fv->inodes.setType(in, iTypeDirectory);
  addLeafName((byte *)".", in);
  addLeafName((byte *)"..", parent);
}

Directory::~Directory()
{
  this->namesEnd();
}

/* pre:: dirEntry/dirf may or may not be 0;; post:: Get the next file
 * name + i-number pair from this directory.  Returns ptr to dirEntry,
 * which is a private data member.  Returns 0 when there are no more
 * entries. */

byte *Directory::nextName()
{
  if (dirf == 0)
    dirf = new File(fv, nInode);
  if (dirEntry == 0)
    dirEntry = new byte // area of mem for file name + i-num
        [fv->superBlock.fileNameLengthMax + 1 + fv->superBlock.iWidth];

  byte *bp = dirEntry;
  while ((*bp = dirf->getNextByte()) != 0)
    bp++;
  if (bp == dirEntry) // end of directory
    return 0;

  for (uint i = 0; i < fv->superBlock.iWidth; i++) // deposit the i-number
    *++bp = dirf->getNextByte();
  return dirEntry;
}

/* Must be called after invocation(s) of nextName(). */

void Directory::namesEnd()
{
  if (dirf)
    delete dirf;
  dirf = 0;
#if 0
  if (dirEntry) delete dirEntry;
  dirEntry = 0;
#endif
}

/* pre:: in may or may not be in this directory;; post:: If in is in
 * this dir, set dirEntry[] so that it contains the file name +
 * i-number. If in is not there, dirEntry[0] == 0. */

byte *Directory::nameOf(uint in)
{
  byte *bp = 0;
  while ((bp = nextName()) != 0)
  {
    if (in == (uint)iNumber(bp))
      break;
  }
  namesEnd();
  return bp;
}

/* pre:: ;; post:: Search directory and set dirEntry matching with
 * leafnm.  Return inumber.  Callers wish to use dirf and dirEntry
 * further; do not do namesEnd(). */

/// @brief Searches the current directory for the given file name.
/// @param leafnm The file name to search for.
/// @return Returns the memory location of the file pointer of this directory,
/// or 0 if not found.
uint Directory::setDirEntry(byte *leafnm)
{
  if (leafnm == 0 || leafnm[0] == 0)
    return 0;

  uint nbToMatch = 1 + strlen((char *)leafnm), result = 0;
  for (byte *bp = 0; (bp = nextName());)
  {
    if (memcmp(bp, leafnm, nbToMatch) == 0)
    {
      result = iNumber(bp);
      break;
    }
  }
  return result;
}

/// @brief Search the current directory for the given file name.
/// @param leafnm The file name to search for.
/// @return If the given file name is found, return the inode number of that
/// file.  Otherwise, returns 0.
uint Directory::iNumberOf(byte *leafnm)
{
  uint in = setDirEntry(leafnm);
  namesEnd();
  return in;
}

uint okNameSyntax(byte *nm)
{
  return nm != 0                        // null pointer?
         && nm[0] != 0                  // empty "" nm?
         && isAlphaNumDot(nm[0])        // invalid begin char for name?
         && index((char *)nm, '/') == 0 // nm contains a slash?
      ;
}

/* pre:: none;; post:: Add file name newName with its inode number in
 * to this directory. */

/// @brief Adds an existing file to this directory.
/// @param newName Name of the file being added to this directory.
/// @param in Inode of the file being added to this directory.
void Directory::addLeafName(byte *newName, uint in)
{
  if (in == 0 || okNameSyntax(newName) == 0)
    return;

  // if name is too long, truncate it
  uint newNameLength = strlen((char *)newName);
  if (newNameLength > fv->superBlock.fileNameLengthMax - 1)
  {
    newNameLength = fv->superBlock.fileNameLengthMax - 1;
    newName[newNameLength] = 0;
  }

  if (setDirEntry(newName) == 0)
  {
    memcpy(dirEntry, newName, newNameLength + 1); // append NUL also
    memcpy(dirEntry + newNameLength + 1, &in, fv->superBlock.iWidth);
    dirf->appendBytes(dirEntry, newNameLength + 1 + fv->superBlock.iWidth);
  }
  namesEnd();
}

/* pre:: in is valid;; post:: List the directory inode in's content in
 * a manner similar to Unix ls -lia. If printfFlag != 0, output it to
 * stdout.  Return the total number of files.  */

uint Directory::lsPrivate(uint in, uint printfFlag)
{
  uint nFiles = 0;
  Directory *d = new Directory(fv, in, 0);
  for (byte *bp = 0; (bp = d->nextName()); nFiles++)
  {
    uint in = iNumber(bp);
    if (printfFlag)
    {
      byte c = (d->fv->inodes.getType(in) == iTypeDirectory ? 'd' : '-');
      printf("%7d %crw-rw-rw-    1 yourName yourGroup %7d Jul 15 12:34 %s\n",
             in, c, d->fv->inodes.getFileSize(in), bp);
    }
  }
  delete d;
  return nFiles - 2; // -2 because of "." and ".."
}

uint Directory::ls()
{
  return lsPrivate(nInode, 1); // 1 ==> printf it
}

/// @brief Creates a new file or folder in the this directory, if it does not
/// already exist.
/// @param leafnm Name of the file or folder to be created.
/// @param dirFlag If true, creates a new directory.  Otherwise, creates an
/// ordinary file.
/// @return Returns the inode number of either the new file/folder, or the
/// inode number of an existing file/folder.  If no file was created, returns 0.
uint Directory::createFile(byte *leafnm, uint dirFlag)
{
  uint in = iNumberOf(leafnm);
  if (in == 0)
  {
    in = fv->inodes.getFree();
    if (in > 0)
    {
      addLeafName(leafnm, in);
      if (dirFlag)
        delete new Directory(fv, in, nInode);
      else
        fv->inodes.setType(in, iTypeOrdinary);
    }
  }
  return in;
}

/* Do not delete if it is dot or dotdot.  Do not delete if it is a
 * non-empty dir. */

/// @brief Deletes the file reference from this directory.  Note: if the inode is
/// not freed, it will only delete this directory's reference to the file
/// without deleting the file!
/// @param leafnm Byte string of the file to be deleted.
/// @param freeInodeFlag Treated as a boolean: if true, set the inode free.  If
/// false, the file will only be dereferenced from this directory.
/// @return Returns the inode of the file deleted, or 0 if nothing was deleted.
uint Directory::deleteFile(byte *leafnm, uint freeInodeFlag)
{
  if (strcmp((char *)leafnm, ".") == 0 ||
      strcmp((char *)leafnm, "..") == 0)
    return 0;

  uint in = setDirEntry(leafnm);
  if (in > 0)
  {
    dirf->deletePrecedingBytes(1 + strlen((char *)leafnm) + fv->superBlock.iWidth);
    if (freeInodeFlag)
      fv->inodes.setFree(in);
  }
  return in;
}

/* pre:: pn is a dir inode, leafnm != 0, leafnm[0] != 0;; post:: Move
 * file named leafnm whose current parent is pn into this directory.;;
 */

/// @brief Moves the given file belonging to the given directory to this
/// directory.  This does NOT rename the file, as it only takes a single file
/// name!
/// @param pn Inode of the parent directory of the file being moved from.
/// @param leafnm File name of the file being moved to this directory.
/// @return Returns the inode of the moved file if successful, 0 if
/// unsuccessful.
uint Directory::moveFile(uint pn, byte *leafnm)
{
  // Create a reference to the source directory:
  Directory *sourceDir = new Directory(fv, pn, 1);

  // Add the file to this directory.
  addLeafName(leafnm, sourceDir->iNumberOf(leafnm));

  // "Delete" the file from the old parent directory, without freeing the file.
  sourceDir->deleteFile(leafnm, 0);

  // Return the inode of the moved file if successful.
  return iNumberOf(leafnm);
}

// -eof-
