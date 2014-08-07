/*
  FUSE: Filesystem in Userspace
  ACV - IST -  Taguspark, October 2012

*/

#define FUSE_USE_VERSION 26

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include "block.h"
#include "fs.h"

#define BLOCK_SIZE 512
#ifndef NUM_BLOCKS
// default storage of 2 MB (8*512 blocks * 512 bytes/block)
#define NUM_BLOCKS (8*512)
#endif

// maximum amount of directory entries 
#define MAX_READDIR_ENTRIES 64 
// maximum size of a file name used in barefs
#define MAX_FILE_NAME_SIZE 14

int myparse(char *pathname);
int myparsepathnames(char* pathname, char* outfilename, char* outdirname);


static fs_t* FS;

///////////////////////////////////////////////////////////
////////////////      AUX FUNCTIONS
///////////////////////////////////////////////////////////



/** myparse() - auxiliar function: checks if the given 'pathname' is coherent with the filesystem */
int myparse(char* pathname) {
  char line[MAX_PATH_NAME_SIZE];
  char *token;
  char *search = "/";
  int i;
  strcpy(line,pathname);

  if(strlen(line) >= MAX_PATH_NAME_SIZE || strlen(line) < 1) /* Wrong pathname size */
      return -1;

  if (strchr(line, ' ') != NULL || strstr( (const char *) line, "//") != NULL || line[0] != '/' ) /* verifies if the given pathname is malformed */
      return -1;

  if ((i=strlen(pathname)) && line[i]=='/') /* verifies if the given pathname is malformed */
      return -1;

  i=0;
  token = strtok(line, search);

  while(token != NULL) {
      if ( strlen(token) > MAX_FILE_NAME_SIZE -1) /* verifies if the given pathname sizes exceeds the limits */
      return -1;
      i++;
      token = strtok(NULL, search);
  }

return 0;
}

  /** myparsepathnames() - auxiliar function: parses through the input pathname and get it's base-
  directory name and filename (if any)
  * - receives as input argument the 'pathname'[in]
  * - the 'outfilename' is the filename of the given pathname [out]
  * - the 'outdirname' is the directory path to the base-directory [out]
  * -» returns the depth of subdirectories the file is in (ie: returns 1 if the file is at the root folder)
  */
int myparsepathnames(char* pathname, char* outfilename, char* outdirname){
  char newfilename[MAX_FILE_NAME_SIZE];
  char newdirname[MAX_PATH_NAME_SIZE];
  char fulldirname[MAX_PATH_NAME_SIZE];
  char *token;
  char *search="/";
  int i;

  memset(&newfilename, 0, MAX_FILE_NAME_SIZE);// the name of the file to be parsed(from pathname)
  memset(&newdirname, 0, MAX_PATH_NAME_SIZE); // the name of the base directory (of the pathname)
  memset(&fulldirname, 0, MAX_PATH_NAME_SIZE);

  strcpy(fulldirname, pathname);
  token = strtok(fulldirname, search);

  for(i=0; token != NULL; i++) {// counts the depth of the directories and parses through directories
    memset(&newfilename, 0, MAX_FILE_NAME_SIZE);
    strncpy(newfilename, token, strlen(token));
    newfilename[strlen(newfilename)] = '\0';
    token = strtok(NULL, search);
  }

  if(i > 1) {// not at the root folder
    strncpy(newdirname, pathname, strlen(pathname)-strlen(newfilename));
   newdirname[strlen(newdirname)] = '\0'; // assure the string ending

  } else { // i=1 :: the file is at the root directory (if i=0 then the pathname is wrong)
    strncpy(newfilename, &pathname[1], strlen(pathname));
    newfilename[strlen(newfilename)] = '\0'; // assure the string ending
    strncpy(newdirname, pathname, strlen(pathname)-strlen(newfilename));
    newdirname[strlen(newdirname)] = '\0'; // assure the string ending
  }

// set the output variables and assure the string ends with '\0'
    strncpy(outfilename, newfilename, MAX_FILE_NAME_SIZE);
    outfilename[MAX_FILE_NAME_SIZE-1]='\0';
    strncpy(outdirname, newdirname, MAX_PATH_NAME_SIZE);
    outdirname[MAX_PATH_NAME_SIZE-1]='\0';

return i;
}

///////////////////////////////////////////////////////////
//
// Prototypes for all these functions, and the C-style comments,
// come indirectly from /usr/include/fuse.h
////////////////////////////////////////////////////////////

/**
 * Initialize filesystem
 */
