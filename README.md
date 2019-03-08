# file_system

* todo:
  *multi users , the privelage control.(don't wanna figure out...)

data structure:
* SuperBlock:

  simplified defined as: bitmap record & free blocks manage & fs information
  
* inode(96 Bytes):
  
  1. user id 
  2. file mode & time information
  3. file blocks pos & blocks count
  
* dir file(1024 Bytes):
  
  1. set of dir entry
  2. number of dir entrys
  
* dir entry(64 Bytes):
  
  1. inode id
  2. dname

* data file(1024 Bytes):
  
  bufffer
