# Project 3: Links and Medium Size Files
> Note: As before, I'm building upon my Project 2 work.

## Hard and Soft Links
- These should be implemented to work much like Linux hard and soft links.
- Inodes need to include a field tracking its number of links.
- There are three types of inodes: ordinary files, directories, and soft-links.
- Do NOT permit hard links to directories!
- DO permit soft links to directories.
## Medium Size Files
- Medium files are defined as those that require using an inodes direct block number entries, as well as requiring single-direct blocks.
- Large files have the same requirements as medium files, plus requiring double-indirect blocks.  This is a bonus point objective.
## Simple Test Session
- Implement the commands described below.  This builds upon how commands worked before, adding additional functionality for links and medium files.  Thus, only *new* functionality is described below.  `testscript.txt` should demonstrate the new functionality involving links and medium files.
- `mkfs disknm`
    - Creates an empty file volume on the simulated disk `disknm.dsk`.
    - Note: inode height *now* includes not only the `fileSize` field, but also the link count.
- `ln opnm npnm`
    - Creates `npnm` as a new *hard* link to `opnm` and prints out the inode number.
    - Creates nothing new and returns `0` if:
        - `opnm` is a directory
        - no ordinary file exists at the given `opnm`
        - `npnm` already exists
        - `npnm` is an invalid pathname
- `ln opnm .`
    - Creates a hard link named the same as the leaf name of `opnm` in the current directory and prints its inode number.
    - Creates nothing new and returns `0` under the same conditions listed above.
    - For bonus points, implement the same functionality if `.` is ommited.
- `ln -s opnm npnm`
    - Creates `npnm` as a *soft* link to `opnm` and prints its inode number.
    - Creates nothing new and returns `0` if:
        - `opnm` does not exist
        - `npnm` already exists
        - `npnm` is an invalid path
- `rm pnm`
    - Removes the path `pnm`, which may or may not be a link, and prints the resulting link count.

## Grading Rubric
- [ ] `testscript.txt` is designed to demonstrate and test all the functionality of the new links and medium sized files (5 pts)
- [ ] `testscript.txt` successfully navigates as expected (5 pts)
- [ ] `ln opnm npnm` (5 pts)
- [ ] `ln -s opnm npnm` (10 pts)
- [ ] `cd link-to-dir` (10 pts)
- [ ] `cd ..` after the above (5 pts)
- [ ] `cp hard-link-pnm pnm` (5 pts)
- [ ] `cp soft-link-pnm pnm` (10 pts)
- [ ] `rm hard-or-soft-link-pnm` (10 pts)
- [ ] `mv soft-link-pnm pnm` (10 pts)
- [ ] `mv hard-link-pnm pnm` (5 pts)
- [ ] `inode` updated to show all inode fields (10 pts)
- [ ] Medium files implemented (10 pts)
- [ ] `ln opnm` no-dot (10 bonus pts)
- [ ] Large files implemented (50 bonus points)