void *barefs_init(struct fuse_conn_info *conn)
{
    FS = fs_new(NUM_BLOCKS);
    fs_format(FS);
    return NULL;
}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 */
void barefs_destroy(void *userdata)
{
}


/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 * 
 */
int barefs_create(const char *path, mode_t mp, struct fuse_file_info *fi) 
{
  char name[MAX_FILE_NAME_SIZE];
  char dirname[MAX_PATH_NAME_SIZE];
  inodeid_t fileid, dir;

  /* check if the pathname is malformed */
  if(myparse((char*)path) != 0){
  printf("[barefs_create] Malformed pathname.\n");
  return -1;
  }

  /* get filename & dirname*/
  int i = myparsepathnames((char*)path, name, dirname);


 /* verifies if the name exists */
  if(name == NULL) {
  printf("[barefs_create] Error looking for directory.\n");
  return -1;
  }

  /* verifies if the inode of the file exists on the dirname */
  if(fs_lookup(FS, dirname, &fileid) != 1){
  printf("[barefs_create] The file '%s' does not exist.\n", path);
  return -1;
  }

  /* verifies if the inode of the directory exists on the dirname */
  if(fs_lookup(FS, dirname,&dir) != 1){
  printf("[barefs_create] The parent-directory does not exist.\n");
  return -1;
  }


  /* if file is at the root directory */
if(i==1)
  dir = 1; 

   /* verifies if fs_create is successful */
  if(fs_create(FS, dir, name, &fileid) != 0) {
    printf("[barefs_create] Error creating file.\n");
    return -1;
  }

  fi->fh = fileid;

return 0; 
}



/** Get file attributes.
 *
*/
int barefs_getattr(const char *path, struct stat *stbuf)
{

   int res = -ENOENT;
   inodeid_t fileid;

   memset(stbuf, 0, sizeof(struct stat));
   
   fs_file_attrs_t attrs;

   // Root Directory Attributes
   if ( strcmp((char*)path,"/")== 0 ) {
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
		stbuf->st_size = BLOCK_SIZE;
		return 0;		
   }
  
   if (fs_lookup(FS,(char*)path,&fileid) == 1) {
    	printf("[barefs_getattr] filename: '%s' [inode: %d]\n", path, fileid);
      	if (fs_get_attrs(FS,fileid,&attrs) == 0) {
	      if (attrs.type == 2) {
			stbuf->st_nlink = attrs.links; /* the number of hard links here */
			stbuf->st_mode = S_IFREG | 0777;
			stbuf->st_size = attrs.size;
			res = 0;
              } else if (attrs.type == 1) {
				stbuf->st_nlink = 2;
				stbuf->st_mode = S_IFDIR | 0777;
				stbuf->st_size = BLOCK_SIZE;
				res = 0;
	             }
      	}
   } 
   return res;   
}

/** Read directory
 *
*/ 
int barefs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    int res = -ENOENT;	
    (void) offset;
    (void) fi;
    inodeid_t fileid;
    int maxentries;	
    fs_file_attrs_t attrs;

    if(strcmp(path, "/") == 0) 
	   fileid=1; //1 is the root directory inode number
    else { 
          if (fs_lookup(FS,(char*)path,&fileid) != 1) 
	      return res;
    }

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
	
    if (fs_get_attrs(FS,fileid,&attrs) == 0) {
	if (attrs.type == 1) {
		maxentries = attrs.num_entries;
		//printf("[barefs_readdir] Directory '%s' has %d entries.\n", path, attrs.num_entries);			
		res = 0;
         }
		
	if (maxentries) {
		// Read the directory
		fs_file_name_t entries[MAX_READDIR_ENTRIES];
		int numentries;
		if (!fs_readdir(FS,fileid,entries,maxentries,&numentries)) {
		    int i;
	            for (i = 0; i < numentries; i++) {						
			filler(buf, entries[i].name, NULL, 0);  
		    }
		    res = 0;	  
		}
	}	
    }
    return res;	
}


