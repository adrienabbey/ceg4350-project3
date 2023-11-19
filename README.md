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
- Implement the commands described below.

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