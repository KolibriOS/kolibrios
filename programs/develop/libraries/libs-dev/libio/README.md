# Libio
> Lib for work with files

## Functions

### Files enumeration

##### `file_find_first`
> Find first file with matching attributes and mask in specified directory.

**Prototype**:
`proc file.find_first dir, mask, attr`

**Arguments**:
| name |  type  | Describtion                                              |
| :--- | :----: | :------------------------------------------------------- |
| dir  | ASCIIZ | Directory path, to search in                             |
| mask | ASCIIZ | File mask, with use of wildcards (a.k.a. shell patterns) |
| attr | ASCIIZ | File attributes mask (combination of FA_* constants)     |

**Result**:
| register | type      | Description                                                             |
| :------- | :-------: | :---------------------------------------------------------------------  |
| EAX      | Fileinfo* | 0 for error<br/>Matched file data pointer (__acts as find descriptor__) |


##### `file_find_next`
> Find next file matching criteria.

**Prototype**:
`proc file.find_next findd`

**Argument**:
| name  |    type   | Describtion                                                |
| :---- | :-------: | :--------------------------------------------------------- |
| findd | Fileinfo* | Find describtion (see [file_find_first](#file_find_first)) |

**Result**:
| register | type      | Description                                                            |
| :------- | :-------: | :--------------------------------------------------------------------- |
| EAX      | Fileinfo* | 0 for info<br/>Matched file data pointer (__acts as find descriptor__) |


##### `file_find_close`
> Find next file matching criteria.

**Prototype**:
`Find next file matching criteria.`

**Arguments**:
| name  |    type   | Describtion                                                |
| :---- | :-------: | :--------------------------------------------------------- |
| findd | Fileinfo* | Find describtion (see [file_find_first](#file_find_first)) |


**Result**:
| register | type  | Description                      |
| :------- | :---: | :------------------------------- |
| EAX      | dword | Result of memory freeing routine |