/**Create a directory with the given name.
* 
*/
int barefs_mkdir(const char* path, mode_t mode) 
{ 


  char newfilename[MAX_FILE_NAME_SIZE];
  char newdirname[MAX_PATH_NAME_SIZE];
  char fulldirname[MAX_PATH_NAME_SIZE];
  int i=0;
  inodeid_t fileid, dir;

  /* check if the pathname is not malformed */
  if(myparse((char*)path) != 0 ) {
    printf("[barefs_mkdir] Malformed pathname.\n");
    return -1;
  }

    /* check if the subdirectory exists */
   if(fs_lookup(FS,(char*)path,&dir) == 1) {
    printf("[barefs_mkdir] Error creating a subdirectory that already exists.\n");
    return -1;
  }


  memset(&newfilename,0,MAX_FILE_NAME_SIZE);
  memset(&newdirname,0,MAX_PATH_NAME_SIZE);
  memset(&fulldirname,0,MAX_PATH_NAME_SIZE);

  i = myparsepathnames((char *)path, newfilename, newdirname);


    /* if directory is on root */
    if (i==1) 
      dir = 1;
    
    /* create a directory elsewhere */
    else { 
      if(fs_lookup(FS,newdirname,&dir) != 1) {
      printf("[barefs_mkdir] Error creating a subdirectory which has a wrong pathname.\n");
      return -1;
        }
     }

     /* verifies if fs_mkdir is successful */
      if(fs_mkdir(FS,dir,newfilename,&fileid) != 0)  {
      printf("[barefs_mkdir] Error creating new directory.\n");
      return -1;
      }


   return 0;  
}



/**Remove the given directory.
*
* This should succeed only if the directory is empty
*/
int barefs_rmdir(const char* path)
{
   /* check if the user tries to remove the root folder */
  if(path[0]=='/' && path[1]=='\0'){
  printf("[barefs_rmdir] The root '/' directory cannot be removed.\n");
  return -1;
  }

  /* check if the pathname is malformed */
  if(myparse((char*)path) != 0){
  printf("[barefs_rmdir] Malformed pathname.\n");
  return -1;
  }

  char name[MAX_FILE_NAME_SIZE];
  char dirname[MAX_PATH_NAME_SIZE];

  /* get filename & dirname */
  int i = myparsepathnames((char*)path, name, dirname);


  /* verifies if the name exists */
  if(name == NULL) {
  printf("[barefs_rmdir] Error looking for directory.\n");
  return -1;
  }

  inodeid_t fileid, dir;

  /* verifies if the fileid exists in the path given */
  if(fs_lookup(FS,(char*)path,&fileid) != 1){
  printf("[barefs_rmdir] The directory '%s' does not exist.\n", path);
  return -1;
  }

  /* verifies if the dir exists in the dirname given */
  if(fs_lookup(FS,dirname,&dir) != 1){
  printf("[barefs_rmdir] The parent-directory does not exist.\n");
  return -1;
  }

  /* if the directory is at the root directory */
  if(i==1)
    dir = 1; 
 
  /* verifies if fs_rmdir is successful */
  if(fs_rmdir(FS, dir, name) != 0) {
    printf("[barefs_rmdir] Error removing directory.\n");
    return -1;
  }
return 0;	
}


/**Create hard link
*
* Create a hard link between "from" and "to".
*/
int barefs_link(const char* from, const char* to)
{
  int res = -ENOENT;
  int i = 0;
  char filename[MAX_FILE_NAME_SIZE];
  char filedirname[MAX_PATH_NAME_SIZE];
  char linkname[MAX_FILE_NAME_SIZE];
  char linkdirname[MAX_PATH_NAME_SIZE];
  inodeid_t fileid, dir;

   
  /* check if the pathname is malformed */
  if(myparse((char*)from) != 0){
  printf("[barefs_link] Malformed pathname.\n");
  return -1;
  }

  myparsepathnames((char*)from, filename, filedirname);
  i = myparsepathnames((char*)to, linkname, linkdirname);

  /* verifies if the given filename exists */
  if(filename == NULL) {
  printf("[barefs_link] Error looking for file.\n");
  return -1;
  }

  /* verifies if the fileid exists in the from given */
  if(fs_lookup(FS, (char *)from, &fileid) != 1){
  printf("[barefs_link] The file '%s' does not exist.\n", from);
  return -1;
  }

  /* verifies if the dir exists in the linkdirname given */
  if(fs_lookup(FS, linkdirname,&dir) != 1){
  printf("[barefs_link] The parent-directory does not exist (Hard Link).\n");
  return -1;
  }

  /* if file is at the root directory */
  if(i==1)
    dir = 1; 

  /*verifies if fs_link is successful */ 
  if(fs_link(FS,dir,linkname,fileid) == 0){
          printf("[barefs_link] Linking file '%s' \n", to);
          res = 0;
        } 
		
   return res;
}



