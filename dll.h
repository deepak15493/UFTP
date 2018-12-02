#ifndef _DLL_H_
#define _DLL_H_

#define BUILDING_DLL 1

#if BUILDING_DLL
# define DLLIMPORT __declspec (dllexport)
#else /* Not BUILDING_DLL */
# define DLLIMPORT __declspec (dllimport)
#endif /* Not BUILDING_DLL */


DLLIMPORT void purgeDirTree(void);
DLLIMPORT void* getCommand(void* filename);
DLLIMPORT int JSONTreeInterpret(void* JSONTreeBuf);
DLLIMPORT void listDir(void);
DLLIMPORT int changeDir(void* subDir);
DLLIMPORT void* InitDirTree(void);
DLLIMPORT void* getrqpath(void);

#endif /* _DLL_H_ */
