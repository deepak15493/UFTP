#ifndef _DLL_H_
#define _DLL_H_

#define BUILDING_DLL 1 

#if BUILDING_DLL
# define DLLIMPORT __declspec (dllexport)
#else /* Not BUILDING_DLL */
# define DLLIMPORT __declspec (dllimport)
#endif /* Not BUILDING_DLL */


DLLIMPORT void* JSONTreeBuild(void);
DLLIMPORT void purgeJSONTree(void);
DLLIMPORT void* fetchJSONBuffer(char* filepath,struct jsontree_object* final_tree);


#endif /* _DLL_H_ */