/** Remove (delete) the given file or hard link 
*
*
*/
int barefs_unlink(const char* path)
{
   struct stat * stbuf = NULL;

   stbuf = (struct stat *)malloc(sizeof(struct stat));

   /* check if the pathname is malformed */
  if(myparse((char*)path) != 0){
  printf("[barefs_unlink] Malformed pathname.\n");
  return -1;
  }

  char name[MAX_FILE_NAME_SIZE];
  char dirname[MAX_PATH_NAME_SIZE];

  int i = myparsepathnames((char*)path, name, dirname);

  /* verifies if the name exists */
  if(name == NULL) {
  printf("[barefs_unlink] Error looking for file.\n");
  return -1;
  }

  inodeid_t fileid, dir;

  /* verifies if the fileid exists in the dirname given */
  if(fs_lookup(FS, dirname, &fileid) != 1){
  printf("[barefs_unlink] The file '%s' does not exist.\n", path);
  return -1;
  }

  /* if file is at the root directory */
if(i==1)
  dir = 1; 

    /* verifies if the dir existes on the dirname given */
   if (fs_lookup(FS,dirname,&dir) == 1) {

      /* get the atributes of the path */
      barefs_getattr(path, stbuf);

          /* then removes the link */
          if (fs_remove(FS,dir,name,&fileid) == 0) {
                  printf("[barefs_unlink] Removing link '%s' \n", path);
          }
  }

   return 0;  
}

/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 *
 */
int barefs_open(const char *path, struct fuse_file_info *fi)
{
   int res = -ENOENT;
   inodeid_t fileid;

   if (fs_lookup(FS,(char*)path,&fileid) == 1) {
	fi->fh= fileid;
	res = 0;
   } 

   return res;	
}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes (check fuse documentation for exceptions).
 *
 */
int barefs_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
   int res = -1;
   int fileid = fi->fh;
   int nread = 0; 
   
   if (fs_read(FS,fileid,offset,size,buf,&nread) == 0){
	offset += nread;
	res = nread;
   }   
   
   return res;	

}


/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes (check fuse documentation for exceptions).
 *
 */
int barefs_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
   int res=-1;
   int fileid = fi->fh; 

   if (!fs_write(FS,fileid,offset,size,(char*)buf)){
		res=(int) size;
   }   

   return res;	
}


/** Create a file node
 *
 *
 * mknod() apparently skould be called, instead of create(),
 * for many shell commands performing file creation, directory 
 * listing, etc ..
 */
int barefs_mknod(const char *path, mode_t mode, dev_t dev)
{
   return 0;
}


/*
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.
 */
int barefs_flush(const char *path, struct fuse_file_info *fi)
{   
   return 0;
}

/** Change the access and/or modification times of a file */
int barefs_utime(const char *path, struct utimbuf *ubuf)
{
    return 0;
}

/** Change the owner and group of a file */
int barefs_chown(const char *path, uid_t uid, gid_t gid)  
{ 
    return 0;
}

/** Change the permission bits of a file */
int barefs_chmod(const char *path, mode_t mode)
{  
    return 0;
}

/** Change the size of a file */
int barefs_truncate(const char *path, off_t newsize)
{
   int res = -ENOENT;
   inodeid_t fileid;

   if (fs_lookup(FS,(char*)path,&fileid) == 1) {
        if (fs_truncate(FS, fileid) == 0)
	   res = 0;
   } 
	
   return res;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 */
int barefs_release(const char *path, struct fuse_file_info *fi)
{
   return 0;
}

/** Get extended attributes */
// Is usefull when executing "ls" shell command
int barefs_getxattr(const char *path, const char *name, char *value, size_t size)
{  
   return 0;
}

static struct fuse_operations barefs_oper = {
	.getattr	= barefs_getattr,
	.readdir	= barefs_readdir,
	.open		= barefs_open,
	.create		= barefs_create,
	.mkdir		= barefs_mkdir,
	.rmdir		= barefs_rmdir,
	.link		= barefs_link,
	.unlink		= barefs_unlink,  
	.mknod		= barefs_mknod,
	.read		= barefs_read,
	.write		= barefs_write,
	.init		= barefs_init,
	.flush		= barefs_flush,
	.destroy	= barefs_destroy,
	.release	= barefs_release,
	.utime		= barefs_utime,
	.chown		= barefs_chown,
	.chmod		= barefs_chmod,
	.truncate	= barefs_truncate,
	.getxattr 	= barefs_getxattr,	
	
};

int main(int argc, char *argv[])
{
    return fuse_main(argc, argv, &barefs_oper, NULL);	
}


